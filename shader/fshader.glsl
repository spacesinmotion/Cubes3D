#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec3 vertex;
varying vec4 normal;

void main()
{
  vec3 lightPos = vec3(0.0,2.0,0.0);
  vec3 lightColor = vec3(1,1,1);

  vec3 L = normalize(lightPos - vertex);
  vec3 Idiff = lightColor * max(dot(normal.xyz,L), 0.0);
  Idiff = clamp(Idiff, 0.2, 1.0);

  gl_FragColor = vec4(Idiff,1.0);
}
