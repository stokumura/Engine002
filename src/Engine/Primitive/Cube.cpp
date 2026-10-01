#include "Cube.h"

Cube::Cube(const VertexAttributeTypeLayout &attributes, float length, unsigned int xSubdivisions, unsigned int ySubdivisions, unsigned int zSubdivisions) : Shape(attributes), length(length), xSubdivisions(xSubdivisions), ySubdivisions(ySubdivisions), zSubdivisions(zSubdivisions) {
    Build();
}

Mesh Cube::ConstructMesh(const VertexAttributeTypeLayout &attributes) const {
    VertexLayout layout = VertexAttribute::GetVertexLayout(attributes);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float half = length / 2.0f;
    unsigned int vertexCount = 0;

    auto AddFace = [&](const glm::vec3 &origin, const glm::vec3 &axisU, const glm::vec3 &axisV, const glm::vec3 &normal, unsigned int subU, unsigned int subV)
    {
        unsigned int rowSize = subU + 1;

        for (unsigned int v = 0; v <= subV; v++)
        {
            float fv = static_cast<float>(v) / static_cast<float>(subV);

            for (unsigned int u = 0; u <= subU; u++)
            {
                float fu = static_cast<float>(u) / static_cast<float>(subU);

                glm::vec3 position = origin + (fu * length) * axisU + (fv * length) * axisV;
                glm::vec2 texCoords(fu, fv);

                AddVertex(vertices, attributes, position, texCoords, normal, axisU, axisV);
            }
        }

        for (unsigned int v = 0; v < subV; v++)
        {
            for (unsigned int u = 0; u < subU; u++)
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

        vertexCount += rowSize * (subV + 1);
    };

    AddFace(glm::vec3(half, -half, -half), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), ySubdivisions, zSubdivisions); // +X
    AddFace(glm::vec3(-half, -half, -half), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), zSubdivisions, ySubdivisions); // -X
    AddFace(glm::vec3(-half, half, -half), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), zSubdivisions, xSubdivisions); //+Y
    AddFace(glm::vec3(-half, -half, -half), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), xSubdivisions, zSubdivisions); //-Y
    AddFace(glm::vec3(-half, -half, half), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), xSubdivisions, ySubdivisions); //+Z
    AddFace(glm::vec3(half, -half, -half), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), xSubdivisions, ySubdivisions); //-Z

    return Mesh(layout, vertices, indices, std::vector<const Texture*>());
}
