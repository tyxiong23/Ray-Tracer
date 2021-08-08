#include "utils.hpp"
#include "shape.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "image.hpp"
#include "texture.hpp"
#include <cstring>
#include <cmath>
#include "scene_parser.hpp"


const int max_depth = 20;

Vec3 get_color(const Ray &r, Object *objs, const Vec3 &bg, int depth, unsigned short* Xi)
{
    Hit hit;
    if (depth >= max_depth) return Vec3();
    if (objs->intersect(r, 0.001, MAX_double, hit))
    {
        Ray s_ray;
        Vec3 dir = r.direction();
        // fprintf(stderr ,"%f %f %F\n", dir.x, dir.y, dir.z);
        Vec3 attenuation;
        Vec3 illuminated = hit.material->illuminate(hit.u, hit.v, hit.p);
        if  (hit.material->scatter(r, hit, attenuation, s_ray))
        {
            Vec3 f = attenuation;
            double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y: f.z;
            if (++depth > 5)
                if (erand48(Xi) < p)
                    f = f * (1 / p);
                else return illuminated;
            Vec3 new_color = get_color(s_ray, objs, bg, depth + 1, Xi).mult(f);
            return new_color + illuminated;
        }
        else
            return illuminated;
    }

    else return bg;

    //background color
    // {
    //     // fprintf(stderr, "%d ", depth);
    //     Vec3 dir = r.direction();
    //     double t = 0.5 * (dir.y + 1.0);

    //     return Vec3(1, 1, 1) * (1 - t) + Vec3(0.5, 0.7, 1.0) * t;
    // }
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: ./main <input scene file> <output bmp file> <samp:int>\n");
        return 1;
    }
    SceneParser parser = SceneParser(argv[1]);
    
    Camera* camera = parser.getCamera();
    int w = camera->width, h =camera->height, samps = atoi(argv[3]) / 4; // # samples
    Image image(w, h);
    image.setAllPixel(Vec3());
    // ObjectList world = moving_scene();
    // ObjectList world = random_scene();
    // ObjectList world = perlin_scene();
    ObjectList* world = parser.getGroup();
    // Camera* camera = getCam(w, h);

    Vec3 color;
#pragma omp parallel for schedule(dynamic, 1) private(color) // OpenMP

    for (int y = 0; y < h; y++)
    { // Loop over image rows
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps, 100. * y / (h - 1));
        for (unsigned short x = 0, Xi[3] = {0, 0, (unsigned short)(y * y * y)}; x < w; x++){
            color = Vec3();
            // int count = 0;
            for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++)       // 2x2 subpixel rows
                for (int sx = 0; sx < 2; sx++){
                    // fprintf(stderr, "start looping x%d y%d\n", x, y);
                    Vec3 samp_color;
                    for (int s = 0; s < samps; s++)
                    {
                        // ++count;
                        
                        double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        double u = double(x + (sx + 0.5 + dx)/2) / double(w);
                        double v = double(y + (sy + 0.5 + dy)/2) / double(h);
                        // fprintf(stderr, "\nstart_ray");
                        Ray ray = camera->generate_ray(u, v);
                        // fprintf(stderr, "origin %f %f %f, dir %f %f %f\n", ray.o.x, ray.o.y, ray.o.z, ray.d.x, ray.d.y, ray.d.z);
                        // fprintf(stderr, "\nfinish_ray");
                        
                        samp_color += get_color(ray, world, parser.getBackgroundColor(), 0, Xi) * 1.0/ samps;
                        // count++;
                    }
                    color += samp_color.clip() * 0.25;
                }
            image.setPixel(x, y, color);
            // fprintf(stderr, "count %d\n", count);
        }
            
    }
    image.SaveImage(argv[2]);
    return 0;
}