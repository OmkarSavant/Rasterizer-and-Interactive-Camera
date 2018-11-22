#include "camera.h"

camera::camera()
    : forward(glm::vec4(0,0,-1,0)), right(glm::vec4(1,0,0,0)), up(glm::vec4(0,1,0,0)),
      vFOV(45), pos_world(glm::vec4(0,0,10,1)), near_clip(0.01), far_clip(100.0),
      aspect_ratio(1.0) {}

glm::mat4 camera::returnViewMat() {
    glm::mat4 O = glm::mat4(glm::vec4(right[0], up[0], forward[0], 0),
                           glm::vec4(right[1], up[1], forward[1], 0),
                           glm::vec4(right[2], up[2], forward[2], 0),
                           glm::vec4(0, 0, 0, 1));
    glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-pos_world[0], -pos_world[1], -pos_world[2]));

    return O * T;
}

glm::mat4 camera::returnPerspectiveProjMat() {
    float A = 1; //width/height = 1
    float S = 1/(tanf(vFOV/2.f));
    float P = far_clip/(far_clip - near_clip);
    float Q = ((-far_clip)*near_clip)/(far_clip - near_clip);

    return glm::mat4(glm::vec4(S/A,0,0,0),glm::vec4(0,S,0,0),glm::vec4(0,0,P,1),glm::vec4(0,0,Q,0));
}

void camera::transForward(float dist) {
    pos_world = pos_world + glm::vec4(0,0,dist,0);
}

void camera::transRight(float dist) {
    pos_world = pos_world + glm::vec4(dist,0,0,0);
}

void camera::transUp(float dist) {
    pos_world = pos_world + glm::vec4(0,dist,0,0);
}

void camera::rotForward(float theta) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1),theta,glm::vec3(forward));
    right = rotation * right;
    up = rotation * up;
}

void camera::rotRight(float theta) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1),theta,glm::vec3(right));
    forward = rotation * forward;
    up = rotation * up;
}

void camera::rotUp(float theta) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1),theta,glm::vec3(up));
    right = rotation * right;
    forward = rotation * forward;
}





