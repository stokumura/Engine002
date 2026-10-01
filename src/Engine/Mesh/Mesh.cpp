#include "Mesh.h"
#include <stdexcept>
#include <string>
#include "../../Utils/Logger/Logger.h"
#include "VertexAttribute.h"

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), instances(0) {}

Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const std::vector<const Texture*> &textures, unsigned int instances) : layout(layout), vertices(vertices), indices(indices), instances(instances), textures(textures) {
    if(!CreateBuffers(nullptr)) {
        LOG("WARNING::MESH_CONSTRUCTOR, mesh could not be constructed");
    }
}

Mesh::~Mesh() {
    Release();
}

Mesh::Mesh(const Mesh &other) : layout(other.layout), vertices(other.vertices), indices(other.indices), textures(other.textures), instances(other.instances) {
    if(!CreateBuffers(&other)) {
        LOG("WARNING::MESH_COPY_CONSTRUCTOR, mesh could not be constructed from copy source");
    }
}

Mesh& Mesh::operator=(const Mesh &other) {
    if(this == &other) return *this;

    Release();
    layout = other.layout;
    vertices = other.vertices;
    indices = other.indices;
    textures = other.textures;
    instances = other.instances;

    if(!CreateBuffers(&other)) {
        LOG("WARNING::MESH_COPY_ASSIGNMENT, mesh could not be constructed from copy source");
    }

    return *this;
}

Mesh::Mesh(Mesh &&other) noexcept : layout(std::move(other.layout)), vertices(std::move(other.vertices)), indices(std::move(other.indices)), textures(std::move(other.textures)), instances(other.instances), VAO(other.VAO), VBO(other.VBO), EBO(other.EBO) {
    other.instances = 0;
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
}

Mesh& Mesh::operator=(Mesh &&other) noexcept {
    if(this == &other) return *this;

    Release();
    layout = std::move(other.layout);
    vertices = std::move(other.vertices);
    indices = std::move(other.indices);
    textures = std::move(other.textures);
    instances = other.instances;

    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;

    other.instances = 0;
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;

    return *this;
}

void Mesh::Bind() const {
    if(!IsValid()) {
        LOG("WARNING::MESH_BIND mesh is not valid to bind, vertex array buffer binding aborted");
        return;
    }
    glBindVertexArray(VAO);
}

void Mesh::Unbind() {
    glBindVertexArray(0);
}

void Mesh::Draw(ShaderProgram &shaderProgram, const Sampler &sampler) const {
    if(!IsValid()) {
        LOG("WARNING::MESH_DRAW mesh is not valid to draw, vertex array buffer draw aborted");
        return;
    }

    unsigned int diffuseNr = 1; 
    unsigned int specularNr = 1; 
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    shaderProgram.Bind();
    for(unsigned int i = 0; i < textures.size(); i++) {
        const Texture *texture = textures[i];
        TextureType type = texture->GetType();
        unsigned int textureNr;
        switch(type) {
            case TextureType::DIFFUSE:
                textureNr = diffuseNr;
                diffuseNr++;
                break;
            case TextureType::SPECULAR:
                textureNr = specularNr;
                specularNr++;
                break;
            case TextureType::NORMAL:
                textureNr = normalNr;
                normalNr++;
                break;
            case TextureType::HEIGHT:
                textureNr = heightNr;
                heightNr++;
                break;
            default:
                throw std::invalid_argument("ERROR::MESH_DRAW::Invalid Texture Type");
        }
        sampler.Bind(texture->GetUnit());
        texture->Bind();
        std::string uniformName = "uObjectMaterial." + TextureTypeToString(type) + std::to_string(textureNr);
        shaderProgram.SetInt(uniformName, texture->GetUnit());
    }

    Bind();
    if(instances > 0) {
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0, instances);
    } else {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    }
    Unbind();

    for(unsigned int i = 0; i < textures.size(); i++) {
        const Texture *texture = textures[i];
        texture->Unbind();
    }

    shaderProgram.Unbind();
}

bool Mesh::CreateBuffers(const Mesh *source) {
    GLsizeiptr vertexSize = static_cast<GLsizeiptr>(vertices.size() * sizeof(float));
    GLsizeiptr elementsSize = static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int));

    if (vertexSize == 0 || elementsSize == 0) {
        VAO = VBO = EBO = 0;
        LOG("WARNING::MESH_ALLOCATION mesh has no vertices or indices, mesh voided");
        return false;
    }

    if (source && !source->IsValid()) {
        VAO = VBO = EBO = 0;
        LOG("WARNING::MESH_ALLOCATION source mesh is not valid, copy aborted, mesh voided");
        return false;
    }

    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    if (source) {
        glNamedBufferStorage(VBO, vertexSize, nullptr, 0);
        glNamedBufferStorage(EBO, elementsSize, nullptr, 0);
        glCopyNamedBufferSubData(source->VBO, VBO, 0, 0, vertexSize);
        glCopyNamedBufferSubData(source->EBO, EBO, 0, 0, elementsSize);
    } else {
        glNamedBufferStorage(VBO, vertexSize, vertices.data(), 0);
        glNamedBufferStorage(EBO, elementsSize, indices.data(), 0);
    }

    SetupVertexArrayLayout();
    return true;
}

void Mesh::SetupVertexArrayLayout() {
    GLuint bindingPoint {0};
    GLsizei stride {0};
    for(const auto &attribute : layout) {
        stride += attribute.components * sizeof(float);
    }
    glVertexArrayVertexBuffer(VAO, bindingPoint, VBO, 0, stride);
    glVertexArrayElementBuffer(VAO, EBO);

    GLuint offset {0};
    for(size_t i = 0; i < layout.size(); i++) {
        const auto &attribute = layout[i];
        glEnableVertexArrayAttrib(VAO, i);
        glVertexArrayAttribFormat(VAO, i, attribute.components, GL_FLOAT, GL_FALSE, offset);
        glVertexArrayAttribBinding(VAO, i, bindingPoint);
        offset += attribute.components * sizeof(float);
    }
    glVertexArrayBindingDivisor(VAO, bindingPoint, instances > 0 ? 1 : 0);
}

void Mesh::Release() {
    if(VAO) glDeleteVertexArrays(1, &VAO);
    if(VBO) glDeleteBuffers(1, &VBO);
    if(EBO) glDeleteBuffers(1, &EBO);
    VAO = VBO = EBO = 0;
}

bool Mesh::IsValid() const {
    return VAO != 0 && VBO != 0 && EBO != 0;
}
