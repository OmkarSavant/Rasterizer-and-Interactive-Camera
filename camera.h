#ifndef CAMERA_H
#define CAMERA_H
#include <glm/gtx/transform.hpp>

class camera
{
public:
    camera();

    glm::vec4 forward;
    glm::vec4 right;
    glm::vec4 up;

    float vFOV;

    glm::vec4 pos_world;

    float near_clip;
    float far_clip;

    float aspect_ratio;

    glm::mat4 returnViewMat();
    glm::mat4 returnPerspectiveProjMat();

    void transForward(float dist);
    void transRight(float dist);
    void transUp(float dist);

    void rotForward(float theta);
    void rotRight(float theta);
    void rotUp(float theta);
};

#endif // CAMERA_H
