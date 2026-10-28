#include "ShadowCubemap.h"
#include "../../Utils/Logger/Logger.h"
#include "ShadowMap.h"

ShadowCubemap::ShadowCubemap() {
    Setup();
    CheckErrors();
}

ShadowCubemap::~ShadowCubemap() {
    Destroy();
}

ShadowCubemap::ShadowCubemap(ShadowCubemap &&other) noexcept : FBO(other.FBO), depthCubemap(other.depthCubemap), error(other.error) {
    other.FBO = 0;
    other.depthCubemap = 0;
    other.error = false;
}

ShadowCubemap& ShadowCubemap::operator=(ShadowCubemap &&other) noexcept {
    if(this == &other) return *this;

    Destroy();

    FBO = other.FBO;
    depthCubemap = other.depthCubemap;
    error = other.error;

    other.FBO = 0;
    other.depthCubemap = 0;
    other.error = false;

    return *this;
}

void ShadowCubemap::BindFBO() const {
    if(!IsValid()) {
        LOG("ERROR::SHADOW_CUBE_MAP::BIND_FBO is not valid, operation aborted");
        return;
    }

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void ShadowCubemap::UnbindFBO(float windowWidth, float windowHeight) const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
}

void ShadowCubemap::BindMap(GLuint unit) const {
    if(!IsValid()) {
        LOG("ERROR::SHADOW_CUBE_MAP::BIND_MAP is not valid, operation aborted");
        return;
    }

    glBindTextureUnit(unit, depthCubemap);
}

void ShadowCubemap::UnbindMap(GLuint unit) const {
    glBindTextureUnit(unit, 0);
}

void ShadowCubemap::Setup() {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &depthCubemap);
    glTextureStorage2D(depthCubemap, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);

    glTextureParameteri(depthCubemap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(depthCubemap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(depthCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(depthCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTextureParameteri(depthCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(depthCubemap, GL_TEXTURE_BORDER_COLOR, borderColor);

    glCreateFramebuffers(1, &FBO);
    glNamedFramebufferTexture(FBO, GL_DEPTH_ATTACHMENT, depthCubemap, 0);

    glNamedFramebufferDrawBuffer(FBO, GL_NONE);
    glNamedFramebufferReadBuffer(FBO, GL_NONE);
}

void ShadowCubemap::CheckErrors() const {
    if(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        error = true;
        LOG("ERROR::SHADOW_MAP::FBO is not complete");
    }
}

void ShadowCubemap::Destroy() const {
    if (FBO) glDeleteFramebuffers(1, &FBO);
    if (depthCubemap) glDeleteTextures(1, &depthCubemap);
}

bool ShadowCubemap::IsValid() const {
    return !error && FBO && depthCubemap;
}
