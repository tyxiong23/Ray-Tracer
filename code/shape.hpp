#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "utils.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "bbox.hpp"
#include <vector>

const double MAX_double = 10000000000.0;

double det(const Vec3& a, const Vec3& b, const Vec3& c) {
    double res = 0;
    res += a.x * b.y * c.z + a.y * b.z * c.x + a.z * b.x * c.y;
    res -= a.x * b.z * c.y + a.y * b.x * c.z + a.z * b.y * c.x;
    return res;
}


class AABB;
class Object {
public:
    Material *material;
    virtual bool intersect(const Ray &r, double t_min, double t_max, Hit &hit) const = 0;
    virtual bool bounding_box(double t0, double t1, AABB& box) const = 0;
};


class ObjectList : public Object {
    std::vector<Object *> list;

public:
    ObjectList() { list.clear(); }
    void add(Object *obj) {
        list.push_back(obj);
    }
    int size() {
        return list.size();
    }
    Object* & operator[] (int i) {return list[i];}
    std::vector<Object *> getList() {return list;}
    virtual bool intersect(const Ray &r, double t_min, double t_max, Hit &hit) const override {
        bool if_hit = false;
        Hit tmp_hit;
        double tmp_t = t_max;
        for (int i = 0; i < list.size(); ++i) {
            if (list[i]->intersect(r, t_min, tmp_t, tmp_hit)) {
                if_hit = true;
                tmp_t = tmp_hit.t;
                hit = tmp_hit;
            }
        }
        return if_hit;
    }
    virtual bool bounding_box(double t0, double t1, AABB& box) const override {
        if (list.empty()) 
            return false;
        AABB tmp_box;
        bool first_box = true;
        for (const auto& object : list) {
            if (!object->bounding_box(t0, t1, tmp_box)) return false;
            box = first_box ? tmp_box : combine_box(box, tmp_box);
            first_box = false;
        }
        return true;
    }
};


class Sphere : public Object
{
    void get_UV(const Vec3& p, double& u, double& v) const{
        double phi = atan2(p.z, p.x);
        double theta = asin(p.y);
        u = 0.5 - phi / (2 * PI);
        v = 0.5 + theta / PI;
    }
public:
    double radius; // radius
    Vec3 center;  // position, emission, color
    Sphere(double rad_, Vec3 cent, Material *mat) : radius(rad_), center(cent) {
        material = mat;
    }
    virtual bool intersect(const Ray &r, double t_min, double t_max, Hit &hit) const override {
        Vec3 op = center - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t, b = op.dot(r.direction()), det = b * b - op.len2() + radius * radius;
        if (det > 0) {
            double tmp = (b - sqrt(det));
            if (tmp > t_min && tmp < t_max) {
                hit.t = tmp;
                hit.p = r.point(tmp);
                hit.norm = (hit.p - center) / radius;
                hit.norm.normalize();
                hit.material = this->material;
                get_UV(hit.norm, hit.u, hit.v);
                // fprintf(stderr, "true %f\n", tmp);
                return true;
            }
            tmp = (b + sqrt(det));
            if (tmp > t_min && tmp < t_max) {
                hit.t = tmp;
                hit.p = r.point(tmp);
                hit.norm = (hit.p - center) / radius;
                hit.norm.normalize();
                // fprintf(stderr, "norm %f \n", hit.norm.len());
                hit.material = this->material;
                get_UV(hit.norm, hit.u, hit.v);
                return true;
            }
        }
        return false;
    }
    virtual bool bounding_box(double t0, double t1, AABB& box) const override {
        box = AABB(center - Vec3(radius, radius, radius), 
                   center + Vec3(radius, radius, radius));
        return true;
    }
};

class Triangle: public Object {

public:
	Triangle() = delete;
    // a b c are three vertex positions of the triangle
	Triangle( const Vec3& a, const Vec3& b, const Vec3& c, Material* m) {
        material = m;
		vertices[0] = a;
		vertices[1] = b;
		vertices[2] = c;
		this->norm = (b - a) % (c - a);
	    this->norm.normalize();
	}
	virtual bool intersect( const Ray& ray, double t_min, double t_max, Hit& hit) const override {
        double t = 0.0, b = 0.0, r = 0.0;
		Vec3 e1 = vertices[0] - vertices[1], e2 = vertices[0] - vertices[2], s = vertices[0] - ray.origin();
		
        double d0 = det(ray.direction(), e1, e2);
        double d1 = det(s, e1, e2);
        double d2 = det(ray.direction(), s, e2);
        double d3 = det(ray.direction(), e1, s);
		if (d0 == 0.0)
			return false;
		t = d1 / d0;
		b = d2 / d0;
		r = d3 / d0;
		if (t > t_min && t < t_max) {
			if (b >= 0 && b <= 1 && r >= 0 && r <= 1 && b + r <= 1) {
				hit.t = t;
                hit.norm = this->norm;
                hit.p = ray.point(t);
                hit.material = this->material;
                Vec3 calc = vertices[0] - hit.p;
				return true;
			}
		}
		return false;
	}
    Vec3 normal() { return this->norm; }
    virtual bool bounding_box(double t0, double t1, AABB& box) const override {
        Vec3 vert[3];
        for (int i = 0; i < 3; ++i)
            vert[i] = vertices[i];
        Vec3 minV = vert[0], maxV = vert[0];
        for (int i = 1; i < 3; ++i) {
            for (int r = 0; r < 3; r++) {
                minV[r] = fmin(minV[r], vert[i][r]) - 0.00001;
                maxV[r] = fmax(maxV[r], vert[i][r]) + 0.00001;
            }
        }
        box = AABB(minV, maxV);
        // fprintf(stderr, "[tri] min %f %f %f, max %f %f %f \n", box.min().x, box.min().y, box.min().z, box.max().x, box.max().y, box.max().z);
        // for (int i = 0; i < 3; ++i){
        //     for (int j = 0; j < 3; ++j)
        //         fprintf(stderr ,"%f ", vertices[i][j]);
        //     fprintf(stderr ,"\n ");
        // }
            
            
        return true;
    }

private:
	Vec3 norm;
	Vec3 vertices[3];

};


class MovSphere: public Object{
    Vec3 center0, center1;
    double time0, time1;
    double radius;
    Vec3 get_center(double t) const {
        return center0 + (center1 - center0) * (t - time0) / (time1 - time0);
    }

public:
    MovSphere() = delete;
    MovSphere(Vec3 cen0, Vec3 cen1, double t0, double t1, double r, Material* m):
        center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r) {
            material = m;
        }
    virtual bool intersect(const Ray &r, double t_min, double t_max, Hit &hit) const override {
        Vec3 center = get_center(r.time);
        Vec3 op = center - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t, b = op.dot(r.direction()), det = b * b - op.len2() + radius * radius;
        if (det > 0) {
            double tmp = (b - sqrt(det));
            if (tmp > t_min && tmp < t_max) {
                hit.t = tmp;
                hit.p = r.point(tmp);
                hit.norm = (hit.p - center) / radius;
                hit.norm.normalize();
                hit.material = this->material;
                // fprintf(stderr, "true %f\n", tmp);
                return true;
            }
            tmp = (b + sqrt(det));
            if (tmp > t_min && tmp < t_max) {
                hit.t = tmp;
                hit.p = r.point(tmp);
                hit.norm = (hit.p - center) / radius;
                hit.norm.normalize();
                // fprintf(stderr, "norm %f \n", hit.norm.len());
                hit.material = this->material;
                return true;
            }
        }
        return false;
    }
    virtual bool bounding_box(double t0, double t1, AABB& box) const override {
        AABB box1(get_center(t0) - Vec3(radius, radius, radius), 
                   get_center(t0) + Vec3(radius, radius, radius));
        AABB box2(get_center(t1) - Vec3(radius, radius, radius), 
                   get_center(t1) + Vec3(radius, radius, radius));
        box = combine_box(box1, box2);
        return true;
    }
};

class Rectangle: public Object {

public:
	Rectangle() = delete;
    // a b c are three vertex positions of the triangle
	Rectangle( const Vec3& a, const Vec3& b, const Vec3& c, Material* m) {
        material = m;
		vertices[0] = a;
		vertices[1] = b;
        Vec3 e1 = (a - b).normalized(), e2 = (c - b).normalized();
        Vec3 cc = c - (c - b) * e1.dot(e2);
		vertices[2] = cc;

        vertices[3] = a + cc - b;
        tr1 = new Triangle(a, b, cc, m);
        tr2 = new Triangle(a, vertices[3], cc, m);
		this->norm = (b - a) % (cc - a);
	    this->norm.normalize();
	}
	virtual bool intersect( const Ray& ray, double t_min, double t_max, Hit& hit) const override {
        bool res1 = tr1->intersect(ray, t_min, t_max, hit);
        t_max = res1 ? hit.t : t_max;
        bool res2 = tr2->intersect(ray, t_min, t_max, hit);
        double t = 0.0, b = 0.0, r = 0.0;
		res1 =  res1 || res2;
        if (res1) {
            get_UV(hit.p, hit.v, hit.u);
            hit.norm = this->norm;
        }
        return res1;
	}
    Vec3 normal() { return this->norm; }
    virtual bool bounding_box(double t0, double t1, AABB& box) const override {
        AABB b1, b2;
        tr1->bounding_box(t0, t1, b1);
        tr2->bounding_box(t0, t1, b2);
        // fprintf(stderr, "[tri] min %f %f %f, max %f %f %f \n", box.min().x, box.min().y, box.min().z, box.max().x, box.max().y, box.max().z);
        // for (int i = 0; i < 3; ++i){
        //     for (int j = 0; j < 3; ++j)
        //         fprintf(stderr ,"%f ", vertices[i][j]);
        //     fprintf(stderr ,"\n ");
        // }
        box = combine_box(b1, b2);
        return true;
    }

private:
	Vec3 norm;
	Vec3 vertices[4];
    Triangle* tr1, *tr2;
    void get_UV(const Vec3& p, double& u, double& v) const {
        Vec3 e1 = (vertices[0] - vertices[1]).normalized();
        Vec3 e2 = (vertices[2] - vertices[1]).normalized();
        Vec3 tmp = p - vertices[1];
        double x1 = (vertices[0] - vertices[1]).len(), x2 = (vertices[2] - vertices[1]).len();
        u = tmp.dot(e1) / x1;
        v = tmp.dot(e2) / x2;
    }

};






#endif