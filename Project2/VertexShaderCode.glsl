#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 out_position = projectionMatrix * viewMatrix * modelMatrix * v;
	gl_Position = out_position;
	UV = vertexUV;
}
