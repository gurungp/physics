#version 330 

in vec3 Normal;
in vec3 Position;
in vec3 FragPos;
in vec3 BoxColor;
out vec4 color;


uniform vec3 lightPos;


void main()
{        
	//Object Color
	//vec3 objectColor = vec3(0.2,0.72,0.9);
	vec3 objectColor = BoxColor;

	//Ambient
	vec3 ambientColor  = 0.8f * vec3(0.5,0.6,0.6);

	//Diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(0.5,0.6,0.6);

	vec3 result = (ambientColor + diffuse) * objectColor;
  	color = vec4(objectColor, 1.0f);
}