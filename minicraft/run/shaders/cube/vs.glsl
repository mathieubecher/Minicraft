#version 400

uniform mat4 mvp;
uniform vec4 cube_color;

layout(location=0) in vec3 vs_position_in;
layout(location=1) in vec3 vs_normal_in;
layout(location=2) in vec2 vs_uv_in;

out vec4 color_out_vs;

void main()
{
	vec4 vecIn = vec4(vs_position_in,1.0);
	gl_Position = mvp * vecIn;
	color_out_vs = vec4(cube_color.xyz * max(0.1,0.4 * vs_normal_in.x + 0.85 * vs_normal_in.z + 0.2*vs_normal_in.y) ,cube_color.a);
}