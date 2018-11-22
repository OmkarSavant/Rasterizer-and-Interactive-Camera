#include "polygon.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <array>

void Polygon::Triangulate()
{
    for(int i = 0; i < m_verts.size() - 2; i++) {
        Triangle newTri = Triangle();
        newTri.m_indices[0] = 0;
        newTri.m_indices[1] = i+1;
        newTri.m_indices[2] = i+2;
        AddTriangle(newTri);
    }
}

void Polygon::setBoundingBoxes(Triangle& tri)
{
    float xPoints[3];
    float yPoints[3];
    int i = 0;

    std::vector<Vertex> theVerts;

    for (int i = 0; i < 3; i++) {
        theVerts.push_back(m_verts[tri.m_indices[i]]);
    }

    for (Vertex& v : theVerts) {
        xPoints[i] = v.m_pos[0];
        yPoints[i] = v.m_pos[1];
        i += 1;
    }

    float minX  = *std::min_element(xPoints,xPoints+3);
    minX = std::max(0.f,minX);

    float minY = *std::min_element(yPoints,yPoints+3);
    minY = std::max(0.f,minY);

    float maxX  = *std::max_element(xPoints,xPoints+3);
    maxX = std::min(511.f,maxX);

    float maxY = *std::max_element(yPoints,yPoints+3);
    maxY = std::min(511.f,maxY);

    //X1, Y1, X2, Y2, X3, Y3, X4, Y4

    tri.boundingBox = glm::vec4(minX,minY,maxX,maxY);
}

glm::vec3 Polygon::barycentric(Polygon& poly, Triangle& tri, glm::vec3 point) {
    glm::vec3 vert1 = glm::vec3(poly.m_verts[tri.m_indices[0]].m_pos);
    glm::vec3 vert2 = glm::vec3(poly.m_verts[tri.m_indices[1]].m_pos);
    glm::vec3 vert3 = glm::vec3(poly.m_verts[tri.m_indices[2]].m_pos);
    vert1.z = vert2.z = vert3.z = 0;

    glm::vec3 cross12 = glm::cross(point-vert1,vert2-vert1); //S3
    glm::vec3 cross13 = glm::cross(point-vert3,vert1-vert3); //S2
    glm::vec3 cross23 = glm::cross(point-vert3,vert2-vert3); //S1

    float S3 = glm::length(cross12)/2.0;
    float S2 = glm::length(cross13)/2.0;
    float S1 = glm::length(cross23)/2.0;

    glm::vec3 crossWholeTriangle = glm::cross(vert2-vert3,vert1-vert3);
    float S = glm::length(crossWholeTriangle)/2.0;

    return glm::vec3(S1/S, S2/S, S3/S);
}

// Creates a polygon from the input list of vertex positions and colors
Polygon::Polygon(const QString& name, const std::vector<glm::vec4>& pos, const std::vector<glm::vec3>& col)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    for(unsigned int i = 0; i < pos.size(); i++)
    {
        m_verts.push_back(Vertex(pos[i], col[i], glm::vec4(), glm::vec2()));
    }
    Triangulate();
}

// Creates a regular polygon with a number of sides indicated by the "sides" input integer.
// All of its vertices are of color "color", and the polygon is centered at "pos".
// It is rotated about its center by "rot" degrees, and is scaled from its center by "scale" units
Polygon::Polygon(const QString& name, int sides, glm::vec3 color, glm::vec4 pos, float rot, glm::vec4 scale)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    glm::vec4 v(0.f, 1.f, 0.f, 1.f);
    float angle = 360.f / sides;
    for(int i = 0; i < sides; i++)
    {
        glm::vec4 vert_pos = glm::translate(glm::vec3(pos.x, pos.y, pos.z))
                           * glm::rotate(rot, glm::vec3(0.f, 0.f, 1.f))
                           * glm::scale(glm::vec3(scale.x, scale.y, scale.z))
                           * glm::rotate(i * angle, glm::vec3(0.f, 0.f, 1.f))
                           * v;
        m_verts.push_back(Vertex(vert_pos, color, glm::vec4(), glm::vec2()));
    }

    Triangulate();
}

Polygon::Polygon(const QString &name)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon()
    : m_tris(), m_verts(), m_name("Polygon"), mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon(const Polygon& p)
    : m_tris(p.m_tris), m_verts(p.m_verts), m_name(p.m_name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    if(p.mp_texture != nullptr)
    {
        mp_texture = new QImage(*p.mp_texture);
    }
    if(p.mp_normalMap != nullptr)
    {
        mp_normalMap = new QImage(*p.mp_normalMap);
    }
}

Polygon::~Polygon()
{
    delete mp_texture;
}

void Polygon::SetTexture(QImage* i)
{
    mp_texture = i;
}

void Polygon::SetNormalMap(QImage* i)
{
    mp_normalMap = i;
}

void Polygon::AddTriangle(const Triangle& t)
{
    m_tris.push_back(t);
}

void Polygon::AddVertex(const Vertex& v)
{
    m_verts.push_back(v);
}

void Polygon::ClearTriangles()
{
    m_tris.clear();
}

Triangle& Polygon::TriAt(unsigned int i)
{
    return m_tris[i];
}

Triangle Polygon::TriAt(unsigned int i) const
{
    return m_tris[i];
}

Vertex &Polygon::VertAt(unsigned int i)
{
    return m_verts[i];
}

Vertex Polygon::VertAt(unsigned int i) const
{
    return m_verts[i];
}

glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage* const image)
{
    if(image)
    {
        int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
        int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
        QColor color = image->pixel(X, Y);
        return glm::vec3(color.red(), color.green(), color.blue());
    }
    return glm::vec3(255.f, 255.f, 255.f);
}
