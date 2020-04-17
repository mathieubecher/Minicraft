#version 400

in vec2 uv;

uniform sampler2D TexColor;
uniform sampler2D TexDepth;
uniform float screen_width;
uniform float screen_height;
uniform vec2 near_far;
uniform vec4 sky_color;
out vec4 color_out;

float LinearizeDepth(float z)
{
	float n = near_far.x; // camera z near
  	float f = near_far.y; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( TexColor , uv );
	float depth = texture2D( TexDepth , uv ).r;	
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);
	//color.rgb = vec3(depth,depth,depth)*5;

	// Blur
	float size = 5;
	float nbPixels = 9;
	vec3 colorBlur = vec3(0,0,0);

	for(float x = -size;x<size; x++){
		for(float y = -size; y<=size; y++){
			nbPixels++;
			vec2 delta = vec2(x*xstep,y*ystep);
			colorBlur += texture2D(TexColor,uv + delta).rgb;
		}
	}

	colorBlur /= nbPixels;
	
	float depthCenter = texture2D(TexDepth, vec2(0.5,0.5)).r;
	depthCenter = LinearizeDepth(depthCenter);

	color.rgb = mix(color.rgb,colorBlur, min(1,abs(depthCenter - depth)*8));
	
	// Fog
	color.rgb = mix(color.rgb,sky_color.rgb,pow(min(1,depth*3),2));
	
	// Sobel
	/*
	vec3 colorSobel = vec3(0,0,0);
	float depthSobel = 0;
	float nbPixels = 0;
	float size = 1;
	for(float x = -size;x<size; x++){
		for(float y = -size; y<=size; y++){
			nbPixels++;
			vec2 delta = vec2(x*xstep,y*ystep)*1;
			colorSobel += texture2D(TexColor,uv + delta).rgb;
			depthSobel += LinearizeDepth(texture2D(TexDepth, uv + delta).r);
		}
	}
	colorSobel -= nbPixels * color.rgb;
	depthSobel -= nbPixels * depth;
	//color.rgb = abs(vec3(colorSobel.r,colorSobel.r,colorSobel.r));
	color.rgb += abs(vec3(depthSobel,depthSobel,depthSobel));
	//color.rgb = vec3(depth,depth,depth);
	*/
    //Gamma correction
    //color.r = pow(color.r,1.0/2.2);
    //color.g = pow(color.g,1.0/2.2);
    //color.b = pow(color.b,1.0/2.2);
	//color.rgb = vec3(depth,depth,depth)*4;
	color_out = vec4(color.rgb,1.0);
}