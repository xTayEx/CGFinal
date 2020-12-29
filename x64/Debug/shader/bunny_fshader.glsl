#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int reflectSwitch;
uniform samplerCube skybox;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 I = normalize(Position - viewPos);

    float ratio = 1.00 / 1.52;
    vec3 R;
    if (reflectSwitch == 1) {
        R = reflect(I, normalize(Normal));
    } else {
        R = refract(I, normalize(Normal), ratio);
    }
    
    vec3 color = texture(skybox, R).rgb;
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - Position);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 1;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow                
    vec3 lighting = color + (ambient + diffuse + specular * 2) * 0.32;    
     
    FragColor = vec4(lighting, 1.0);
}