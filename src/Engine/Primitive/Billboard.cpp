#include "Billboard.h"

Billboard::Billboard(const VertexAttributeTypeLayout &attributes, unsigned int amount, unsigned int xSubdivisions, unsigned int ySubdivisions, float length) : Shape(attributes), amount(amount), xSubdivisions(xSubdivisions), ySubdivisions(ySubdivisions), length(length) {
    Build();
}

Mesh Billboard::ConstructMesh(const VertexAttributeTypeLayout &attributes) const {
    VertexLayout layout = VertexAttribute::GetVertexLayout(attributes);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float anglePerPlane = 2 * M_PI / amount;
    unsigned int vertexCount = 0;

    auto AddPlane = [&](const glm::vec3 &origin, float angle) {
        unsigned int rowSize = xSubdivisions + 1;

        glm::vec3 axisY(0.0, 1.0, 0.0);
        glm::quat rot = glm::angleAxis(angle, axisY);
        glm::vec3 axisX = rot * glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 axisZ = glm::cross(axisX, axisY);


        for (unsigned int v = 0; v <= xSubdivisions; v++)
        {
            float fv = static_cast<float>(v) / static_cast<float>(xSubdivisions);
            float fvN = fv - 0.5f;

            for (unsigned int u = 0; u <= ySubdivisions; u++)
            {
                float fu = static_cast<float>(u) / static_cast<float>(ySubdivisions);
                glm::vec3 position = origin + (fu * length) * axisY + (fvN * length) * axisX;
                glm::vec2 texCoords(fv, fu);

                AddVertex(vertices, attributes, position, texCoords, axisZ, axisY, axisX);
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

        vertexCount += rowSize * (ySubdivisions + 1);
    };

    for(int i = 0; i < amount; i++) {
        float angle = anglePerPlane * i;
        AddPlane(glm::vec3(0.0, 0.0, 0.0), angle);
    }

    count = indices.size();

    return Mesh(layout, vertices, indices, std::vector<const Texture*>());
}
