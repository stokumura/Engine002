#include "Plane.h"

Plane::Plane(const VertexAttributeTypeLayout &attributes, unsigned int xSubdivisions, unsigned int ySubdivisions, float length, glm::quat rotation) : Shape(attributes), xSubdivisions(xSubdivisions), ySubdivisions(ySubdivisions), length(length), rotation(rotation) {
    Build();
}

Mesh Plane::ConstructMesh(const VertexAttributeTypeLayout &attributes) const {
    VertexLayout layout = VertexAttribute::GetVertexLayout(attributes);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    unsigned int vertexCount = 0;

    auto AddPlane = [&](const glm::vec3 &origin, glm::quat rot) {
        unsigned int rowSize = ySubdivisions + 1;

        glm::vec3 axisX = rot * glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 axisZ = rot * glm::vec3(0.0, 0.0, 1.0);
        glm::vec3 axisY = glm::cross(axisZ, axisX);


        for (unsigned int v = 0; v <= xSubdivisions; v++)
        {
            float fv = static_cast<float>(v) / static_cast<float>(xSubdivisions);

            for (unsigned int u = 0; u <= ySubdivisions; u++)
            {
                float fu = static_cast<float>(u) / static_cast<float>(ySubdivisions);
                glm::vec3 position = origin + (fu * length) * axisY + (fv * length) * axisX;
                glm::vec2 texCoords(fv, fu);

                AddVertex(vertices, attributes, position, texCoords, -axisZ, axisX, -axisY);
            }
        }

        for (unsigned int v = 0; v < xSubdivisions; v++)
        {
            for (unsigned int u = 0; u < ySubdivisions; u++)
            {
                unsigned int current = vertexCount + v * rowSize + u;
                unsigned int next = current + rowSize;

                indices.push_back(current);
                indices.push_back(current + 1);
                indices.push_back(next);

                indices.push_back(current + 1);
                indices.push_back(next + 1);
                indices.push_back(next);
            }
        }

        vertexCount += rowSize * (xSubdivisions + 1);
    };

    AddPlane(glm::vec3(0.0, 0.0, 0.0), rotation);
    count = indices.size();

    return Mesh(layout, vertices, indices, std::vector<const Texture*>());
}
