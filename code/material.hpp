#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "utils.hpp"
#include "ray.hpp"
#include "texture.hpp"

class Material;
struct Hit
{
    Vec3 p;
    Vec3 norm;
    double t;
    Material *material;
    double u = 0.0, v = 0.0;
};


class Material
{
public:
    virtual bool scatter(const Ray &ray, const Hit &hit,
                         Vec3 &attenuation, Ray &scattered) const = 0;
    virtual Vec3 illuminate(double u, double v, const Vec3& p) const {
        return Vec3(0,0,0);
    }
};


class Diffuse : public Material
{
public:
    Diffuse(Texture* a) : albedo(a) {}
    virtual bool scatter(const Ray &ray, const Hit &hit,
                         Vec3 &attenuation, Ray &scattered) const
    {
        Vec3 target = hit.p + hit.norm + random_in_unit_sphere();
        scattered = Ray(hit.p, (target - hit.p).normalized());
        attenuation = albedo->value(hit.u, hit.v, hit.p);
        return true;
    }

    Texture* albedo;
};

class Specular : public Material
{
    Texture* albedo;
    double fuzz;

public:
    Specular(Texture* a, double f) : albedo(a), fuzz(clamp(f)) {}
    virtual bool scatter(const Ray &ray, const Hit &hit,
                         Vec3 &attenuation, Ray &scattered) const
    {
        Vec3 reflected = ray.direction().reflect(hit.norm);
        scattered = Ray(hit.p, reflected + random_in_unit_sphere() * fuzz);
        attenuation = albedo->value(hit.u, hit.v, hit.p);
        return (scattered.direction().dot(hit.norm) > 0);
    }
};

class Refract : public Material
{
    bool refract(const Vec3 &v, const Vec3 &n, double n_relative, Vec3 &refr) const
    {
        Vec3 vv = v.normalized();
        double cosv = v.dot(n);
        double det = 1 - n_relative * n_relative * (1 - cosv * cosv);
        if (det > 0)
        {
            refr = (v - n * cosv) * n_relative + n * sqrt(det);
            return true;
        }
        else
            return false;
    }
    double schlick (double cosine, double ref) const
    {
        double r0 = (1 - ref) / (1 + ref);
        r0 *= r0;
        return r0 + (1 - r0) * pow(1 - cosine, 5);
    }
    Vec3 color;

public:
    Refract(double ref, Vec3 c = Vec3(1,1,1)) : ri(ref), color(c) {}
    virtual bool scatter(const Ray &ray, const Hit &hit,
                         Vec3 &attenuation, Ray &scattered) const
    {
        Vec3 out_norm;
        Vec3 reflected = ray.direction().reflect(hit.norm);
        double n_relative;
        attenuation = color;
        Vec3 refracted;
        double cosine;
        if (hit.norm.dot(ray.direction()) > 0)
        {
            out_norm = hit.norm;
            n_relative = ri;
            cosine = ri * ray.direction().dot(hit.norm);
        }
        else
        {
            out_norm = Vec3() - hit.norm;
            n_relative = 1 / ri;
            cosine = -ri * ray.direction().dot(hit.norm);
        }

        if (refract(ray.direction(), out_norm, n_relative, refracted))
        {
            double reflect_prob = schlick(cosine, ri);
            if (reflect_prob <= drand48())
                scattered = Ray(hit.p, refracted);
            else
                scattered = Ray(hit.p, reflected);
        }
        else
        {
            scattered = Ray(hit.p, reflected);
        }
        return true;
    }

    double ri;
};

class DiffuseLight: public Material{
    Texture* emit;
public:
    DiffuseLight(Texture *a): emit(a) {}
    virtual bool scatter(const Ray &ray, const Hit &hit,
                         Vec3 &attenuation, Ray &scattered) const override {
        return false;
    }
    virtual Vec3 illuminate(double u, double v, const Vec3& p) const {
        return emit->value(u, v, p);
    }

};


class Isotropic : public Material {
    public:
        Isotropic(Vec3 c) {albedo = new ConstantTexture(c);}
        Isotropic(Texture* a) : albedo(a) {}

        virtual bool scatter(
            const Ray& r, const Hit& rec, Vec3& attenuation, Ray& scattered) const override {
            scattered = Ray(rec.p, random_in_unit_sphere(), r.time);
            attenuation = albedo->value(rec.u, rec.v, rec.p);
            return true;
        }

    public:
        Texture* albedo = nullptr;
};



#endif