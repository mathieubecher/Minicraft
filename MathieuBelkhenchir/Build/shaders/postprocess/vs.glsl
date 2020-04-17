#version 400

layout(location=0) in vec3 vs_position_in;

out vec2 uv;

void main()
{
	gl_Position = vec4(vs_position_in,1.0);
	uv = (vs_position_in.xy+vec2(1,1))/2;
}