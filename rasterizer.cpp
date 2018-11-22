#pragma once
#include "rasterizer.h"
#include <iostream>
#include <array>

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons, char shading)
    : m_polygons(polygons), cam(camera()), scheme(shading) {}

QImage Rasterizer::RenderScene(char shading)
{
    scheme = shading;
    float z_buffer[562144];

    for(int i = 0; i < 562144; i++) {
        z_buffer[i] = std::numeric_limits<float>::max();
    }
    QImage image(512, 512, QImage::Format_RGB32);
    image.fill(0);

    //Floats to store the line intersections with triangle segments
    float X_intercept = (10000);

    //Sorted intersections here
    glm::mat4 projMat = cam.returnPerspectiveProjMat();
    glm::mat4 viewMat = cam.returnViewMat();

    for (Polygon poly : m_polygons) {

        for(int i = 0; i< poly.m_verts.size(); i++) {
            poly.m_verts[i].m_pos = viewMat*poly.m_verts[i].m_pos;
            poly.m_verts[i].m_pos = projMat*poly.m_verts[i].m_pos;
            float w = poly.m_verts[i].m_pos[3];

            for(int j = 0; j < 4; j++) {
                poly.m_verts[i].m_pos[j] = poly.m_verts[i].m_pos[j] / w;
            }

            poly.m_verts[i].m_pos[0] =  ((poly.m_verts[i].m_pos[0]+1)/2.0)*512.0;
            poly.m_verts[i].m_pos[1] =  ((1 - poly.m_verts[i].m_pos[1])/2.0)*512.0;
        }

        for (Triangle& tri : poly.m_tris) {

            poly.setBoundingBoxes(tri);

            //Extract X,Y coordinates for each vertext in the triangle
            glm::vec4 vert1 = poly.m_verts[tri.m_indices[0]].m_pos;
            glm::vec4 vert2 = poly.m_verts[tri.m_indices[1]].m_pos;
            glm::vec4 vert3 = poly.m_verts[tri.m_indices[2]].m_pos;

            //Create 3 line segments per triangle representing each side
            glm::vec4 side1_ep1(vert1[0],vert1[1],vert1[2],vert1[3]);
            glm::vec4 side1_ep2{vert2[0],vert2[1],vert2[2],vert2[3]};
            lineSegment side1 = lineSegment(side1_ep1,side1_ep2);

            glm::vec4 side2_ep1{vert1[0],vert1[1],vert1[2],vert1[3]};
            glm::vec4 side2_ep2{vert3[0],vert3[1],vert3[2],vert3[3]};
            lineSegment side2 = lineSegment(side2_ep1,side2_ep2);

            glm::vec4 side3_ep1{vert2[0],vert2[1],vert2[2],vert2[3]};
            glm::vec4 side3_ep2{vert3[0],vert3[1],vert3[2],vert3[3]};
            lineSegment side3 = lineSegment(side3_ep1,side3_ep2);

            //Extract vertex colors
            glm::vec3 vert1Color = poly.m_verts[tri.m_indices[0]].m_color;
            glm::vec3 vert2Color = poly.m_verts[tri.m_indices[1]].m_color;
            glm::vec3 vert3Color = poly.m_verts[tri.m_indices[2]].m_color;

            //Extract Vertex U and V
            float vert1U = poly.m_verts[tri.m_indices[0]].m_uv[0];
            float vert2U = poly.m_verts[tri.m_indices[1]].m_uv[0];
            float vert3U = poly.m_verts[tri.m_indices[2]].m_uv[0];

            float vert1V = poly.m_verts[tri.m_indices[0]].m_uv[1];
            float vert2V = poly.m_verts[tri.m_indices[1]].m_uv[1];
            float vert3V = poly.m_verts[tri.m_indices[2]].m_uv[1];

            //Extract normal vector components
            glm::vec4 vert1_norm = poly.m_verts[tri.m_indices[0]].m_normal;
            glm::vec4 vert2_norm = poly.m_verts[tri.m_indices[1]].m_normal;
            glm::vec4 vert3_norm = poly.m_verts[tri.m_indices[2]].m_normal;

            //Vertices U vector - perspective corrected
            glm::vec3 uVec = glm::vec3(vert1U/vert1[2],vert2U/vert2[2],vert3U/vert3[2]);

            //Vertices V vector
            glm::vec3 vVec = glm::vec3(vert1V/vert1[2],vert2V/vert2[2],vert3V/vert3[2]);

            //Vertices Z values - perspective corrected
            glm::vec3 vert_z_values = glm::vec3(1/vert1[2],1/vert2[2],1/vert3[2]);

            std::array<lineSegment,3> segments = {side1, side2, side3};

            //used for filters
            float t = 0.0;

            //Bounding box elements are [minX,minY,maxX,maxY]
            //Iterate through Y values in bounding box
            for (float i = floor(tri.boundingBox[1]); i <= tri.boundingBox[3]; i++) {

                float row_min = 511;
                float row_max = 0;

                // Iterate over each segment
                // If it intersects this row, compare its X intersect to a stored min and max X
                // update min and/or max X if > or < X respectively.
                for (lineSegment& seg : segments) {
                    bool intersects = seg.getIntersection(i,X_intercept,tri.boundingBox[0],tri.boundingBox[2]);
                    if(intersects){
                        if (X_intercept < row_min) {
                            row_min = X_intercept;
                        }
                        if (X_intercept > row_max) {
                            row_max = X_intercept;
                        }
                    }
                }

                row_min = std::fmax(0.f,row_min);
                row_max = std::fmin(511.f,row_max);

                for(float j = ceil(row_min); j <= row_max; j++) {

                    glm::vec3 influenceVec = poly.barycentric(poly, tri, glm::vec3(j,i,0));

                    //Z value
                    float point_Zval = 1/(glm::dot(vert_z_values,influenceVec));

                    glm::vec3 normFinal = glm::normalize(influenceVec.r * glm::vec3(vert1_norm) + influenceVec.g * glm::vec3(vert2_norm) + influenceVec.b * glm::vec3(vert3_norm));

                    glm::vec3 normalized_forward = glm::normalize(glm::vec3(cam.forward));

                    normalized_forward = -1.f * normalized_forward;

                    //Calculate how much light should land on each pixel
                    float light_amt = glm::clamp(glm::dot(normFinal,normalized_forward),0.f,1.f);

                    //UV Interpolation
                    float interpolated_U = point_Zval * glm::dot(uVec,influenceVec);
                    float interpolated_V = point_Zval * glm::dot(vVec,influenceVec);

                    //Blinn-Phong
                    //use normFinal for surfaceNormal
                    //use normalized_forward for viewDirection

                    float angleNormLight = glm::dot(normFinal, normalized_forward);
                    angleNormLight = glm::clamp(angleNormLight, 0.f, 1.f);

                    glm::vec3 halfAng = glm::normalize(normalized_forward + normalized_forward);

                    float blinnTerm = glm::dot(normFinal, halfAng);

                    blinnTerm = glm::clamp(blinnTerm, 0.f, 1.f);
                    if (angleNormLight == 0.0){
                        blinnTerm = 0.0;
                    }

                    blinnTerm = std::pow(blinnTerm, 600);

                    //End Blinn-Phong

                    //Procedurally Generated

                    //Iridescent

                    glm::vec3 a(0.5f,0.5f,0.5f);
                    glm::vec3 b(0.5f,0.5f,0.5f);
                    glm::vec3 c(1.0f,1.0f,1.0f);
                    glm::vec3 d(0.0f,0.33f,0.67f);

                    //Flashlight effect
                    if (scheme == 'G'){
                        a = glm::vec3(0.5f,0.5f,0.5f);
                        b = glm::vec3(0.5f,0.5f,0.5f);
                        c = glm::vec3(1.0f,1.0f,0.5f);
                        d = glm::vec3(0.8f,0.9f,0.3f);
                    }

                    c *= light_amt;
                    c += d;
                    c *= 6.28318f;

                    c[0] = cosf(c[0]);
                    c[1] = cosf(c[1]);
                    c[2] = cosf(c[2]);

                    c *= b;

                    c += a;

                    glm::vec3 pointColor = GetImageColor(glm::vec2(interpolated_U,interpolated_V),poly.mp_texture);

                    //Do Z buffer checks
                    if (z_buffer[int(round(j)) + 512*int(round(i))] >= point_Zval) {

                        z_buffer[int(round(j)) + 512*int(round(i))] = point_Zval;

                        if (scheme == 'L') {
                            image.setPixel(int(round(j)),int(round(i)), qRgb(light_amt*pointColor[0],light_amt*pointColor[1],light_amt*pointColor[2]));
                        }
                        if (scheme == 'B') {
                            image.setPixel(int(round(j)),int(round(i)), qRgb((light_amt+blinnTerm)*pointColor[0],(light_amt+blinnTerm)*pointColor[1],(light_amt+blinnTerm)*pointColor[2]));
                        }
                        if (scheme == 'I' || scheme == 'G') {
                            image.setPixel(int(round(j)),int(round(i)), qRgb((c[0])*pointColor[0],(c[1])*pointColor[1],(c[2])*pointColor[2]));
                        }
                    }
                }
             }
        }
    }
    return image;
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}
