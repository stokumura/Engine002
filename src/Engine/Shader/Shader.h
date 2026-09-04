#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>

const unsigned int SHADER_INFO_LOG_SIZE = 1024;

enum class ShaderType {
    COMPUTE,
    VERTEX,
    TESS_CONTROL,
    TESS_EVAL,
    GEOMETRY,
    FRAGMENT
};

std::string ShaderTypeToString(ShaderType type);
GLenum ShaderTypeToGL(ShaderType type);

class Shader {
    private:
        GLuint ID;
        ShaderType type;

        mutable bool error;

    public:
        Shader(ShaderType type);
        void AddSource(const std::string &source);

        Shader(const std::string &source, ShaderType type);
        ~Shader();

        [[deprecated("WARNING::SHADER_COPY_CTOR cannot be coppied")]]
        Shader(const Shader &other) = delete;

        [[deprecated("WARNING::SHADER_COPY_ASSIGNMENT cannot be coppied")]]
        Shader& operator=(const Shader &other) = delete;

        Shader(Shader &&other) noexcept;
        Shader& operator=(Shader &&other) noexcept;

        void AttachShader(GLuint program, bool &success) const;

        const ShaderType &GetType() const { return type; }

    private:
        void Compile(const std::string &source);
        void CheckCompileErrors() const;

        bool IsValid() const;
};
