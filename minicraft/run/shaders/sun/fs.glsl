#version 400

uniform vec3 sun_color;

out vec4 color_out;

void main()
{
	color_out = vec4(sqrt(sun_color),1);
}