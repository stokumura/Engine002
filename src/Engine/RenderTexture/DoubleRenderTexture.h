#pragma once

#include <glad/gl.h>

class DoubleRenderTexture {
    private:
        bool hdr;
        unsigned int width, height;
        GLuint color[2], depth, FBO;

        mutable bool error = false;
    public:
        DoubleRenderTexture();
        DoubleRenderTexture(unsigned int width, unsigned int height, bool hdr = false);
        ~DoubleRenderTexture();

        DoubleRenderTexture(const DoubleRenderTexture &other) = delete;
        DoubleRenderTexture& operator=(const DoubleRenderTexture &other) = delete;

        DoubleRenderTexture(DoubleRenderTexture &&other) noexcept;
        DoubleRenderTexture& operator=(DoubleRenderTexture &&other) noexcept;

        void BindFBO() const;
        void BindTexture(GLuint unit, unsigned int index) const;

    private:
        void SetupAttachments();
        void CheckAttachments() const;
        void Destroy();

        bool IsValid() const;
};
