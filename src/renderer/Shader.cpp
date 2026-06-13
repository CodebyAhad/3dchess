#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader() {
    if (id) { glDeleteProgram(id); id = 0; }
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "[Shader] Cannot open: " << path << "\n"; return ""; }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);
        std::cerr << "[Shader] Compile error:\n" << log << "\n";
    }
    return s;
}

bool Shader::loadFromSource(const char* vertSrc, const char* fragSrc) {
    GLuint vert = compileShader(GL_VERTEX_SHADER,   vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    id = glCreateProgram();
    glAttachShader(id, vert); glAttachShader(id, frag);
    glLinkProgram(id);
    glDeleteShader(vert); glDeleteShader(frag);
    GLint ok; glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetProgramInfoLog(id, 1024, nullptr, log);
        std::cerr << "[Shader] Link error:\n" << log << "\n";
        return false;
    }
    return true;
}

bool Shader::loadFromFiles(const std::string& vp, const std::string& fp) {
    std::string vs = readFile(vp), fs = readFile(fp);
    if (vs.empty() || fs.empty()) return false;
    return loadFromSource(vs.c_str(), fs.c_str());
}
