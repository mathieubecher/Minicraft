#version 400

//Variables en entree
in vec3 normal;
in vec4 color;
in vec2 uv;
in vec3 wPos;
flat in float type;

uniform float time;
uniform sampler2D colorTex1;

out vec4 color_out;
uniform vec3 sun_pos;
uniform vec4 sun_color; 
uniform vec3 cam_pos;
//Globales
const float ambientLevel = 0.4;

#define CUBE_EAU 5.0

#define WAVEFORCE 0.4f;
float noiseWater(vec3 v){
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
	float specForce = 0;
	if(type == CUBE_EAU) specForce = 1;
	vec4 texture = texture(colorTex1, uv);
	texture.rgb =texture.rgb;

	// Apply color for gray
	if(texture.r == texture.g && texture.r==texture.b) 
		texture = texture * color;

	// Prepare texture for treatment
	texture.rgb = pow(texture.rgb,vec3(2,2,2));

	// Direct vector
	vec3 L = normalize(sun_pos);
	vec3 V = normalize(cam_pos - wPos);
	vec3 N = normal;

	// Update normal
	if(type == CUBE_EAU){
		vec3 A = wPos;
		A.z -= noiseWater(A) * WAVEFORCE;

		vec3 B = wPos + vec3(0.2,0,0);
		B.z -= noiseWater(B) * WAVEFORCE;

		vec3 C = wPos + vec3(0,0.2,0);
		C.z -= noiseWater(C) *  WAVEFORCE;

		N = normalize(cross(normalize(B-A),normalize(C-A)));

		float fresnel = pow(max(0,dot(N,V)),0.7f) *0.5f;

		texture.a *= 1 - fresnel;
	}
	// diffuse
	float diff = max(0.05f,dot(L,N));
	texture.rgb *= diff;
	// specular
	vec3 H = normalize(L+V);
	float spec = abs(dot(N,H));
	spec = 5f * pow(spec,100) * specForce;

	texture.rgb += sun_color.xyz * spec * diff;
	// ambiant
	texture.rgb += 0.01 * sun_color.xyz * (1-diff);

	color_out = vec4(sqrt(texture.rgb),texture.a);
	//color_out = vec4(N*10,1);
}