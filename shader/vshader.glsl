#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform mat4 normal_matrix;

attribute vec3 a_position;
attribute vec3 a_color;
attribute vec3 a_normal;

varying vec3 vertex;
varying vec4 normal;

void main()
{
  gl_Position = mvp_matrix * vec4(a_position, 1);
  vertex = gl_Position.xyz;
  normal = (normal_matrix * vec4(a_normal, 0.0));
}
