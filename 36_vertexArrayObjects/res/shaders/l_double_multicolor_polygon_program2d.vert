#version 150

// transformation matrices
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

// vertex position attribute
in vec2 vertex_pos2d;

// multicolor attribute
in vec4 multicolor1;
in vec4 multicolor2;

out vec4 multicolor;

void main()
{
  // process color
  multicolor = multicolor1 * multicolor2;

  // process vertex
  gl_Position = projection_matrix * modelview_matrix * vec4(vertex_pos2d.x, vertex_pos2d.y, 0.0, 1.0);
}
