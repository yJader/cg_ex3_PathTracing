//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    Intersection()
    {
        happened = false;
        coords = Vector3f();
        normal = Vector3f();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }
    bool happened;
    Vector3f coords;  // 交点坐标
    Vector3f tcoords; // 纹理坐标
    Vector3f normal;  // 交点法向量
    Vector3f emit;    // 物体发射光照
    double distance;  // 光线与交点距离
    Object *obj;      // 交点物体
    Material *m;      // 交点材质
};
#endif // RAYTRACING_INTERSECTION_H
