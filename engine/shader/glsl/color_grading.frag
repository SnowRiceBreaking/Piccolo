#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture_sampler;

layout(location = 0) out highp vec4 out_color;

void main()
{
    highp ivec2 lut_tex_size = textureSize(color_grading_lut_texture_sampler, 0);
    highp float _COLORS      = float(lut_tex_size.y);

    highp vec4 color       = subpassLoad(in_color).rgba;
    
  //  highp int square_index = int(color.z / 4.0);

  //  highp int bx = square_index / 8;
  //  highp int by = square_index - bx * 8;
  //  highp int x = int(color.x / 4.0);
  //  highp int y = int(color.y / 4.0);

  //  highp vec2 uv = vec2(x + bx * 64, y + by * 64);
  //  highp vec4 color_sampled = texture(color_grading_lut_texture_sampler, uv);

  //  color_sampled.x += (color.r - float(x * 4));
  //  color_sampled.y += (color.g - float(y * 4));
  //  color_sampled.z += (color.b - float(square_index * 4));


    highp int square_index = int(color.z / 16.0);

    highp int x = int(color.x / 16.0);
    highp int y = int(color.y / 16.0);

    highp vec2 uv = vec2(x + square_index * 16, y);
    highp vec4 color_sampled = texture(color_grading_lut_texture_sampler, uv);

    color_sampled.x += (color.r - float(x * 16));
    color_sampled.y += (color.g - float(y * 16));
    color_sampled.z += (color.b - float(square_index * 16));

    out_color = color_sampled;
}
