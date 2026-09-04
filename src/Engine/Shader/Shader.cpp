#include "Shader.h"
#include "../../Utils/Logger/Logger.h"
#include <stdexcept>

std::string ShaderTypeToString(ShaderType type) {
    switch (type) {
        case ShaderType::COMPUTE:
            return "COMPUTE";
        case ShaderType::VERTEX:
            return "VERTEX";
        case ShaderType::TESS_CONTROL:
            return "TESS_CONTROL";
        case ShaderType::TESS_EVAL:
            return "TESS_EVALUATION";
        case ShaderType::GEOMETRY:
            return "GEOMETRY";
        case ShaderType::FRAGMENT:
            return "FRAGMENT";
        default:
            LOG("ERROR::SHADER_TYPE_TO_STRING unknown type specified");
            return "UNKNOWN";
    }
}

GLenum ShaderTypeToGL(ShaderType type) {
    switch (type) {
        case ShaderType::COMPUTE:
            return GL_COMPUTE_SHADER;
        case ShaderType::VERTEX:
            return GL_VERTEX_SHADER;
        case ShaderType::TESS_CONTROL:
            return GL_TESS_CONTROL_SHADER;
        case ShaderType::TESS_EVAL:
            return GL_TESS_EVALUATION_SHADER;
        case ShaderType::GEOMETRY:
            return GL_GEOMETRY_SHADER;
        case ShaderType::FRAGMENT:
            return GL_FRAGMENT_SHADER;
        default:
            LOG("ERROR::SHADER_TYPE_TO_GL unknown type specified");
            throw std::invalid_argument("Unknown shader type");
    }
}

Shader::Shader(ShaderType type) : ID(0), type(type), error(false) {
}

void Shader::AddSource(const std::string &source) {
    if(IsValid()) {
        LOG("WARNING::SHADER_ADD_SOURCE Shader has been already initialized, operation aborted");
        return;
    }
    Compile(source);
}


Shader::Shader(const std::string &source, ShaderType type) : type(type), error(false){
    Compile(source);
}

Shader::~Shader() {
    if(IsValid())
        glDeleteShader(ID);
}

Shader::Shader(Shader &&other) noexcept : ID(other.ID), type(other.type), error(other.error) {
    other.ID = 0;
}

Shader& Shader::operator=(Shader &&other) noexcept {
    if(this == &other) return *this;

    if(IsValid())
        glDeleteShader(ID);

    ID = other.ID;
    type = other.type;
    error = other.error;
    other.ID = 0;
    other.error = false;

    return *this;
}

void Shader::AttachShader(GLuint program, bool &success) const {
    success = IsValid() && !error;

    if(success)
        glAttachShader(program, ID);
}

void Shader::Compile(const std::string &source) {
    const char* sourceStr = source.c_str();
    ID = glCreateShader(ShaderTypeToGL(type));
    glShaderSource(ID, 1, &sourceStr, nullptr);
    glCompileShader(ID);
    CheckCompileErrors();
}

void Shader::CheckCompileErrors() const {
    int success;
    char infoLog[SHADER_INFO_LOG_SIZE];
    glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
    if(!success) {
        error = true;
        glGetShaderInfoLog(ID, SHADER_INFO_LOG_SIZE, nullptr, infoLog);
        LOG("ERROR::SHADER_COMPILATION failed of type: %s\n %s\n -- --------------------------------------------------- -- ", ShaderTypeToString(type).c_str(), infoLog);
    }
}

bool Shader::IsValid() const {
    return ID != 0;
}
