#version 450

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[]( 
 vec2(0.f, -0.5f),
 vec2(0.5f, -0.5f),
 vec2(-0.5f, -0.5f)
);

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.f, 0.f);	
	fragColor = vec3(positions[gl_VertexIndex], 0.f);
}