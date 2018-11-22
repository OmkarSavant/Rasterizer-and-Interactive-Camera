#pragma once
#include "polygon.h"
#include "linesegment.h"
#include "camera.h"
#include <QImage>
#include <limits>
#include <cstddef>

class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    std::vector<Polygon> m_polygons;
public:
    Rasterizer(const std::vector<Polygon>& polygons,char shading);
    QImage RenderScene(char shading);
    void ClearScene();
    camera cam;
    char scheme;
};
