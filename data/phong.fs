#version 440

struct Light 
{
	int type;
	
	vec3 position;
	vec3 color;
	
	vec3 direction;
	float thresh;
};

in  vec3 fragPosition;
in  vec3 fragNormal;
in  vec2 fragTexcoord;

out vec4 outColor;

uniform Light uLights[5];
uniform sampler2D uTexture;

vec3 eval_light(in Light l, in vec3 pos, in vec3 normal)
{
	if(l.type == 1) // point light
	{
		vec3 delta = pos - l.position;
		delta = normalize(delta);
		
		return l.color * clamp(dot(-normal, delta), 0.0, 1.0);
	}
	else if(l.type == 2) //spot light
	{
		vec3 delta = pos - l.position;
		delta = normalize(delta);
		
		if(dot(delta, l.direction) > l.thresh)
			return l.color * clamp(dot(-normal, delta), 0.0, 1.0);
		else
			return vec3(0.0);
	}
	else //unknown/inactive
		return vec3(0.0);
}

void main()
{
	vec3 light_sum = vec3(0.0);
	vec3 normal = normalize(fragNormal);
	
	for(int i=0; i<5; i++)
		light_sum += eval_light(uLights[i], fragPosition, normal);
		
	outColor = texture2D(uTexture, fragTexcoord) * vec4(light_sum, 1.0);
}