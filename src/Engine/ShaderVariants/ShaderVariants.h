#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "../Shader/Shader.h"
#include "ShaderDefines.h"

class ShaderVariants {
    private:
        std::string path;
        std::string source;

        ShaderType type;

        std::unordered_map<uint32_t, Shader> variants;
    public:
        ShaderVariants(const std::string &path, ShaderType type);
        ~ShaderVariants() = default;

        [[deprecated("WARNING::SHADER_VARIANTS_COPY_CTOR cannot be coppied")]]
        ShaderVariants(const ShaderVariants&) = delete;

        [[deprecated("WARNING::SHADER_VARIANTS_ASSIGNMENT cannot be coppied")]]
        ShaderVariants& operator=(const ShaderVariants&) = delete;

        ShaderVariants(ShaderVariants &&other) noexcept;
        ShaderVariants& operator=(ShaderVariants &&other) noexcept;

        const Shader &GetShader(const std::vector<std::string> &defines);
        const Shader &GetBaseShader();
    private:
        const Shader &InitBaseShader();

        std::string InjectDefines(const std::string &source, const std::vector<std::string> &defines) const;

        bool IsValid() const;
};
