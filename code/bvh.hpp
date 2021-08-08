#ifndef __BVH_H__
#define __BVH_H__

#include "shape.hpp"
#include "bbox.hpp"
#include <algorithm>
#include <cstdio>

bool box_compare(Object* a, Object*b, int axes) {
    AABB boxA, boxB;
    if (!a->bounding_box(0,0,boxA) || !b->bounding_box(0,0,boxB))
        fprintf(stderr, "No bounding box in bvh_node constructor.\n");
    return boxA.min()[axes] < boxB.min()[axes];
}
bool box_compareX(Object* a, Object*b) { return box_compare(a, b, 0); }
bool box_compareY(Object* a, Object*b) { return box_compare(a, b, 1); }
bool box_compareZ(Object* a, Object*b) { return box_compare(a, b, 2); }


class BVH_node: public Object{
    static int count;

public:
    BVH_node() = default;
    BVH_node(const std::vector<Object*>& sl, int start, int end, double t0, double t1){
        int axis = int(3 * drand48());
        auto l = sl;
        // fprintf(stderr, "start %d  end %d \n", start, end);
        int num = end - start;
        auto *compare_function = (axis == 0) ? box_compareX
                              : (axis == 1) ? box_compareY
                                            : box_compareZ; 
        if (num == 1) {
            lc = rc = l[start];
        } else if (num == 2) {
            // fprintf(stderr, "num = 2 %d\n", count++);
            if (compare_function(l[start], l[start])) {
                lc = l[start]; rc = l[start+1];
            } else {
                lc = l[start+1]; rc = l[start];
            }
        } else {
            std::sort(l.begin() + start, l.begin() + end, compare_function);
            int middle = start + num / 2;
            lc = new BVH_node(l, start, middle, t0, t1);
            rc = new BVH_node(l, middle, end, t0, t1);
            
        }
        AABB boxL, boxR;

        if (!lc->bounding_box (t0, t1, boxL)
            || !rc->bounding_box (t0, t1, boxR)
        )
            fprintf(stderr, "No bounding box in bvh_node constructor.\n");

        nbox = combine_box(boxL, boxR);
        // fprintf(stderr, " ### %d, size of bbox %f \n", 0, (nbox.max() - nbox.min()).len());
    }
    virtual bool intersect(const Ray& r, double t_min, double t_max, Hit& hit) const override {
        double ttmin = t_min;
        if (nbox.hit(r, ttmin, t_max)) {
            Hit l_rec, r_rec;
            bool Lhit = lc->intersect(r, t_min, t_max, l_rec);
            bool Rhit = rc->intersect(r, t_min, t_max, r_rec);
            if (Lhit && Rhit) {
                if (l_rec.t < r_rec.t)
                    hit = l_rec;
                else
                    hit = r_rec;
                return true;
            } else if (Lhit) {
                hit = l_rec;
                return true;
            } else if (Rhit) {
                hit = r_rec;
                return true;
            }
        }
        return false;
    }
    virtual bool bounding_box(double t0, double t1, AABB& box) const override {
        box = nbox;
        return true;
    }

    Object *lc, *rc;
    AABB nbox;
};

int BVH_node::count = 0;

#endif