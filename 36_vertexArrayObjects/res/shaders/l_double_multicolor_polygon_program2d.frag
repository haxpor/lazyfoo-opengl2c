#version 150

// multicolor attribute
in vec4 multicolor;

// final color
out vec4 final_color;

void main()
{
  // set fragment color
  final_color = multicolor;
}
