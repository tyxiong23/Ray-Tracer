#include <iostream>
#include <cstdio>
#include "utils.hpp"
#include "curve.hpp"

int main() {
    Quat4f q;
    q.setAxisAngle(PI/4, Vec3(0, 1, 0));
    Matrix3f rot = Matrix3f::rotation(q);
    Vec3 a = Vec3(1, 0, 0);
    a = rot * a;
    printf("%lf %lf %lf\n", a[0], a[1], a[2]);
    std::vector<Vec3> points;
    // -2 2 0 ]
    //             [ -4 0 0 ]
    //             [ 0 0 0 ]
    //             [ -2 -2 0 ]
    points.push_back(Vec3(-2, 2, 0));
    points.push_back(Vec3(-4, 0, 0));
    points.push_back(Vec3(0,0,0));
    points.push_back(Vec3(-2, -2, 0));
    BezierCurve bc(points);
    double dx = 0.05;
    for (double i = dx / 2; i < 1; i += dx)
        {
            Vec3 v = bc.getPoint(i).V;
            fprintf(stderr,"[%d] %lf %lf %lf", int(i / dx), v[0], v[1], v[2]);
        }
    fprintf(stderr, "\n\n%lf %lf %lf %lf", atan2(0, 1), atan2(1, 0), atan2(0, -1), atan2(-1, 0));
    return 0;
    fprintf(stderr, "%lf %lf %lf %lf", atan2(0, 1), atan2(1, 0), atan2(0, -1), atan2(-1, 0));
}