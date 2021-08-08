#ifndef __BBOX_H__
#define __BBOX_H__

#include "ray.hpp"
#include "utils.hpp"
// bounding box


class AABB {
    Vec3 minV, maxV;
public:
    AABB() = default;
    AABB(const Vec3& minv, const Vec3& maxv): minV(minv), maxV(maxv) {  } 
    Vec3 min() const {return minV;}
    Vec3 max() const {return maxV;} 
    // 基于slab的长方体求交算法，LEC4  
    bool hit(const Ray& r, double& t_min, double t_max) const {
        Vec3 minVV = minV, maxVV = maxV;
        for (int i = 0; i < 3; i++) {
            double ttmin, ttmax;
            ttmin = fmin((minVV[i] - r.origin()[i])/r.direction()[i],
                              (maxVV[i] - r.origin()[i])/r.direction()[i]);
            ttmax = fmax((minVV[i] - r.origin()[i]) / r.direction()[i],
                               (maxVV[i] - r.origin()[i]) / r.direction()[i]);
            t_min = fmax(t_min, ttmin);
            t_max = fmin(t_max, ttmax);
        }
        if (t_min <= t_max){
            return true;
        }
        else   
            return false;
    }
};

AABB combine_box(const AABB& a, const AABB& b) {
    Vec3 minV (
        fmin(a.min().x, b.min().x),
        fmin(a.min().y, b.min().y),
        fmin(a.min().z, b.min().z)
    );
    Vec3 maxV (
        fmax(a.max().x, b.max().x),
        fmax(a.max().y, b.max().y),
        fmax(a.max().z, b.max().z)
    );
    // fprintf(stderr, "[combine] %f %f %f %f %f %f", minV[0],minV[1],minV[2],maxV[0],maxV[1],maxV[2] );
    // fprintf(stderr, "            %f %f %f %f %f %f", a.min()[0],a.min()[1],a.min()[2],b.min()[0],b.min()[1],b.min()[2] );
    // fprintf(stderr, "            %f %f %f %f %f %f", a.min()[0],a.max()[1],a.max()[2],b.max()[0],b.max()[1],b.max()[2] );
    return AABB(minV, maxV);
}


#endif