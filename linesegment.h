#pragma once

#include <vector>
#include <glm/glm.hpp>

class lineSegment
{
public:

    glm::vec4 endpoint1;
    glm::vec4 endpoint2;

    float slope;

    lineSegment(glm::vec4 ep1, glm::vec4 ep2);

    bool getIntersection(int y, float &x, float boundMin, float boundMax);
};

