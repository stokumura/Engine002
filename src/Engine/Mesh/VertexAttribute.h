#pragma once

#include <vector>

enum class VertexAttributeType {
    POSITION2D,
    POSITION3D,
    TEXCOORD,
    NORMAL,
    TANGENT,
    BITANGENT,
};

using VertexAttributeTypeLayout = std::vector<VertexAttributeType>;

const VertexAttributeTypeLayout Vertex2D { VertexAttributeType::POSITION2D, VertexAttributeType::TEXCOORD };
const VertexAttributeTypeLayout Vertex3DUnlit { VertexAttributeType::POSITION3D, VertexAttributeType::TEXCOORD };
const VertexAttributeTypeLayout Vertex3DLit { VertexAttributeType::POSITION3D, VertexAttributeType::TEXCOORD, VertexAttributeType::NORMAL };
const VertexAttributeTypeLayout Vertex3DLitFull { VertexAttributeType::POSITION3D, VertexAttributeType::TEXCOORD, VertexAttributeType::NORMAL, VertexAttributeType::TANGENT, VertexAttributeType::BITANGENT };

struct VertexAttributeLayout {
    unsigned int components;
};

using VertexLayout = std::vector<VertexAttributeLayout>;

class VertexAttribute {
    public:
        static VertexLayout GetVertexLayout(const VertexAttributeTypeLayout &attributes);
    private:
        static unsigned int GetComponentsOfAttributeType(const VertexAttributeType &attributeType); 
};
