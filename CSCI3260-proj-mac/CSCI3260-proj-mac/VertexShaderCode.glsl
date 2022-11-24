#version 330

layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUV;
layout(location=2) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

void main()
{
    vec4 v = vec4(position, 1.0);
    vec4 newPosition = modelMatrix * v;
    vec4 out_position = projectionMatrix * viewMatrix * newPosition;
    gl_Position = out_position;
    UV = vertexUV;

    vec4 normal_temp = modelMatrix * vec4(normal, 0);
    normalWorld = normal_temp.xyz;

    vertexPositionWorld = newPosition.xyz;
}

