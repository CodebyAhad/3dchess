#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec2 aUV;
layout(location=3) in float aColorId;

out vec3 fragPos;
out vec3 normal;
out vec2 uv;
out float colorId;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(){
    vec4 world = model * vec4(aPos, 1.0);
    fragPos  = world.xyz;
    normal   = mat3(transpose(inverse(model))) * aNorm;
    uv       = aUV;
    colorId  = aColorId;
    gl_Position = proj * view * world;
}
