#version 150
// transformation matrices
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

#if __VERSION__ >= 130

// vertex position attributes
in vec2 vertex_pos2d;

// multicolor attribute
in vec4 multi_color;
out vec4 out_multi_color;

#else

// vertex position attribute
attribute vec2 vertex_pos2d;

// multicolor attribute
attribute vec3 multi_color;
varying vec4 out_multi_color;

#endif

void main()
{
  // process color
  out_multi_color = multi_color;
  // process vertex
  gl_Position = projection_matrix * modelview_matrix * vec4(vertex_pos2d.x, vertex_pos2d.y, 0.0, 1.0);
}
