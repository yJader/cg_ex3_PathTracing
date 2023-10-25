//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

void Scene::buildBVH()
{
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    // TO DO Use BVH.cpp's Intersect function instead of the current traversal method
    Intersection dmin, tmp;
    for (auto i : objects)
    {
        tmp = i->getIntersection(ray);
        dmin = dmin.distance > tmp.distance ? tmp : dmin;
    }
    return dmin;
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Vector3f dir = {0, 0, 0}, indir = {0, 0, 0};
    // 判断光线是否与场景中的物体相交
    Intersection inter = intersect(ray);

    // 没有交点
    if (!inter.happened)
    {
        return dir;
    }

    // ray打到光源
    if (inter.m->hasEmission())
    {
        if (depth == 0)
            return inter.m->getEmission();
        else
            return dir;
    }

    // ray打到物体, 进行递归

    // 采样光源
    Intersection light_pos;
    float pdf_light = 0;
    sampleLight(light_pos, pdf_light);

    // 计算直接光照部分
    Vector3f p = inter.coords;
    Vector3f N = inter.normal.normalized();
    Vector3f
}