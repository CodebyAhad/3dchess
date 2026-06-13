#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    GLuint id = 0;

    Shader() = default;
    ~Shader();

    
    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    
    bool loadFromSource(const char* vertSrc, const char* fragSrc);

    void use()   const { glUseProgram(id); }
    void unuse() const { glUseProgram(0);  }

    void setInt  (const char* name, int v)               const { glUniform1i(loc(name), v); }
    void setFloat(const char* name, float v)             const { glUniform1f(loc(name), v); }
    void setVec3 (const char* name, const glm::vec3& v)  const { glUniform3fv(loc(name), 1, glm::value_ptr(v)); }
    void setVec4 (const char* name, const glm::vec4& v)  const { glUniform4fv(loc(name), 1, glm::value_ptr(v)); }
    void setMat4 (const char* name, const glm::mat4& v)  const { glUniformMatrix4fv(loc(name), 1, GL_FALSE, glm::value_ptr(v)); }

private:
    GLint loc(const char* name) const { return glGetUniformLocation(id, name); }
    static GLuint compileShader(GLenum type, const char* src);
    static std::string readFile(const std::string& path);
};
