#version 400

//Variables en entree
in vec3 normal;
in vec4 color;
in vec2 uv;
in vec3 wPos;
flat in float type;

uniform float time;
uniform sampler2D colorTex1;
uniform vec3[20] ondes;
out vec4 color_out;
uniform vec3 sun_pos;
uniform vec4 sun_color; 
uniform vec3 cam_pos;

uniform float ondesSize;
in vec2 vertex_pos;
//Globales
const float ambientLevel = 0.4;

#define CUBE_EAU 5.0

#define WAVEFORCE 0.4f;
float noiseWater(vec3 v){
	vec2 dir = vec2(v.x,v.y);

	float l = dot(vec2(0.3,1),dir);
	float n = (sin(l*2+time*2)+1)/2; //1

	l = dot(vec2(1,0.3),dir);
	n += (sin(l*2+time*2)+1)/2; //1

	l = dot(vec2(-0.8,0.8),dir);
	n += (sin(l*2+time*2)+1)/2; //1

	l = dot(vec2(0.5,-0.8),dir);
	n += (sin(l+time*4)+1)/4; // 0.5

	l = dot(vec2(-0.5,-0.2),dir);
	n += (sin(l*4+time*4)+1)/8; // 0.25

	l = dot(vec2(0.2,-0.8),dir);
	n += (sin(l*4+time*4)+1)/8; // 0.25
	return n/4;
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


	float firstOnde = 0;
	float firstDist = 0;
	vec3 firstChoc = vec3(0,0,0);

	// Update normal
	if(type == CUBE_EAU){

		float totalOnde = 0;
		uint nbOndes = 0;
		vec3 shoc = vec3(0,0,0);
		float dist = 0;

		//float dist = 1-step(distance,mod(time,2)*5 + 0.1) * (1-step(distance,mod(time,2)*5));
		//float dist = step(distance,mod(time,2)*5);
		for(int i = 0; i < min(20,ondesSize); ++i){
			float distance_float = sqrt(pow(ondes[i].x-vertex_pos.x,2)+pow(ondes[i].y-vertex_pos.y,2)/*+pow(distance_vec.z-1,2)*/);
			vec3 distance = vec3(ondes[i].xy,0)-vec3(vertex_pos,0);
			vec3 normalvec = normalize(distance);
			float onde_point = min(2,time-ondes[i].z)*5;
			//float dist = 1- min(1,abs(distance_float - onde_point)*1);
			float onde = max(-1,min(1,distance_float - onde_point));
			
			totalOnde += onde;
			float actualDist = (1 - abs(onde))*(1-(onde_point/10));
			dist += actualDist;
			vec3 inverse_vec = vec3(-normalvec.xy,0);
			shoc += (inverse_vec * min(1,max(0,onde)*2) + normalvec * min(1,max(0,-onde)*2) + vec3(0,0,1)*actualDist)*actualDist; 
			++nbOndes;
			if(i <= 1) {
				firstDist = dist;
				firstOnde = onde;
				firstChoc = shoc;
			}
		}
		totalOnde = totalOnde/nbOndes;
		shoc = vec3(min(shoc.x,1),min(shoc.y,1),min(shoc.z,1));

		vec3 A = wPos;
		A.z -= noiseWater(A) * WAVEFORCE;

		vec3 B = wPos + vec3(0.2,0,0);
		B.z -= noiseWater(B) * WAVEFORCE;

		vec3 C = wPos + vec3(0,0.2,0);
		C.z -= noiseWater(C) *  WAVEFORCE;

		N = normalize(cross(normalize(B-A),normalize(C-A)));
		N = normalize(N * (1-dist) + shoc); 
		//N = shoc;
		float fresnel = pow(max(0,dot(N,V)),0.7f) *0.5f;

		texture.a = 0.8 * ( 1 - fresnel);

	}
	// diffuse
	float diff = max(0.05f,dot(L,N));
	texture.rgb *= diff;
	// specular
	vec3 H = normalize(L+V);
	float spec = abs(dot(N,H));
	spec = 5 * pow(spec,100) * specForce;

	texture.rgb += sun_color.xyz * spec * diff;
	// ambiant
	texture.rgb += 0.01 * sun_color.xyz * (1-diff);
	color_out = vec4(sqrt(texture.rgb),texture.a);

}