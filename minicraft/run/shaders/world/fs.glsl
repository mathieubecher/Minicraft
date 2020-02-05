#version 400

//Variables en entree
in vec3 normal;
in vec4 color;
in vec2 uv;

uniform sampler2D colorTex1;

out vec4 color_out;
uniform vec3 sun_pos; 

//Globales
const float ambientLevel = 0.4;

void main()
{
	vec4 texture = texture(colorTex1, uv);
	if(texture.r == texture.g && texture.r==texture.b) 
		texture = texture * color;
	vec3 toLight = sun_pos; //normalize(vec3(0.2,0.5,1));
	color_out = vec4(texture.rgb * max(0.5,dot(toLight,normal)*1.4) * 0.97 + 0.03 * vec3(0.8,0.9,1),texture.a);
}