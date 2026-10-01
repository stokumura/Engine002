#include "Texture.h"
#include <algorithm>
#include "../../Utils/Logger/Logger.h"
#include <stdexcept>

GLint Texture::maxCombinedUnits = 0;

std::string TextureTypeToString(TextureType type) {
    switch (type)
    {
        case TextureType::DIFFUSE:
            return "diffuse";
        case TextureType::SPECULAR:
            return "specular";
        case TextureType::NORMAL:
            return "normal";
        case TextureType::HEIGHT:
            return "height";
        case TextureType::DISPLACEMENT:
            return "displacement";
        default:
            LOG("ERROR::TEXTURE_TYPE_TO_STRING unknown type specified");
            throw std::invalid_argument("Invalid TextureType");
    }
}

Texture::Texture(const std::string &path, TextureType type, GLuint unit) : path(path), type(type) {
    SetUnit(unit);
    stbi_set_flip_vertically_on_load(true);
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if(!data) {
        LOG("ERROR::TEXTURE_LOAD failed to load texture from: %s", path.c_str());
        throw std::runtime_error("Failed to load texture: " + path);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &ID);
    int mipLevels = 1 + static_cast<int>(std::floor(std::log2(std::max(width, height))));

    GLenum format, internalformat;

    switch(type)
    {
        case TextureType::NORMAL:
            internalformat = GL_RGB8;
            break;
        case TextureType::HEIGHT:
        case TextureType::DISPLACEMENT:
            internalformat = GL_R8;
            break;
        case TextureType::DIFFUSE:
        case TextureType::SPECULAR:
            if(channels == 1) {
                internalformat = GL_R8;
            } else if(channels == 3) {
                internalformat = GL_SRGB8;
            } else if(channels == 4) {
                internalformat = GL_SRGB8_ALPHA8;
            } else {
                stbi_image_free(data);
                LOG("ERROR::TEXTURE_CONFIG unsupported number of color channels in texture from: %s", path.c_str());
                throw std::invalid_argument("Unsupported number of color channels in texture");
            }
            break;
        default:
            stbi_image_free(data);
            LOG("ERROR::TEXTURE_CONFIG automatic texture recognition failed in texture from: %s", path.c_str());
            throw std::invalid_argument("Automatic Texture type recognition failed");
    }

    if (channels == 1) {
        format = GL_RED;
    } else if (channels == 3) {
        format = GL_RGB;
    } else if (channels == 4) {
        format = GL_RGBA;
    } else {
        stbi_image_free(data);
        LOG("ERROR::TEXTURE_CONFIG unsupported number of color channels in texture from: %s", path.c_str());
        throw std::invalid_argument("Unsupported number of color channels in texture");
    }

    glTextureStorage2D(ID, mipLevels, internalformat, width, height);
    glTextureSubImage2D(ID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(ID);
}

Texture::~Texture() {
    if(ID) glDeleteTextures(1, &ID);
}

Texture::Texture(Texture &&other) noexcept : ID(other.ID), width(other.width), height(other.height), path(other.path), type(other.type), unit(other.unit), binded(other.binded) {
    other.path = "";
    other.ID = 0;
    other.binded = false;
}

Texture& Texture::operator=(Texture &&other) noexcept {
    if(this == &other) return *this;

    if(ID) glDeleteTextures(1, &ID);

    ID = other.ID;
    width = other.width;
    height = other.height;
    path = other.path;
    type = other.type;
    unit = other.unit;
    binded = other.binded;

    other.ID = 0;
    other.binded = false;
    other.path = "";

    return *this;
}

void Texture::Bind() const {
    if(binded) return;
    if(ID) {
        glBindTextureUnit(unit, ID);
        binded = true;
    }
}

void Texture::Unbind() const {
    if(!binded) return;
    glBindTextureUnit(unit, 0);
    binded = false;
}

void Texture::SetUnit(GLuint newUnit) {
    if(binded) {
        LOG("WARNING::TEXTURE_SET_UNIT cannot change unit of texture while it is binded, operation aborted");
        return;
    }
    if(!maxCombinedUnits)
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedUnits);
    unit = std::clamp(static_cast<int>(newUnit), 0, maxCombinedUnits - 1);
}
