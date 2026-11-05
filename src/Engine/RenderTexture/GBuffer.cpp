#include "GBuffer.h"
#include "../../Utils/Logger/Logger.h"

GBuffer::GBuffer() {
    width = 0;
    height = 0;
    FBO = 0;
    gPosition = 0;
    gNormal = 0;
    gAlbedoSpec = 0;
    depth = 0;
    error = 0;
}

GBuffer::GBuffer(unsigned int width, unsigned int height) : width(width), height(height) {
    SetupAttachments();
    CheckAttachments();
}

GBuffer::~GBuffer() {
    Destroy();
}

GBuffer::GBuffer(GBuffer &&other) noexcept : hdr(other.hdr), width(other.width), height(other.height), gPosition(other.gPosition), gNormal(other.gNormal), gAlbedoSpec(other.gAlbedoSpec), depth(other.depth), FBO(other.FBO), error(other.error) {
    other.hdr = false;
    other.width = 0;
    other.height = 0;
    other.gPosition= 0;
    other.gNormal = 0;
    other.gAlbedoSpec = 0;
    other.depth = 0;
    other.FBO = 0;
    other.error = 0;
}

GBuffer& GBuffer::operator=(GBuffer &&other) noexcept {
    if(this == &other) return *this;

    Destroy();

    hdr = other.hdr;
    width = other.width;
    height = other.height;
    gPosition = other.gPosition;
    gNormal = other.gNormal;
    gAlbedoSpec = other.gAlbedoSpec;
    depth = other.depth;
    FBO = other.FBO;
    error = other.error;

    other.hdr = false;
    other.width = 0;
    other.height = 0;
    other.gPosition= 0;
    other.gNormal = 0;
    other.gAlbedoSpec = 0;
    other.depth = 0;
    other.FBO = 0;
    other.error = 0;

    return *this;
}

void GBuffer::BindFBO() const {
    if(!IsValid()) {
        LOG("ERROR::DOUBLE_RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void GBuffer::BindTexture(GLuint unit, unsigned int index) const {
    if(index < 0 || index >= 3) {
        LOG("ERROR::DOUBLE_RENDER_TEXTURE::BindFBO index [%d] out of bounds");
        return;
    }

    if(!IsValid()) {
        LOG("ERROR::DOUBLE_RENDER_TEXTURE::BindFBO Render Texture is not valid, operation aborted");
        return;
    }

    GLuint tex = 0;
    if(index == 0) tex = gPosition;
    else if(index  == 1) tex = gNormal;
    else if (index == 2) tex = gAlbedoSpec;

    glBindTextureUnit(unit, tex);
}

void GBuffer::SetupAttachments() {
    glCreateFramebuffers(1, &FBO);

    glCreateTextures(GL_TEXTURE_2D, 1, &gPosition);
    glTextureStorage2D(gPosition, 1, GL_RGBA16F, width, height);
    glTextureParameteri(gPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(gPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, gPosition, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &gNormal);
    glTextureStorage2D(gNormal, 1, GL_RGBA16F, width, height);
    glTextureParameteri(gNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(gNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT1, gNormal, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &gAlbedoSpec);
    glTextureStorage2D(gAlbedoSpec, 1, GL_RGBA8, width, height);
    glTextureParameteri(gAlbedoSpec, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(gAlbedoSpec, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT2, gAlbedoSpec, 0);

    glCreateRenderbuffers(1, &depth);
    glNamedRenderbufferStorage(depth, GL_DEPTH24_STENCIL8, width, height);
    glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);

    GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glNamedFramebufferDrawBuffers(FBO, 3, attachments);
}

void GBuffer::CheckAttachments() const {
    if(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        error = true;
        LOG("ERROR::RENDER_TEXTURE framebuffer is incomplete");
    }
}

void GBuffer::Destroy() {
    if(FBO) glDeleteFramebuffers(1, &FBO);
    if(gPosition) glDeleteTextures(1, &gPosition);
    if(gNormal) glDeleteTextures(1, &gNormal);
    if(gAlbedoSpec) glDeleteTextures(1, &gAlbedoSpec);
}

bool GBuffer::IsValid() const {
    return !error && FBO;
}
