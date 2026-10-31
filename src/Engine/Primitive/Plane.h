#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/quaternion.hpp>

#include "Shape.h"

class Plane : public Shape {
    private:
        unsigned int xSubdivisions;
        unsigned int ySubdivisions;
        float length;
        glm::quat rotation;

        mutable unsigned int count;
    public:
        Plane(const VertexAttributeTypeLayout &attributes, unsigned int xSubdivisions, unsigned int ySubdivisions, float length, glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    private:
        Mesh ConstructMesh(const VertexAttributeTypeLayout &attributes) const override;
};
