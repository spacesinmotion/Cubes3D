attribute highp vec2 vertex;

uniform highp mat4 mvp;

void main()
{
  gl_Position = mvp * vec4(vertex,0,1);
}
