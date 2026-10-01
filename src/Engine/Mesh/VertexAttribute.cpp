#include "VertexAttribute.h"

VertexLayout VertexAttribute::GetVertexLayout(const VertexAttributeTypeLayout &attributes) {
    VertexLayout layout(attributes.size());
    for(int i = 0; i < layout.size(); i++) {
        unsigned int components {GetComponentsOfAttributeType(attributes[i])};
        layout[i] = VertexAttributeLayout { components };
    }
    return layout;
}

unsigned int VertexAttribute::GetComponentsOfAttributeType(const VertexAttributeType &attributeType) {
    switch (attributeType) {
        case VertexAttributeType::TEXCOORD:
        case VertexAttributeType::POSITION2D:
            return 2;
        default:
            return 3;
    }
}
