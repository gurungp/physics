#version 330 

in vec3 Normal;
in vec3 Position;
in vec3 FragPos;

out vec4 color;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform samplerCube skybox;
uniform float RenderWithLight;

const float EtaR = 0.65;
const float EtaG = 0.67;
const float EtaB = 0.69;
const float F = ((1.0-EtaG) * (1.0-EtaG)) / ((1.0+EtaG) * (1.0+EtaG));
const vec3 lightColor = vec3(1.0,1.0,1.0);


void main()
{        
	vec3 incident = normalize(Position - cameraPos);
	vec3 N = normalize(Normal);
	vec3 Reflect = reflect(incident,N);
	vec3 RefractR = refract(incident,N, EtaR);
	vec3 RefractG = refract(incident,N, EtaG);
	vec3 RefractB = refract(incident,N, EtaB);

	vec4 refractColor;
	refractColor.r = texture(skybox, RefractR).r;
	refractColor.g = texture(skybox, RefractG).g;
	refractColor.b = texture(skybox, RefractB).b;

	vec4 reflectColor = texture(skybox, Reflect);

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

	float Ratio = F + (1.0 - F) * pow((1.0 - dot(-incident,N)), 5.0);
 	
 	if(RenderWithLight==0){
  		color = mix(refractColor, reflectColor, Ratio);
    }else{ 
   		vec4 result_color = mix(refractColor, reflectColor, Ratio);
  		vec3 result = diffuse + specular + result_color.xyz;
  		color = result_color * vec4(result, 1.0);
    }


 
}