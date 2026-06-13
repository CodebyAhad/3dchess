#version 330 core
in vec3 fragPos;
in vec3 normal;

out vec4 FragColor;

uniform vec3 pieceColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;

void main(){
    vec3 norm    = normalize(normal);
    vec3 ld      = normalize(lightDir);
    vec3 viewDir = normalize(viewPos - fragPos);

    
    float diff = max(dot(norm, ld), 0.0);

    
    vec3 halfDir = normalize(ld + viewDir);
    float spec   = pow(max(dot(norm, halfDir), 0.0), 80.0);

    
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 3.0) * 0.15;

    vec3 color = ambientColor * pieceColor
               + diff * lightColor * pieceColor
               + spec * lightColor * 0.5
               + rim  * vec3(0.4, 0.5, 0.7);

    FragColor = vec4(color, 1.0);
}
