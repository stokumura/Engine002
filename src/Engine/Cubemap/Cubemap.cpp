#include "Cubemap.h"
#include "../../Utils/Logger/Logger.h"

Cubemap::Cubemap(const std::filesystem::path &parent, const std::vector<std::string> &faces) {
    SetupTexture(parent, faces);
}

Cubemap::~Cubemap() {
    Destroy();
}

Cubemap::Cubemap(Cubemap &&other) noexcept : ID(other.ID) {
    other.ID = 0;
}

Cubemap& Cubemap::operator=(Cubemap &&other) noexcept {
    if(&other == this) return *this;

    Destroy();

    ID = other.ID;

    other.ID = 0;

    return *this;
}

void Cubemap::Bind(GLuint unit) const {
    if(!IsValid()) {
        LOG("ERROR::CUBEMAP::BIND cubemap is not valid, operation aborted");
        return;
    }
    glBindTextureUnit(unit, ID);
}

void Cubemap::SetupTexture(const std::filesystem::path &parent, const std::vector<std::string> &faces) {
    if(faces.size() < 6) {
        LOG("ERROR::CUBEMAP::SETUP faces specified are not enough to setup cubemap texture (%d), operation aborted", faces.size());
        return;
    }
    stbi_set_flip_vertically_on_load(false);
    int width, height, channels;

    unsigned char* data[6];
    for(unsigned int face = 0; face < 6; face++) {
        std::filesystem::path path = parent / faces[face];
        data[face] =  stbi_load(path.string().c_str(), &width, &height, &channels, 0);
        if(!data[face]) {
            LOG("ERROR::CUBEMAP_TEXTURE_LOAD failed to load texture from: %s", path.string().c_str());
            for(unsigned int i = 0; i <= face; i++) {
                stbi_image_free(data[i]);
            }
            throw std::runtime_error("Failed to load texture: " + path.string());
        }
    }

    GLenum internalFormat = GL_RGBA8;
    GLenum format = GL_RGBA;
    switch (channels) {
        case 1:
            internalFormat = GL_R8;
            format = GL_RED;
            break;
        case 3:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        case 4:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            break;
    }

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &ID);
    int mipLevels = 1 + static_cast<int>(std::floor(std::log2(std::max(width, height))));
    glTextureStorage2D(ID, mipLevels, internalFormat, width, height);

    for(unsigned int face = 0; face < 6; face++) {
        glTextureSubImage3D(ID, 0, 0, 0, face, width, height, 1, format, GL_UNSIGNED_BYTE, data[face]);
        stbi_image_free(data[face]);
    }
    glGenerateTextureMipmap(ID);

    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Cubemap::Destroy() {
    if(ID) glDeleteTextures(1, &ID);
}

bool Cubemap::IsValid() const {
    return ID != 0;
}
