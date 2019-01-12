// transformation matrices
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

void main()
{
  // process vertex
  gl_Position = projection_matrix * modelview_matrix * gl_Vertex;
}
