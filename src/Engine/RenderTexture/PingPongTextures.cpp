#include "PingPongTextures.h"
#include "../../Utils/Logger/Logger.h"

PingPongTextures::PingPongTextures() {
    width = 0;
    height = 0;
    FBO[0] = 0;
    FBO[1] = 0;
    color[0] = 0;
    color[1] = 0;
    error = 0;
}

PingPongTextures::PingPongTextures(unsigned int width, unsigned int height, bool hdr) : hdr(hdr), width(width), height(height) {
    SetupAttachments();
    CheckAttachments();
}

PingPongTextures::~PingPongTextures() {
    Destroy();
}

PingPongTextures::PingPongTextures(PingPongTextures &&other) noexcept : hdr(other.hdr), width(other.width), height(other.height), error(other.error) {
    FBO[0] = other.FBO[0];
    FBO[1] = other.FBO[1];
    color[0] = other.color[0];
    color[1] = other.color[1];

    other.hdr = false;
    other.width = 0;
    other.height = 0;
    other.FBO[0] = 0;
    other.FBO[1] = 0;
    other.color[0] = 0;
    other.color[1] = 0;
    other.error = 0;
}

PingPongTextures& PingPongTextures::operator=(PingPongTextures &&other) noexcept {
    if(this == &other) return *this;

    Destroy();

    hdr = other.hdr;
    width = other.width;
    height = other.height;
    FBO[0] = other.FBO[0];
    FBO[1] = other.FBO[1];
    color[0] = other.color[0];
    color[1] = other.color[1];
    error = other.error;

    other.hdr = false;
    other.width = 0;
    other.height = 0;
    other.FBO[0] = 0;
    other.FBO[1] = 0;
    other.color[0] = 0;
    other.color[1] = 0;
    other.error = 0;

    return *this;
}

void PingPongTextures::BindFBO(unsigned int index) const {
    if(!IsValid(index)) {
        LOG("ERROR::PING_PONG_TEXTURES::BindFBO Render Texture [%d] is not valid, operation aborted", index);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO[index]);
}

void PingPongTextures::BindTexture(GLuint unit, unsigned int index) const {
    if(!IsValid(index)) {
        LOG("ERROR::PING_PONG_TEXTURES::BindFBO Render Texture [%d] is not valid, operation aborted", index);
        return;
    }

    glBindTextureUnit(unit, color[index]);
}

void PingPongTextures::SetupAttachments() {
    GLenum internalFormat = GL_RGBA8;
    if(hdr) internalFormat = GL_RGBA16F;

    glCreateTextures(GL_TEXTURE_2D, 2, color);
    glCreateFramebuffers(2, FBO);

    for(unsigned int i = 0; i < 2; i++) {
        glTextureStorage2D(color[i], 1, internalFormat, width, height);
        glTextureParameteri(color[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(color[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(color[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(color[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(FBO[i], GL_COLOR_ATTACHMENT0, color[i], 0);
    }
}

void PingPongTextures::CheckAttachments() const {
    for (unsigned int i = 0; i < 2; i++) {
        if(glCheckNamedFramebufferStatus(FBO[i], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            error = true;
            LOG("ERROR::PING_PONG_TEXTURES framebuffer [%d] is incomplete", i);
        }
    }
}

void PingPongTextures::Destroy() {
    if(FBO[0] && FBO[1]) glDeleteFramebuffers(2, FBO);
    if(color[0] && color[1]) glDeleteTextures(2, color);
}

bool PingPongTextures::IsValid(unsigned int index) const {
    if(index < 0 || index >= 2) {
        LOG("WARNING::PING_PONG_TEXTURES::IsValid index %d is out of bounds", index);
        return false;
    }
    return !error && FBO[index];
}

