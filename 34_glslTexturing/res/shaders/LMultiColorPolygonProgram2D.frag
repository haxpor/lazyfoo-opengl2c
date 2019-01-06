#version 150
#if __VERSION__ >= 130

// multi color attribute
in vec4 out_multi_color;

// final color
out vec4 out_color;

#else

// multicolor attribute
varying vec4 out_multi_color;

#endif

void main()
{
#if __VERSION__ >= 130
  // set fragment
  out_color = out_multi_color;
#else
  gl_FragColor = out_multi_color;
#endif
}
