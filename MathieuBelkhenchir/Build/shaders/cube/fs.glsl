#version 400

in vec4 color_out_vs;

out vec4 color_out;

void main()
{
	color_out = vec4(sqrt(color_out_vs.rgb),1);
}