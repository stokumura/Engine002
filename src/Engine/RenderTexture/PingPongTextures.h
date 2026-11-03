#pragma once

#include <glad/gl.h>

class PingPongTextures {
    private:
        bool hdr;
        unsigned int width, height;
        GLuint color[2], FBO[2];

        mutable bool error = false;
    public:
        PingPongTextures();
        PingPongTextures(unsigned int width, unsigned int height, bool hdr = false);
        ~PingPongTextures();

        PingPongTextures(const PingPongTextures &other) = delete;
        PingPongTextures& operator=(const PingPongTextures &other) = delete;

        PingPongTextures(PingPongTextures &&other) noexcept;
        PingPongTextures& operator=(PingPongTextures &&other) noexcept;

        void BindFBO(unsigned int index) const;
        void BindTexture(GLuint unit, unsigned int index) const;

    private:
        void SetupAttachments();
        void CheckAttachments() const;
        void Destroy();

        bool IsValid(unsigned int index) const;
};
