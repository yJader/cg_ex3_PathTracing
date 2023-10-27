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
    float emit_area_sum = 0; // 发光区域面积之和
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }

    // 对场景中的所有光源按照加权面积采样, sample一个点 ,计算该sample的pdf
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        // 遍历寻找光源
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf); // 采样, 得到光源单元球面上一个点和pdf
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
    Vector3f L_dir = {0, 0, 0}, L_indir = {0, 0, 0};
    // 判断光线是否与场景中的物体相交
    Intersection obj_inter = bvh->Intersect(ray);

    // 没有交点
    if (!obj_inter.happened)
    {
        return L_dir; // 没有达到物体(没有交点), (0,0,0)
    }

    // ray打到光源
    if (obj_inter.m->hasEmission())
    {
        if (depth == 0)
            return obj_inter.m->getEmission(); // 返回光源信息
        else
            return L_dir;
    }

    // ray打到物体, 返回直接光源和反射光的间接光源
    Vector3f obj_p = obj_inter.coords;              // 交点坐标
    Material *obj_material = obj_inter.m;           // 物体材质
    Vector3f obj_N = obj_inter.normal.normalized(); // 交点法向量
    Vector3f wo = ray.direction;                    // 像素->物体(obj)向量

    // 对光源进行随机采样, 获得一个pdf
    Intersection light_inter;
    float pdf_light = 0;
    sampleLight(light_inter, pdf_light);

    Vector3f light_p = light_inter.coords;              // 光源球面上的点
    Vector3f ws = (light_p - obj_p).normalized();       // obj_p->light_p向量
    Vector3f light_N = light_inter.normal.normalized(); // 球面法向量
    Vector3f emit = light_inter.emit;
    float d = (obj_p - light_p).norm(); // 物体到光源的距离

    // 判断直接光源是否被阻挡
    Ray obj2light(obj_p, ws);
    if (bvh->Intersect(obj2light).distance - d > -0.001)
    {
        Vector3f eval = obj_material->eval(wo, ws, obj_N);
        // L_dir = emit * eval(wo, ws, N) * dot(ws, N) * dot(ws, NN) / | x - p | ^2 / pdf_light
        // ws: obj->light, 方向向球面内; light_N: 方向向球面外
        L_dir = emit * eval * dotProduct(ws, obj_N) * dotProduct(-ws, light_N) / pow(d, 2) / pdf_light;
    }

    // 间接光源
    float P_RR = get_random_float();
    // 轮盘赌
    if (P_RR < RussianRoulette)
    {
        Vector3f wi = obj_material->sample(wo, obj_N); // 间接光源的入射向量
        Ray indir_ray(obj_p, wi);                      // 间接光源入射光线
        // L_indir = shade(q, wi) * eval(wo, wi, N) * dot(wi, N) / pdf(wo, wi, N) / RussianRoulette
        Intersection inter = bvh->Intersect(indir_ray);
        if (inter.happened && !inter.m->hasEmission()) // 间接光源得会发光啊
        {
            Vector3f eval = obj_material->eval(wo, wi, obj_N);
            float pdf_o = obj_material->pdf(wo, wi, obj_N);
            if (pdf_o > EPSILON)
            {
                L_indir = castRay(indir_ray, depth + 1) * eval * dotProduct(wi, obj_N) / pdf_o / RussianRoulette;
            }
        }
    }

    return L_dir + L_indir;
}