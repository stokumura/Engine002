#pragma once

#include <glad/gl.h>

class RenderTexture {
    private:
        unsigned int width, height;
        GLuint color, depth, FBO;

        mutable bool error = false;
    public:
        RenderTexture();
        RenderTexture(unsigned int width, unsigned int height);
        ~RenderTexture();

        RenderTexture(const RenderTexture &other) = delete;
        RenderTexture& operator=(const RenderTexture &other) = delete;

        RenderTexture(RenderTexture &&other) noexcept;
        RenderTexture& operator=(RenderTexture &&other) noexcept;

        void BindFBO() const;
        void BindTexture(GLuint unit) const;

        void Blit(GLuint targetFBO, int offsetX = 0, int offsetY = 0) const;

    private:
        void SetupAttachments();
        void CheckAttachments() const;
        void Destroy();

        bool IsValid() const;
};
