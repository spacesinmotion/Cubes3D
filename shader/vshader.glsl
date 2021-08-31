#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 projection, model_view, normal_matrix;
uniform mat4 object_transformation = mat4(1.0);
uniform mat4 object_normal = mat4(1.0);
uniform vec4 light_pos = vec4(0.0, -2.0, 8.0, 1.0);

attribute vec3 a_position, a_normal;

varying vec4 vertex, normal, lightPos;

void main()
{
  lightPos = model_view * light_pos;

  vertex =  model_view * object_transformation * vec4(a_position, 1.0);
  gl_Position = projection * vertex;
  normal = normal_matrix * object_normal * vec4(a_normal, 0.0);
}
