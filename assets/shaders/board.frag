#version 330 core
in vec3 fragPos;
in vec3 normal;
in vec2 uv;
in float colorId;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;

void main(){
    
    vec3 lightSquare = vec3(0.90, 0.82, 0.68);
    vec3 darkSquare  = vec3(0.30, 0.18, 0.10);
    vec3 borderColor = vec3(0.16, 0.10, 0.07);

    vec3 baseColor = (colorId < 0.25) ? lightSquare :
                     (colorId > 0.75) ? darkSquare  : borderColor;

    
    vec3 norm = normalize(normal);
    float diff = max(dot(norm, normalize(lightDir)), 0.0);

    
    vec2 centered = abs(uv - 0.5);
    float vignette = 1.0 - 0.12 * dot(centered, centered) * 8.0;

    vec3 color = (ambientColor + diff * lightColor) * baseColor * vignette;
    FragColor = vec4(color, 1.0);
}
