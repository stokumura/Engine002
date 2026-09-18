#include "ShaderVariants.h"
#include "../../Utils/FileReader/FileReader.h"
#include "../../Utils/Logger/Logger.h"

ShaderVariants::ShaderVariants(const std::string &path, ShaderType type) : path(path), type(type) {
    source = FileReader::ReadFile(path);

    InitBaseShader();
}

ShaderVariants::ShaderVariants(ShaderVariants &&other) noexcept : path(std::move(other.path)), source(std::move(other.source)), type(other.type), variants(std::move(other.variants)) {
}

ShaderVariants& ShaderVariants::operator=(ShaderVariants &&other) noexcept {
    if(this == &other) return *this;

    variants.clear();

    path = std::move(other.path);
    source = std::move(other.source);
    type = other.type;
    variants = std::move(other.variants);

    return *this;
}

const Shader &ShaderVariants::GetShader(const std::vector<std::string> &defines) {
    if(!IsValid()) {
        LOG("ERROR::SHADER_VARIANTS::GET_SHADER source is empty, returning base empty Shader of type %s", ShaderTypeToString(type).c_str());
        return variants.at(0);
    }

    uint32_t hash = ShaderDefines::Hash(defines);
    auto it = variants.find(hash);
    if(it == variants.end()) {
        const std::string variantSource = InjectDefines(source, defines);
        it = variants.emplace(hash, Shader(variantSource, type)).first;
    }
    return it->second;
}

const Shader &ShaderVariants::GetBaseShader() {
    if(!IsValid()) {
        LOG("ERROR::SHADER_VARIANTS::GET_BASE_SHADER source is empty, returning base empty Shader of type %s", ShaderTypeToString(type).c_str());
    }
    return variants.at(0);
}

const Shader &ShaderVariants::InitBaseShader() {
    if(!IsValid()) {
        LOG("ERROR::SHADER_VARIANTS::INIT_BASE_SHADER source is empty, base shader is set to empty Shader of type %s", ShaderTypeToString(type).c_str());
        variants.emplace(0, Shader(type));
    } else {
        variants.emplace(0, Shader(source, type));
    }
    return variants.at(0);
}

std::string ShaderVariants::InjectDefines(const std::string &source, const std::vector<std::string> &defines) const {
    if(!IsValid()) {
        LOG("ERROR::SHADER_VARIANTS::INJECT_DEFINES source is empty, operation aborted");
        return "";
    }

    size_t versionLine = source.find("#version");
    if(versionLine == std::string::npos) {
        LOG("ERROR::SHADER_VARIANTS::INJECT_DEFINES source is invalid, operation aborted");
        return "";
    }
    size_t versionEnd = source.find('\n', versionLine);
    if (versionEnd == std::string::npos) {
        LOG("WARNING::SHADER_VARIANTS::INJECT_DEFINES source version line has not found a newline character, defaulting to source last character");
        versionEnd = source.length() - 1;
    }

    std::string variantSource = source.substr(0, versionEnd + 1);
    for(const std::string &define : defines)
        variantSource += "#define " + define + "\n";
    variantSource += source.substr(versionEnd + 1);
    return variantSource;
}

bool ShaderVariants::IsValid() const {
    return !source.empty();
}

