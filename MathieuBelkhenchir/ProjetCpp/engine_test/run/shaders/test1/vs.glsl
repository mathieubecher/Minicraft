#version 400

uniform float elapsed;
uniform mat4 mvp;

layout(location=0) in vec3 vs_position_in;
layout(location=1) in vec4 vs_color_in;
layout(location=2) in vec3 vs_normal_in;

out vec4 color;

void main()
{
	gl_Position = mvp * vec4(vs_position_in,1.0);;
	color = vs_color_in;
}