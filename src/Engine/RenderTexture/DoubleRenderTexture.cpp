#include "DoubleRenderTexture.h"
#include "../../Utils/Logger/Logger.h"

DoubleRenderTexture::DoubleRenderTexture() {
    width = 0;
    height = 0;
    FBO = 0;
    color[0] = 0;
    color[1] = 0;
    error = 0;
}

DoubleRenderTexture::DoubleRenderTexture(unsigned int width, unsigned int height, bool hdr) : hdr(hdr), width(width), height(height) {
    SetupAttachments();
    CheckAttachments();
}

DoubleRenderTexture::~DoubleRenderTexture() {
    Destroy();
}

DoubleRenderTexture::DoubleRenderTexture(DoubleRenderTexture &&other) noexcept : hdr(other.hdr), width(other.width), height(other.height), depth(other.depth), FBO(other.FBO), error(other.error) {
    color[0] = other.color[0];
    color[1] = other.color[1];

    other.hdr = false;
    other.width = 0;
    other.height = 0;
    other.depth = 0;
    other.FBO = 0;
    other.color[0] = 0;
    other.color[1] = 0;
    other.error = 0;
}

DoubleRenderTexture& DoubleRenderTexture::operator=(DoubleRenderTexture &&other) noexcept {
    if(this == &other) return *this;

    Destroy();

    hdr = other.hdr;
    width = other.width;
    height = other.height;
    depth = other.depth;
    FBO = other.FBO;
    color[0] = other.color[0];
    color[1] = other.color[1];
    error = other.error;

    other.hdr = false;
    other.width = 0;
    other.height = 0;
    other.depth = 0;
    other.FBO = 0;
    other.color[0] = 0;
    other.color[1] = 0;
    other.error = 0;

    return *this;
}

void DoubleRenderTexture::BindFBO() const {
    if(!IsValid()) {
        LOG("ERROR::DOUBLE_RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void DoubleRenderTexture::BindTexture(GLuint unit, unsigned int index) const {
    if(index < 0 || index >= 2) {
        LOG("ERROR::DOUBLE_RENDER_TEXTURE::BindFBO index [%d] out of bounds");
        return;
    }

    if(!IsValid()) {
        LOG("ERROR::DOUBLE_RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    glBindTextureUnit(unit, color[index]);
}

void DoubleRenderTexture::SetupAttachments() {
    GLenum internalFormat = GL_RGBA8;
    if(hdr) internalFormat = GL_RGBA16F;

    glCreateTextures(GL_TEXTURE_2D, 2, color);
    glCreateFramebuffers(1, &FBO);

    for(unsigned int i = 0; i < 2; i++) {
        glTextureStorage2D(color[i], 1, internalFormat, width, height);
        glTextureParameteri(color[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(color[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(color[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(color[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0 + i, color[i], 0);
    }

    glCreateRenderbuffers(1, &depth);
    glNamedRenderbufferStorage(depth, GL_DEPTH24_STENCIL8, width, height);
    glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);

    GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glNamedFramebufferDrawBuffers(FBO, 2, attachments);
}

void DoubleRenderTexture::CheckAttachments() const {
    if(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        error = true;
        LOG("ERROR::RENDER_TEXTURE framebuffer is incomplete");
    }
}

void DoubleRenderTexture::Destroy() {
    if(FBO) glDeleteFramebuffers(1, &FBO);
    if(color[0] && color[1]) glDeleteTextures(2, color);
}

bool DoubleRenderTexture::IsValid() const {
    return !error && FBO;
}
