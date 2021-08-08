#ifndef __RAY_H__
#define __RAY_H__

#include "utils.hpp"

struct Ray
{
    Vec3 o, d;
    double time;

    Ray() = default;
    Ray(const Vec3 &o_, const Vec3 &d_, double t = 0.0)
    {
        o = o_;
        d = d_.normalized();
        time = t;
    }
    Vec3 point(double t) const { return o + d * t; }
    Vec3 origin() const { return o; }
    Vec3 direction() const { return d; }
};


class Camera
{
    Vec3 random_in_unit_disk()
    {
        Vec3 res;
        do
        {
            res = Vec3(drand48(), drand48(), 0) * 2 - Vec3(1, 1, 0) ;
        } while (res.len() >= 1);
        return res;
    }

public:
    Camera(int width_, int height_, Vec3 from, Vec3 at, Vec3 vup, double theta,
           double aperture, double focus_dist, double t0, double t1)
    {
        time0 = t0; time1 = t1; width = width_; height = height_;
        double aspect = double(width) / double(height);
        double hh = tan(theta / 2);
        double ww = aspect * hh;
        origin = from;
        w = (from - at).normalized();
        u = (vup % w).normalized();
        v = w % u;
        horiz = u * 2 * ww * focus_dist;
        verti = v * 2 * hh * focus_dist;
        lower_left = origin - horiz / 2 - verti / 2 - w * focus_dist;
        lens_radius = aperture / 2;
    }
    Ray generate_ray(double hor, double ver)
    {
        Vec3 random_coff = random_in_unit_disk() * lens_radius;
        Vec3 r_vec = u * random_coff.x + v * random_coff.y;
        double time = time0 + (time1 - time0) * drand48();
        return Ray(origin + r_vec, 
            lower_left + horiz * hor + verti * ver - origin - r_vec, time);
    }

    Vec3 origin;
    Vec3 lower_left;
    Vec3 horiz, verti;
    Vec3 u, v, w;
    double lens_radius;
    int width, height;
    double time0, time1; //time interval for taking photo
};

#endif