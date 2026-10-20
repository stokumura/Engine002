#include "RenderTexture.h"
#include "../../Utils/Logger/Logger.h"

RenderTexture::RenderTexture() {
    width = 0;
    height = 0;
    FBO = 0;
    color = 0;
    depth = 0;
    error = 0;
}
RenderTexture::RenderTexture(unsigned int width, unsigned int height) : width(width), height(height) {
    SetupAttachments();
    CheckAttachments();
}

RenderTexture::~RenderTexture() {
    Destroy();
}

RenderTexture::RenderTexture(RenderTexture &&other) noexcept : width(other.width), height(other.height), FBO(other.FBO), color(other.color), depth(other.depth), error(other.error) {
    other.width = 0;
    other.height = 0;
    other.FBO = 0;
    other.color = 0;
    other.depth = 0;
    other.error = 0;
}

RenderTexture& RenderTexture::operator=(RenderTexture &&other) noexcept {
    if(this == &other) return *this;

    Destroy();

    width = other.width;
    height = other.height;
    FBO = other.FBO;
    color = other.color;
    depth = other.depth;
    error = other.error;

    other.width = 0;
    other.height = 0;
    other.FBO = 0;
    other.color = 0;
    other.depth = 0;
    other.error = 0;

    return *this;
}

void RenderTexture::BindFBO() const {
    if(!IsValid()) {
        LOG("ERROR::RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void RenderTexture::BindTexture(GLuint unit) const {
    if(!IsValid()) {
        LOG("ERROR::RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    glBindTextureUnit(unit, color);
}

void RenderTexture::Blit(GLuint targetFBO, int offsetX, int offsetY) const {
    if(!IsValid()) {
        LOG("ERROR::RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    glBlitNamedFramebuffer(FBO, targetFBO, 0, 0, width, height, offsetX, offsetY, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void RenderTexture::SetupAttachments() {
    glCreateTextures(GL_TEXTURE_2D, 1, &color);
    glTextureStorage2D(color, 1, GL_RGB16F, width, height);
    glTextureParameteri(color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glCreateRenderbuffers(1, &depth);
    glNamedRenderbufferStorage(depth, GL_DEPTH24_STENCIL8, width, height);

    glCreateFramebuffers(1, &FBO);
    glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, color, 0);
    glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
}

void RenderTexture::CheckAttachments() const {
    if(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        error = true;
        LOG("ERROR::RENDER_TEXTURE framebuffer is incomplete");
    }
}

void RenderTexture::Destroy() {
    if(FBO) glDeleteFramebuffers(1, &FBO);
    if(color) glDeleteTextures(1, &color);
    if(depth) glDeleteRenderbuffers(1, &depth);
}

bool RenderTexture::IsValid() const {
    return !error && FBO;
}
