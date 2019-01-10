#version 120
// plain polygon color
uniform vec4 polygon_color = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
  // set fragment
  gl_FragColor = polygon_color;
}
