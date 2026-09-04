#include <algorithm>
#include "ShaderProgram.h"
#include "../../Utils/Logger/Logger.h"
#include "Shader.h"

ShaderProgram::ShaderProgram() : compiled(false), error(false) {
    ID = glCreateProgram();
    uniforms.SetShaderProgram(ID);
}

ShaderProgram::~ShaderProgram() {
    attachments.clear();
    uniformBlocks.clear();
    glDeleteProgram(ID);
}

ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept {
    if(!other.IsValid()) {
        ID = 0;
        compiled = false;
        error = false;
        uniforms.SetShaderProgram(0);
        uniformBlocks.clear();
    } else {
        ID = other.ID;
        attachments = std::move(other.attachments);
        uniformBlocks = std::move(other.uniformBlocks);
        compiled = other.compiled;
        error = other.error;
        uniforms.SetShaderProgram(ID);

        other.ID = 0;
        other.compiled = false;
        other.error = false;
        other.uniforms.SetShaderProgram(0);
    }
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram &&other) noexcept {
    if(this == &other) return *this;

    attachments.clear();
    glDeleteProgram(ID);
    compiled = false;
    error = false;
    uniformBlocks.clear();

    if(!other.IsValid()) {
        ID = 0;
        compiled = false;
        error = false;
        uniforms.SetShaderProgram(0);
    } else {
        ID = other.ID;
        attachments = std::move(other.attachments);
        uniformBlocks = std::move(other.uniformBlocks);
        compiled = other.compiled;
        error = other.error;
        uniforms.SetShaderProgram(ID);

        other.ID = 0;
        other.compiled = false;
        other.error = false;
        other.uniforms.SetShaderProgram(0);
    }

    return *this;
}

void ShaderProgram::AttachShader(const Shader &shader) {
    const ShaderType& shaderType = shader.GetType();
    auto it = std::find(attachments.begin(), attachments.end(), shaderType);
    if(it == attachments.end()) {
        bool success;
        shader.AttachShader(ID, success);
        if(success) {
            attachments.push_back(shaderType);
        } else {
            LOG("ERROR::SHADER_PROGRAM_ATTACHMENT failed of type: %s\n Shader attachment failed, shader was not valid or had erros", ShaderTypeToString(shaderType).c_str());
        }
    } else {
        LOG("ERROR::SHADER_PROGRAM_ATTACHMENT failed of type: %s\n Another shader of the same type has been already attached", ShaderTypeToString(shaderType).c_str());
    }
}

void ShaderProgram::Compile() {
    compiled = true;
    glLinkProgram(ID);
    CheckLinkErrors();
}

void ShaderProgram::Bind() const {
    if(error || !HasBeenCompiled() || !IsValid()) {
        LOG("WARNING::SHADER_PROGRAM_BIND is not suitable for using");
        return;
    }
    glUseProgram(ID);
}

void ShaderProgram::Unbind() {
    glUseProgram(0);
}

void ShaderProgram::BindUniformBlock(GLuint bindingPoint, const std::string &blockName) {
    if(!IsValid()) {
        LOG("WARNING_SHADER_PROGRAM_BIND_UNIFORM_BLOCK shader program is not valid, uniform block binding aborted");
        return;
    }
    if(!HasBeenCompiled()) {
        LOG("WARNING_SHADER_PROGRAM_BIND_UNIFORM_BLOCK shader program has not been compiled yet, uniform block binding aborted");
        return;
    }
#ifdef DEBUG
    for(const auto &binding : uniformBlocks) {
        if(bindingPoint == binding.bindingPoint)
            LOG("WARNING::SHADER_PROGRAM_BIND_UNIFORM_BLOCK uniform block of name: %s is already binded to binding point: %u, overwrite binding point with uniform block of name: %s", binding.uniformBlockName.c_str(), static_cast<unsigned int>(bindingPoint), blockName.c_str());
        if(blockName == binding.uniformBlockName)
            LOG("WARNING::SHADER_PROGRAM_BIND_UNIFORM_BLOCK there's already a uniform block of name: %s binded in the shader program", binding.uniformBlockName.c_str());
    }
#endif
    GLuint blockIndex = glGetUniformBlockIndex(ID, blockName.c_str());
    if(blockIndex == GL_INVALID_INDEX) {
        LOG("ERROR::SHADER_PROGRAM::BIND_UNIFORM_BLOCK block index invalid, operation aborted");
        return;
    }
    glUniformBlockBinding(ID, blockIndex, bindingPoint);
    uniformBlocks.push_back(ShaderUniformBlockBinding{bindingPoint, blockIndex, blockName});
}

void ShaderProgram::CheckLinkErrors() const {
    int success;
    char infoLog[SHADER_INFO_LOG_SIZE];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success) {
        error = true;
        glGetProgramInfoLog(ID, SHADER_INFO_LOG_SIZE, nullptr, infoLog);
        LOG("ERROR::SHADER_PROGRAM_LINKING failed:\n%s", infoLog);
    }
}

bool ShaderProgram::HasBeenCompiled() const {
    return compiled;
}

bool ShaderProgram::IsValid() const {
    return ID != 0;
}
