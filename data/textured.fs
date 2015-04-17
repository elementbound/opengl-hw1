#version 440

in 	vec3 fragPosition;
in 	vec3 fragNormal;
in	vec2 fragTexcoord;

out vec4 outColor;

uniform vec3 uAmbient;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform sampler2D uTexture;

void main()
{
	vec3 normal = normalize(fragNormal);
	vec3 delta = fragPosition - uLightPos;
	delta = normalize(delta);

	vec3 light = clamp(dot(-normal, delta), 0.0, 1.0) * uLightColor;
		 light += uAmbient;
	outColor = vec4(texture(uTexture, fragTexcoord).rgb, 1.0) * vec4(light, 1.0);
}