#pragma once
#include "linesegment.h"
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <cfloat>
#include <iostream>

lineSegment::lineSegment(glm::vec4 ep1, glm::vec4 ep2)
    :endpoint1(ep1), endpoint2(ep2) {

    slope = (ep2[1] - ep1[1])/(ep2[0] - ep1[0]);
}

bool lineSegment::getIntersection(int y, float& x, float boundMin, float boundMax) {


    if(!((y >= endpoint1[1] && y <= endpoint2[1]) || (y >= endpoint2[1] && y <= endpoint1[1]))) {
        return false;
    }
    //Check for 0 slope
    if(slope == 0 || slope == -0) {
        return false;
    }

    //Check for undefined slope
    else if(std::isinf(slope)) {
        x = endpoint1[0];
        return true;
    }

    else{

        float intersection = (float(y) - endpoint1[1] + endpoint1[0]*slope)/slope;
        glm::clamp(intersection, 0.f, 512.f);
        x = intersection;
        return true;
    }
}
