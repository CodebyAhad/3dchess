#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;

out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(){
    vec4 world = model * vec4(aPos, 1.0);
    fragPos  = world.xyz;
    normal   = mat3(transpose(inverse(model))) * aNorm;
    gl_Position = proj * view * world;
}
