#version 150

// texture color
// to be applied to texture's pixels
uniform vec4 texture_color;

// texture unit
uniform sampler2D texture_sampler;

// texture coordinate
in vec2 outin_texcoord;

// final color
out vec4 final_color;

void main()
{
  final_color = texture(texture_sampler, outin_texcoord) * texture_color;
}
