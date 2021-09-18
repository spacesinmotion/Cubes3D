#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform vec4 objectColor;
uniform bool useLight;

varying vec4 vertex, normal, lightPos;

void main()
{
  vec3 Idiff = vec3(1.0);
  if (useLight)
  {
    vec3 lightColor = vec3(1,1,1);

    vec3 L = normalize(lightPos - vertex).xyz;
    float dens = max(dot(normalize(normal.xyz),L), 0.0);
    if (dens > 0.7)
      dens = 1.0;
    else if (dens > 0.3)
      dens = 0.7;
    else 
      dens = 0.3;
    Idiff = lightColor * dens;
    Idiff = clamp(Idiff, vec3(0.0), vec3(1.0));
  }

  gl_FragColor = objectColor * vec4(Idiff,1.0);
}
