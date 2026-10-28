#pragma once

#include <glad/gl.h>
#include "ShadowMap.h"

class ShadowCubemap {
    private:
        GLuint FBO, depthCubemap;

        mutable bool error = false;
    public:
        ShadowCubemap();
        ~ShadowCubemap();

        ShadowCubemap(const ShadowCubemap &other) = delete;
        ShadowCubemap& operator=(const ShadowCubemap &other) = delete;

        ShadowCubemap(ShadowCubemap &&other) noexcept;
        ShadowCubemap& operator=(ShadowCubemap &&other) noexcept;

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
