#pragma once

#include <glad/gl.h>

class GBuffer {
    private:
        bool hdr;
        unsigned int width, height;
        GLuint gPosition, gNormal, gAlbedoSpec, depth, FBO;

        mutable bool error = false;
    public:
        GBuffer();
        GBuffer(unsigned int width, unsigned int height);
        ~GBuffer();

        GBuffer(const GBuffer &other) = delete;
        GBuffer& operator=(const GBuffer &other) = delete;

        GBuffer(GBuffer &&other) noexcept;
        GBuffer& operator=(GBuffer &&other) noexcept;

        void BindFBO() const;
        void BindTexture(GLuint unit, unsigned int index) const;

        GLuint GetFBO() const { return FBO; }

    private:
        void SetupAttachments();
        void CheckAttachments() const;
        void Destroy();

        bool IsValid() const;
};
