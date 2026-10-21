#pragma once

#include <glad/gl.h>
#include <stb/stb_image.h>
#include <filesystem>
#include <string>
#include <vector>
#include <cmath>

class Cubemap {
    private:
        GLuint ID;

    public:
        Cubemap(const std::filesystem::path &parent, const std::vector<std::string> &faces);
        ~Cubemap();

        Cubemap(const Cubemap &other) = delete;
        Cubemap& operator=(const Cubemap &other) = delete;

        Cubemap(Cubemap &&other) noexcept;
        Cubemap& operator=(Cubemap &&other) noexcept;

        void Bind(GLuint unit) const;

    private:
        void SetupTexture(const std::filesystem::path &parent, const std::vector<std::string> &faces);
        void Destroy();

        bool IsValid() const;
};
