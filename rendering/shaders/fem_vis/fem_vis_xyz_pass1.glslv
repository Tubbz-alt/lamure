// Copyright (c) 2014-2018 Bauhaus-Universitaet Weimar
// This Software is distributed under the Modified BSD License, see license.txt.
//
// Virtual Reality and Visualization Research Group 
// Faculty of Media, Bauhaus-Universitaet Weimar
// http://www.uni-weimar.de/medien/vr

#version 420 core


out VertexData {
  vec3 pass_ms_u;
  vec3 pass_ms_v;
  vec3 pass_normal;
} VertexOut;

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;
uniform mat4 model_view_matrix;
uniform mat4 inv_mv_matrix;
uniform mat4 model_to_screen_matrix;

uniform float near_plane;

uniform bool face_eye;
uniform vec3 eye;
uniform float max_radius;

uniform float point_size_factor;
uniform float model_radius_scale;

uniform int fem_result;
uniform int fem_vis_mode;
uniform float fem_deform_factor;
uniform float fem_min_absolute_deform;
uniform float fem_max_absolute_deform;
uniform vec3 fem_min_deformation;
uniform vec3 fem_max_deformation;

layout(location = 0) in vec3 in_position;
layout(location = 1) in float in_r;
layout(location = 2) in float in_g;
layout(location = 3) in float in_b;
layout(location = 4) in float empty;
layout(location = 5) in float in_radius;
layout(location = 6) in vec3 in_normal;

layout(location = 7) in float prov1;
layout(location = 8) in float prov2;
layout(location = 9) in float prov3;
layout(location = 10) in float prov4;
layout(location = 11) in float prov5;
layout(location = 12) in float prov6;
layout(location = 13) in float prov7;
layout(location = 14) in float prov8;
layout(location = 15) in float prov9;

INCLUDE ../common/compute_tangent_vectors.glsl

vec3 unpack_fem_result(float v){
  vec3 res = vec3(fract(v), fract(v * 256.0), fract(v * 65536.0));
  res *= (fem_max_deformation - fem_min_deformation);
  res += fem_min_deformation;
  res *= 2.0;
  res -= 1.0;
  return res;
}

void main() {
  float radius = in_radius;
  if (radius > max_radius) {
    radius = max_radius;
  }

  vec3 new_in_position = in_position;
  vec3 deformation = vec3(0.0);
  float deform = 0.0;
  if(0 != fem_result){
    if(1 == fem_result){
      deform = prov1;
    }
    else if(2 == fem_result){
      deform = prov2;
    }
    else if(3 == fem_result){
      deform = prov3;
    }
    else if(4 == fem_result){
      deform = prov4;
    }
    else if(5 == fem_result){
      deform = prov5;
    }
    else if(6 == fem_result){
      deform = prov6;
    }
    else if(7 == fem_result){
      deform = prov7;
    }
    else if(8 == fem_result){
      deform = prov8;
    }
    else if(9 == fem_result){
      deform = prov9;
    }
    
    if(0.0 != deform){
      deformation = unpack_fem_result(deform);

      if(1 == fem_vis_mode){
        new_in_position += fem_deform_factor * deformation;
      }

    }
  }


  vec3 normal = in_normal;
  if (face_eye) {
    normal = normalize(eye-(model_matrix*vec4(new_in_position, 1.0)).xyz);
  }
 
  // precalculate tangent vectors to establish the surfel shape
  vec3 tangent   = vec3(0.0);
  vec3 bitangent = vec3(0.0);
  compute_tangent_vectors(normal, radius, tangent, bitangent);

  if (!face_eye) {
    normal = normalize((inv_mv_matrix * vec4(in_normal, 0.0)).xyz );
  }

  VertexOut.pass_ms_u = tangent;
  VertexOut.pass_ms_v = bitangent;
  VertexOut.pass_normal = normal;
  gl_Position = vec4(new_in_position, 1.0);

}
