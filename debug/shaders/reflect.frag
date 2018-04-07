#version 330 core
in vec3 Normal;
in vec3 Position;
in vec3 FragPos;

out vec4 finalColor;

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform samplerCube skybox;

const vec3 lightColor = vec3(1.0,1.0,1.0);

void main()
{          

	//Diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm,lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	//Specular
	float specStrength = 0.8f;
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.8), 32);
	vec3 specular = specStrength * spec * lightColor;

    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
   	vec4 color = texture(skybox, R);
  	vec3 result = diffuse + specular + color.xyz;
  	finalColor = color * vec4(result, 1.0);

   // color = vec4(1.0,1.0,1.0,1.0);
}