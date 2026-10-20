#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "Shape.h"
#include <glm/glm.hpp>

const glm::vec2 QUAD_POSITIONS[4] = {
    glm::vec2(-1.0, 1.0),
    glm::vec2(-1.0, -1.0),
    glm::vec2(1.0, 1.0),
    glm::vec2(1.0, -1.0),
};

class Quad : public Shape {
    public:
        Quad(const VertexAttributeTypeLayout &attributes);
    private:
        Mesh ConstructMesh(const VertexAttributeTypeLayout &attributes) const override;
};
