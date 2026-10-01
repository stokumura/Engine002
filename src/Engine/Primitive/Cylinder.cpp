#include "Cylinder.h"

Cylinder::Cylinder(const VertexAttributeTypeLayout &attributes, float radius, float height, unsigned int segments, unsigned int rings) : Shape(attributes), radius(radius), height(height), segments(segments), rings(rings) {
    Build();
}

Mesh Cylinder::ConstructMesh(const VertexAttributeTypeLayout &attributes) const {
    VertexLayout layout = VertexAttribute::GetVertexLayout(attributes);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0;

    AddVertex(vertices, attributes, glm::vec3(0.0, - halfHeight, 0.0), glm::vec2(0.5, 0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
    AddVertex(vertices, attributes, glm::vec3(0.0, halfHeight, 0.0), glm::vec2(0.5, 1.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 0.0, -1.0));

    for (int ring = 0; ring <= rings; ring++)
    {
        float v = static_cast<float>(ring) / static_cast<float>(rings);
        float currentHeight = v * height - halfHeight;

        for (int seg = 0; seg <= segments; seg++)
        {
            float u = static_cast<float>(seg) / static_cast<float>(segments);
            float phi = u * 2.0f * M_PI;

            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 position(radius * cosPhi,
                               currentHeight,
                               radius * sinPhi);
            glm::vec2 texCoords(u, v * 0.95);

            glm::vec3 normal = glm::vec3(cosPhi, 0.0f, sinPhi);
            glm::vec3 tangent(-sinPhi, 0.0f, cosPhi);
            glm::vec3 bitangent(0, 1, 0);

            AddVertex(vertices, attributes, position, texCoords, normal, tangent, bitangent);
        }
    }

    int offset = 2;
    int rowSize = segments + 1;
    for (int ring = 0; ring < rings; ring++)
    {
        for (int seg = 0; seg < segments; seg++)
        {
            int current = ring * rowSize + seg + offset;
            int next = current + rowSize;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    int bottomVertex = 0;
    for(int seg = 0; seg < segments; seg++) {
        int current = seg + offset;

        indices.push_back(current);
        indices.push_back(current + 1);
        indices.push_back(bottomVertex);
    }
    int topVertex = 1;
    for(int seg = 0; seg < segments; seg++) {
        int current = seg + offset + rings * rowSize;

        indices.push_back(current);
        indices.push_back(topVertex);
        indices.push_back(current + 1);
    }

    return Mesh(layout, vertices, indices, std::vector<const Texture*>());
}
