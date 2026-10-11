#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/quaternion.hpp>

#include "Shape.h"

class Billboard : public Shape {
    private:
        unsigned int amount;
        unsigned int xSubdivisions;
        unsigned int ySubdivisions;
        float length;

        mutable unsigned int count;
    public:
        Billboard(const VertexAttributeTypeLayout &attributes, unsigned int amount, unsigned int xSubdivisions, unsigned int ySubdivisions, float length);
        unsigned int GetIndexCount() const { return count; }
    private:
        Mesh ConstructMesh(const VertexAttributeTypeLayout &attributes) const override;
};
