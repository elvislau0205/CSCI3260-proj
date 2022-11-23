#version 430

in vec2 UV;
uniform sampler2D myTextureSampler0;
uniform sampler2D myTextureSampler1;

in vec3 vertexPositionWorld;
in vec3 normalWorld;

uniform bool normalMapping_flag;
uniform bool light_flag;

out vec4 Color0;

struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct DirLight{
	vec3 direction;
	float intensity;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 viewDir, Material material){
	//ambient shading
	vec3 ambient = light.ambient * material.ambient;

	//diffuse shading
	vec3 norm = Normal;
	vec3 lightDir = normalize(-light.direction); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.intensity * light.diffuse * diff * material.diffuse;

	//specular shading
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.intensity * light.specular * spec * material.specular;

	return (ambient + clamp(diffuse, 0, 1) + specular);
}

vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 viewDir, Material material){
	//ambient shading
	vec3 ambient = light.ambient * material.ambient;

	//diffuse shading
	vec3 norm = Normal;
	vec3 lightDir = normalize(light.position - vertexPositionWorld);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.diffuse;

	//specular shading
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * material.specular;

	//attenuation
	float distance = length(light.position - vertexPositionWorld);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return attenuation * (ambient + clamp(diffuse, 0, 1) + specular);
}

Material general;
uniform PointLight pointLight;
uniform DirLight dirLight;
uniform vec3 eyePositionWorld;

void main()
{
	Color0 = texture( myTextureSampler0, UV );
	if(light_flag){
		vec3 viewDir = normalize(eyePositionWorld - vertexPositionWorld);

		general.ambient = texture( myTextureSampler0, UV ).rgb;
		general.diffuse = texture( myTextureSampler0, UV ).rgb;
		general.specular = vec3(0.3, 0.3, 0.3);
		general.shininess = 50;

		vec3 normal = normalize(normalWorld);

		if(normalMapping_flag){
			normal = texture( myTextureSampler1, UV ).rgb;
			normal = normalize(normal * 2.0 - 1.0);
		}

		vec3 result = CalcPointLight(pointLight,  normal,  viewDir, general );
		result += CalcDirLight(dirLight, normal, viewDir, general); 

		Color0 = vec4(result, 1.0);
	}
}
