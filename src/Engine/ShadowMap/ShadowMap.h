#pragma once

#include <glad/gl.h>

const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

class ShadowMap {
    private:
        GLuint FBO, depthMap;

        mutable bool error = false;
    public:
        ShadowMap();
        ~ShadowMap();

        ShadowMap(const ShadowMap &other) = delete;
        ShadowMap& operator=(const ShadowMap &other) = delete;

        ShadowMap(ShadowMap &&other) noexcept;
        ShadowMap& operator=(ShadowMap &&other) noexcept;

        void BindFBO() const;
        void UnbindFBO(float windowWidth, float windowHeight) const;

        void BindMap(GLuint unit) const;
        void UnbindMap(GLuint unit) const;
    private:
        void Setup();
        void CheckErrors() const;

        void Destroy() const;

        bool IsValid() const;
};
