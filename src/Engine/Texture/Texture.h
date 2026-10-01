#pragma once

#include <glad/gl.h>
#include <glm/ext/vector_float4.hpp>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

#include <string>

enum class TextureType {
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT,
    DISPLACEMENT,
};

std::string TextureTypeToString(TextureType type);

class Texture {
    private:
        GLuint ID;
        TextureType type;
        GLuint unit;
        int width, height;
        std::string path;

        mutable bool binded = false;

        static GLint maxCombinedUnits;

    public:
        Texture(const std::string &path, TextureType type, GLuint unit);
        ~Texture();

        [[deprecated("WARNING::TEXTURE_COPY_CTOR cannot be coppied")]]
        Texture(const Texture&) = delete;

        [[deprecated("WARNING::TEXTURE_COPY_ASSIGNMENT cannot be coppied")]]
        Texture& operator=(const Texture&) = delete;

        Texture(Texture &&other) noexcept;
        Texture& operator=(Texture &&other) noexcept;

        void Bind() const;
        void Unbind() const;
        void SetUnit(GLuint newUnit);

        TextureType GetType() const { return type; }
        GLuint GetUnit() const { return unit; }
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        std::string GetPath() const { return path; }
};
