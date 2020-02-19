#version 400

uniform float elapsed;
uniform mat4 mvp;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

uniform mat4 nmat;
uniform float time;

uniform sampler2D colorTex1;


layout(location=0) in vec3 vs_position_in;
layout(location=1) in vec3 vs_normal_in;
layout(location=2) in vec2 vs_uv_in;
layout(location=3) in float vs_type_in;

//Variables en sortie
out vec3 normal;
out vec4 color;
out vec2 uv;
flat out float type;
out vec3 wPos;

#define CUBE_HERBE 1.0
#define CUBE_EAU 5.0
#define WAVEFORCE 0.2;
float noiseWater(vec4 v){
	vec2 dir = vec2(v.x,v.y);

	float l = dot(vec2(0.3,1),dir);
	float n = (sin(l*2+time)+1)/2; //1

	l = dot(vec2(1,0.3),dir);
	n += (sin(l*2+time)+1)/2; //1

	l = dot(vec2(0.5,-0.8),dir);
	n += (sin(l+time*2)+1)/4; // 0.5

	l = dot(vec2(-0.5,-0.2),dir);
	n += (sin(l*4+time*2)+1)/8; // 0.25
	return n/2.75;
}
void main()
{
	color = vec4(1.0,1.0,1.0,1.0);

	if(vs_type_in == CUBE_HERBE)
		color = vec4(154/255.0,255/255.0,87/255.0,1);

	vec4 vecIn = vec4(vs_position_in,1.0);
	vec4 vecInW = m * vecIn;
	if(vs_type_in == CUBE_EAU){
		float wave = noiseWater(vecInW)*WAVEFORCE;
		vecInW.z -= wave;
	}

	wPos = vecInW.xyz;
	vec4 vecInV = v * vecInW;
	gl_Position = p * vecInV;
		
	normal = (nmat * vec4(vs_normal_in,1.0)).xyz; 

	//if(vs_type_in == CUBE_EAU) normal = normalWater(vecInW);
	
	uv = vs_uv_in;
	type = vs_type_in;

}
