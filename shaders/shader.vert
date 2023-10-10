#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

const float AMBIENT = 0.02f;

void main()
{
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * push.modelMatrix * vec4(position, 1.f);

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);
	
	fragColor = lightIntensity * color;
	fragTexCoord = uv;
}