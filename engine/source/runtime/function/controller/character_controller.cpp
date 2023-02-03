#include "runtime/function/controller/character_controller.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/physics/physics_scene.h"
#include<iostream>
namespace Pilot
{
    CharacterController::CharacterController(const Capsule& capsule) : m_capsule(capsule)
    {
        m_rigidbody_shape                                    = RigidBodyShape();
        m_rigidbody_shape.m_geometry                         = PILOT_REFLECTION_NEW(Capsule);
        *static_cast<Capsule*>(m_rigidbody_shape.m_geometry) = m_capsule;

        m_rigidbody_shape.m_type = RigidBodyShapeType::capsule;

        Quaternion orientation;
        orientation.fromAngleAxis(Radian(Degree(90.f)), Vector3::UNIT_X);

        m_rigidbody_shape.m_local_transform =
            Transform(
                Vector3(0, 0, capsule.m_half_height + capsule.m_radius),
                orientation,
                Vector3::UNIT_SCALE);
    }

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        std::vector<PhysicsHitInfo> hits;

        Transform world_transform = Transform(
            current_position + 0.1f * Vector3::UNIT_Z,
            Quaternion::IDENTITY,
            Vector3::UNIT_SCALE);

        Vector3 vertical_displacement   = displacement.z * Vector3::UNIT_Z;
        Vector3 horizontal_displacement = Vector3(displacement.x, displacement.y, 0.f);

        Vector3 vertical_direction   = vertical_displacement.normalisedCopy();
        Vector3 horizontal_direction = horizontal_displacement.normalisedCopy();

        Vector3 final_position = current_position;

        m_is_touch_ground = physics_scene->sweep(
            m_rigidbody_shape,
            world_transform.getMatrix(),
            Vector3::NEGATIVE_UNIT_Z,
            0.105f,
            hits);
       // std::cout << m_is_touch_ground << std::endl;
        hits.clear();
        
        world_transform.m_position -= 0.1f * Vector3::UNIT_Z;

        // vertical pass
        if (physics_scene->sweep(
            m_rigidbody_shape,
            world_transform.getMatrix(),
            vertical_direction,
            vertical_displacement.length(),
            hits))
        {
            final_position += hits[0].hit_distance * vertical_direction;
            m_is_touch_ground = true;
        }
        else
        {
            final_position += vertical_displacement;
           
        }

        hits.clear();

        // side pass
        if (physics_scene->sweep(
            m_rigidbody_shape,
            world_transform.getMatrix(),
            horizontal_direction,
            horizontal_displacement.length(),
            hits))
        {
            if (hits[0].hit_distance)
            {
                final_position += hits[0].hit_distance * horizontal_direction;
            }
            else
            {
                std::vector<PhysicsHitInfo> hits_one, hits_two, hits_three;
                Vector3                     q_position = current_position;
            //    std::cout << current_position.x << " " << q_position.x << "\n";
                bool is_hitted_one =
                    physics_scene->raycast(q_position + Vector3::UNIT_Z,
                                           Vector3(0.966 * horizontal_direction.x - 0.259 * horizontal_direction.y,
                                                   0.259 * horizontal_direction.x + 0.966 * horizontal_direction.y,
                                                   horizontal_direction.z),
                                                            2.0,
                                                            hits_one);
                bool is_hitted_two =
                    physics_scene->raycast(q_position + Vector3::UNIT_Z,
                                           Vector3(0.966 * horizontal_direction.x + 0.259 * horizontal_direction.y,
                                                   -0.259 * horizontal_direction.x + 0.966 * horizontal_direction.y,
                                                   horizontal_direction.z),
                                                        2.0,
                                                        hits_two);


                bool is_hitted_three =
                    physics_scene->raycast(q_position + Vector3::UNIT_Z, horizontal_direction, 2.0, hits_three);

             //   std::cout << "BEFORE\n";
             //   std::cout << q_position.x << " " << q_position.y << " " << q_position.z  << "\n";
                if (is_hitted_one && is_hitted_two && is_hitted_three)
                {
                    std::cout << "hittttt\n";
                    std::cout << hits_one[0].hit_position.x << " " << hits_three[0].hit_position.x << " "
                              << hits_two[0].hit_position.x
                              << "\n";
                    std::cout << hits_one[0].hit_position.y << " " << hits_three[0].hit_position.y << " "
                              << hits_two[0].hit_position.y
                              << "\n";
                    std::cout << hits_one[0].hit_position.z << " " << hits_three[0].hit_position.z << " "
                              << hits_two[0].hit_position.z
                              << "\n";
                    Vector3 ab = hits_one[0].hit_position - hits_three[0].hit_position;
                    Vector3 ac = hits_two[0].hit_position - hits_three[0].hit_position;
                    Vector3 n =
                        Vector3(ab.y * ac.z - ac.y * ab.z, ab.z * ac.x - ac.z * ab.x, ab.x * ac.y - ac.x * ab.y);

                    if (n.length() < 0.5)
                    {
                    //    Vector3 sq = q_position - hits[0].hit_position;
                       // Vector3 so = sq + n * (sq * n - 1); 
                        Vector3 so = ab.dotProduct(horizontal_direction) < 0 ? ac : ab; 
                        so.z       = 0;
                        so.normalise();
                        std::cout << "MOve " << vertical_displacement.length() << '\n';
                        final_position += 0.1 * so;
                    }
                    

                }
            }
        }
        else
        {
            final_position += horizontal_displacement;
        }

        return final_position;
    }

} // namespace Pilot
