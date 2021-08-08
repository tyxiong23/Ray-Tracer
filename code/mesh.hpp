#ifndef MESH_H
#define MESH_H
#include <vector>
#include "shape.hpp"



class Mesh : public Object {

public:
    Mesh(const char *filename, Material *m, const Vec3& center, const Vec3& scale, double ry);

    struct TriangleIndex {
        TriangleIndex() {
            x[0] = 0; x[1] = 0; x[2] = 0;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
        int x[3]{};
    };

    std::vector<Vec3> v;
    std::vector<TriangleIndex> t;
    std::vector<Vec3> n;
    virtual bool intersect(const Ray &r, double tmin, double t_max, Hit &h) const override;
    virtual bool bounding_box(double t0, double t1, AABB& box) const override;
    ObjectList* get_all_triangles() {
        ObjectList* tmp_list = new ObjectList();
        for (int triId = 0; triId < (int) t.size(); ++triId) {
            TriangleIndex& triIndex = t[triId];
            Triangle* tri = new Triangle(v[triIndex[0]],
                          v[triIndex[1]], v[triIndex[2]], material);
            tmp_list->add(tri);
        }
        return tmp_list;
    }

private:

    // Normal can be used for light estimation
    void computeNormal();
};



#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

bool Mesh::intersect(const Ray &r, double t_min, double t_max, Hit& hit) const {

    // Optional: Change this brute force method into a faster one.
    bool result = false;
    for (int triId = 0; triId < (int) t.size(); ++triId) {
        TriangleIndex triIndex = t[triId];
        Triangle triangle(v[triIndex[0]],
                          v[triIndex[1]], v[triIndex[2]], material);
        if(triangle.intersect(r, t_min, t_max, hit)) {
            result = true;
            t_max = hit.t;
        }
    }
    return result;
}

bool Mesh::bounding_box(double t0, double t1, AABB& box) const {
    Vec3 minV = v[0], maxV = v[0];
    std::vector<Vec3> vv = v;
    for (int i = 1; v.size(); ++i) {
        for (int r = 0; r < 3; r++) {
            minV[r] = fmin(minV[r], vv[i][r]);
            maxV[r] = fmax(maxV[r], vv[i][r]);
        }
    }
    box = AABB(minV, maxV);
    return true;
}

Mesh::Mesh(const char *filename, Material *mat, const Vec3& center, const Vec3& scale, double ry) {
    material = mat;

    // Optional: Use tiny obj loader to replace this simple one.
    std::ifstream f;
    f.open(filename);
    if (!f.is_open()) {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    char bslash = '/', space = ' ';
    std::string tok;
    int texID;
    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            break;
        }
        if (line.size() < 3) {
            continue;
        }
        if (line.at(0) == '#') {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok) {
            Vec3 vec;
            ss >> vec.x >> vec.y >> vec.z;
            double theta = ry * PI / 180.0;
            double x = vec.x * cos(theta) + vec.z * sin(theta);
            double z = vec.x * -sin(theta) + vec.z * cos(theta);
            vec.x = x; vec.z = z;
            vec = vec.mult(scale) + center;
            v.push_back(vec);
        } else if (tok == fTok) {
            if (line.find(bslash) != std::string::npos) {
                std::replace(line.begin(), line.end(), bslash, space);
                std::stringstream facess(line);
                TriangleIndex trig;
                facess >> tok;
                for (int ii = 0; ii < 3; ii++) {
                    facess >> trig[ii] >> texID;
                    trig[ii]--;
                }
                t.push_back(trig);
            } else {
                TriangleIndex trig;
                for (int ii = 0; ii < 3; ii++) {
                    ss >> trig[ii];
                    trig[ii]--;
                }
                t.push_back(trig);
            }
        }
    }
    computeNormal();

    f.close();
}

void Mesh::computeNormal() {
    n.resize(t.size());
    for (int triId = 0; triId < (int) t.size(); ++triId) {
        TriangleIndex& triIndex = t[triId];
        Vec3 a = v[triIndex[1]] - v[triIndex[0]];
        Vec3 b = v[triIndex[2]] - v[triIndex[0]];
        b = a % b;
        n[triId] = b.normalized();
    }
}
#endif
