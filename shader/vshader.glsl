#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 projection;
uniform mat4 model_view;
uniform mat4 object_transformation = mat4(1.0);

attribute vec3 a_position;
attribute vec3 a_normal;

varying vec4 vertex;
varying vec4 normal;
varying vec4 lightPos;

void main()
{
  lightPos = model_view * vec4(0.0, -6.0, 10.0, 1.0);

  vertex =  model_view * object_transformation * vec4(a_position, 1.0);
  gl_Position = projection * vertex;
//  normal = transpose(inverse(model_view)) * vec4(a_normal, 0.0);
  normal = model_view * vec4(a_normal, 0.0);
}
