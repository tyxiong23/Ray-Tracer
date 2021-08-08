#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "utils.hpp"

class Texture {
public:
    virtual Vec3 value(double u, double v, const Vec3& p) const = 0;
};

class ConstantTexture: public Texture {
    Vec3 color;

public:
    ConstantTexture() = default;
    ConstantTexture(const Vec3& c): color(c) {}
    virtual Vec3 value(double u, double v, const Vec3& p) const override {
        return color;
    }
};

class CheckerTexture: public Texture {
    Texture *first, *second;
    double scale;
    bool pos[3];
public:
    CheckerTexture() = default;
    CheckerTexture(Texture* t0, Texture* t1, double de = 10): 
                            scale(de), first(t0), second(t1) {
    }
    virtual Vec3 value(double u, double v, const Vec3& p) const override {
        Vec3 pp = p;
        double judge = 1;
        for (int i = 0; i < 3; ++i )  
            judge *= sin(scale * pp[i]);
        if (judge < 0)
            return first->value(u, v, p);
        else
            return second->value(u, v, p);
    }

};

const int NUM = 256;
static double interpolation(double c[2][2][2], double u, double v, double w) {
    double res = 0;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++) 
                res += (i * u + (1-i) * (1-u)) *
                       (j * v + (1-j) * (1-v)) * 
                       (k * w + (1-k) * (1-w)) * c[i][j][k];
    return res;
}


class Perlin {
    static int* perm_x, *perm_y, *perm_z;
    static double* randdouble;
public:
    double noise(const Vec3& p) const {
        double u = p.x - floor(p.x);
        double v = p.y - floor(p.y);
        double w = p.z - floor(p.z);
        // # NO Interpolation
        // int i = int(4 * p.x) & 255, j = int(4 * p.y) % 255, k = int(4 * p.y) & 255;
        // return randdouble[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
        int i = floor(p.x), j = floor(p.y), k = floor(p.z);
        double cc[2][2][2];
        for (int a = 0; a < 2; a++) 
            for (int b = 0; b < 2; b++)
                for (int c = 0; c < 2; c++) {
                    cc[a][b][c] = randdouble[perm_x[(i+a) & 255] ^ perm_y[(j + b) & 255] ^ perm_z[(k + c) & 255]];
                }
        return interpolation(cc, u, v, w);
        
    }
    double turb(const Vec3& p, int depth = 8) const {
        double ans = 0.0, weight = 1;
        Vec3 tmp_vec = p;
        while(depth--){
            ans += weight * noise(tmp_vec);
            weight *= 0.4;
            tmp_vec *= 2;
        }
        return ans;
    }
};

static double* perlin_generate() {
    double* tmp = new double[NUM];
    for (int i = 0; i < NUM; ++i)
        tmp[i] = drand48();
    return tmp;
}
static int* permute_int() {
    int* tmp = new int[NUM];
    for (int i = 0; i < NUM; i++) tmp[i] = i;
    for (int i = NUM - 1; i >= 0; --i) {
        int target = int((drand48() - 0.000001) * (i + 1));
        int t = tmp[target]; tmp[target] = tmp[i]; tmp[i] = t;
    }
    return tmp;
}


double* Perlin::randdouble = perlin_generate();
int* Perlin::perm_x = permute_int();
int* Perlin::perm_y = permute_int();
int* Perlin::perm_z = permute_int();

class NoiseTexture: public Texture {
    Perlin pNoise;
    Vec3 color1, color2;
    double scale;
public:
    NoiseTexture(double ac, Vec3 c1, Vec3 c2 = Vec3()): color1(c1), color2(c2), scale(ac) {}
    virtual Vec3 value(double u, double v, const Vec3& p) const override {
        // double n = pNoise.noise(p * scale);
        double n = sin(5 * (pNoise.noise(p) * scale + 4 * pNoise.turb(p)))/2 + 0.5;
        return color1 * n + color2 * (1 - n);
    }

};

class ImageTexture: public Texture {
    unsigned char* data;
    int w, h;
public:
    ImageTexture() = default;
    ImageTexture(unsigned char* pix, int width, int height):
            data(pix), w(width), h(height) {}
    virtual Vec3 value(double u, double v, const Vec3& p) const override {
        // fprintf(stderr, "%f %f\n", u, v);
        int x = u * w, y = (1-v) * h - 0.001;
        x = x < 0 ? 0 : (x > w - 1 ? w - 1 : x);
        y = y < 0 ? 0 : (y > h - 1 ? h - 1 : y);
        double r = int(data[3*x+3*w*y]) / 255.0;
        double g = int(data[3*x+3*w*y+1]) / 255.0;
        double b = int(data[3*x+3*w*y+2]) / 255.0;
        return Vec3(r, g, b);

    }
};

#endif