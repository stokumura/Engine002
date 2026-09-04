#pragma once

#include "Shader.h"
#include "ShaderUniforms.h"
#include "UniformBlock.h"

#include <vector>

class ShaderProgram {
    private:
        GLuint ID;
        std::vector<ShaderType> attachments;
        std::vector<ShaderUniformBlockBinding> uniformBlocks;
        ShaderUniforms uniforms;

        bool compiled = false;
        mutable bool error = false;

    public:
        ShaderProgram();
        ~ShaderProgram();

        void AttachShader(const Shader &shader);
        void Compile();

        [[deprecated("WARNING::SHADER_PROGRAM_COPY_CTOR cannot be coppied")]]
        ShaderProgram(const ShaderProgram &other) = delete;

        [[deprecated("WARNING::SHADER_PROGRAM_ASSIGNMENT cannot be coppied")]]
        ShaderProgram& operator=(const ShaderProgram &other) = delete;
        
        ShaderProgram(ShaderProgram &&other) noexcept;
        ShaderProgram& operator=(ShaderProgram &&other) noexcept;

        void Bind() const;
        static void Unbind();

        void SetBool(const std::string &name, bool value) const;
        void SetInt(const std::string &name, int value) const;
        void SetFloat(const std::string &name, float value) const;
        void SetVec2(const std::string &name, const glm::vec2 &value) const;
        void SetVec2(const std::string &name, float x, float y) const;
        void SetVec3(const std::string &name, const glm::vec3 &value) const;
        void SetVec3(const std::string &name, float x, float y, float z) const;
        void SetVec4(const std::string &name, const glm::vec4 &value) const;
        void SetVec4(const std::string &name, float x, float y, float z, float w) const;
        void SetMat2(const std::string &name, const glm::mat2 &mat) const;
        void SetMat3(const std::string &name, const glm::mat3 &mat) const;
        void SetMat4(const std::string &name, const glm::mat4 &mat) const;

        void BindUniformBlock(GLuint bindingPoint, const std::string &blockName);
    private:
        void CheckLinkErrors() const;

        bool HasBeenCompiled() const;
        bool IsValid() const;
};
