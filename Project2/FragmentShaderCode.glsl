#version 430

in vec2 UV;
uniform sampler2D myTextureSampler0;

out vec3 Color0;

void main()
{
	Color0 = texture( myTextureSampler0, UV ).rgb;
}
