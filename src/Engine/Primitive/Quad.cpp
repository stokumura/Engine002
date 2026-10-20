#include "Quad.h"

Quad::Quad(const VertexAttributeTypeLayout &attributes) : Shape(attributes) {
    Build();
}

Mesh Quad::ConstructMesh(const VertexAttributeTypeLayout &attributes) const {
    VertexLayout layout = VertexAttribute::GetVertexLayout(attributes);

    std::vector<float> vertices;
    std::vector<unsigned int> indices {
        0, 1, 2,
        2, 1, 3
    };

    for(unsigned int i = 0; i < 4; i++) {
        glm::vec3 position = glm::vec3(QUAD_POSITIONS[i], 0.0);;
        glm::vec2 texCoords = QUAD_POSITIONS[i] * 0.5f + 0.5f;
        AddVertex(vertices, attributes, position, texCoords, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    return Mesh(layout, vertices, indices, std::vector<const Texture*>());
}
