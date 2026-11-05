#pragma once

#include <glad/gl.h>

class RenderTexture {
    private:
        bool hdr;
        unsigned int width, height;
        GLuint color, depth, FBO;

        mutable bool error = false;
    public:
        RenderTexture();
        RenderTexture(unsigned int width, unsigned int height, bool hdr = false);
        ~RenderTexture();

        RenderTexture(const RenderTexture &other) = delete;
        RenderTexture& operator=(const RenderTexture &other) = delete;

        RenderTexture(RenderTexture &&other) noexcept;
        RenderTexture& operator=(RenderTexture &&other) noexcept;

        void BindFBO() const;
        void BindTexture(GLuint unit) const;

        void Blit(GLuint targetFBO, int offsetX = 0, int offsetY = 0) const;

        GLuint GetFBO() const { return FBO; }
    private:
        void SetupAttachments();
        void CheckAttachments() const;
        void Destroy();

        bool IsValid() const;
};
