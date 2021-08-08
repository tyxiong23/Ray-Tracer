#include "utils.hpp"
#include "shape.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "image.hpp"
#include "texture.hpp"
#include <cstring>
#include "scene_parser.hpp"



const int max_depth = 50;

ObjectList moving_scene()
{
    ObjectList world;
    Texture* ground_texture = new CheckerTexture(new ConstantTexture(Vec3(0.5, 0.5, 0.5)), new ConstantTexture(Vec3(0.4, 0.08, 0.45)));
    Material *ground_material = new Diffuse(ground_texture);
    Object *ground = new Sphere(1000, Vec3(0, -1000, 0), ground_material);
    world.add(ground);

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = drand48();
            Vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());

            if ((center - Vec3(4, 0.2, 0)).len() > 0.9)
            {
                Material *sphere_material = nullptr;
                Object *new_obj = nullptr;
                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = new ConstantTexture(Vec3(drand48(), drand48(), drand48()));
                    sphere_material = new Diffuse(albedo);
                    new_obj = new MovSphere(center, center+Vec3(0,0.5*drand48(),0), 
                                            0.0, 1.0, 0.2, sphere_material);
                    world.add(new_obj);
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = new ConstantTexture(Vec3(rand_double(0.5, 1), rand_double(0.5, 1), rand_double(0.5, 1)));
                    auto fuzz = rand_double(0, 0.5);
                    sphere_material = new Specular(albedo, fuzz);
                    new_obj = new Sphere(0.2, center, sphere_material);
                    world.add(new_obj);
                }
                else
                {
                    // glass
                    sphere_material = new Refract(1.5);
                    new_obj = new Sphere(0.2, center, sphere_material);
                    world.add(new_obj);
                }
            }
        }
    }

    auto material1 = new Refract(1.5);
    world.add(new Sphere(1.0, Vec3(0, 1, 0), material1));

    auto material2 = new Diffuse(new NoiseTexture(1.5, Vec3(0.4, 0.2, 0.1)));
    world.add(new Sphere(1.0, Vec3(-4, 1, 0), material2));

    auto material3 = new Specular(new ConstantTexture(Vec3(0.7, 0.6, 0.5)), 0.0);
    world.add(new Sphere(1.0, Vec3(4, 1, 0), material3));

    return world;
}

ObjectList random_scene()
{
    ObjectList world;
    Texture* ground_texture = new CheckerTexture(new ConstantTexture(Vec3(0.5, 0.5, 0.5)), new ConstantTexture(Vec3(0.4, 0.08, 0.45)));
    Material *ground_material = new Diffuse(ground_texture);
    Object *ground = new Sphere(1000, Vec3(0, -1000, 0), ground_material);
    world.add(ground);

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = drand48();
            Vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());

            if ((center - Vec3(4, 0.2, 0)).len() > 0.9)
            {
                Material *sphere_material = nullptr;
                Object *new_obj = nullptr;
                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = new ConstantTexture(Vec3(drand48(), drand48(), drand48()));
                    sphere_material = new Diffuse(albedo);
                    new_obj = new Sphere(0.2, center, sphere_material);
                    world.add(new_obj);
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = new ConstantTexture(Vec3(rand_double(0.5, 1), rand_double(0.5, 1), rand_double(0.5, 1)));
                    auto fuzz = rand_double(0, 0.5);
                    sphere_material = new Specular(albedo, fuzz);
                    new_obj = new Sphere(0.2, center, sphere_material);
                    world.add(new_obj);
                }
                else
                {
                    // glass
                    sphere_material = new Refract(1.5);
                    new_obj = new Sphere(0.2, center, sphere_material);
                    world.add(new_obj);
                }
            }
        }
    }

    auto material1 = new Refract(1.5);
    world.add(new Sphere(1.0, Vec3(0, 1, 0), material1));

    auto material2 = new Diffuse(new ConstantTexture(Vec3(0.4, 0.2, 0.1)));
    world.add(new Sphere(1.0, Vec3(-4, 1, 0), material2));

    auto material3 = new Specular(new ConstantTexture(Vec3(0.7, 0.6, 0.5)), 0.0);
    world.add(new Sphere(1.0, Vec3(4, 1, 0), material3));

    return world;
}

Camera* getCam(int w, int h) {
    // camera
    Vec3 lookfrom(13, 2, 3);
    Vec3 lookat(0, 0, 0);
    Vec3 vup(0, 1, 0);
    double dist_to_focus = 10.0;
    double aperture = 0;
    double t1 = 0.0, t2 = 1.0;
    return new Camera(w, h, lookfrom, lookat, vup, 20 * PI / 180, aperture, dist_to_focus, t1, t2);
}

ObjectList perlin_scene()
{
    ObjectList world;
    Material *ground_material1 = new Diffuse(new ConstantTexture(Vec3(0.6, 0.6, 0.6)));
    Material *ground_material = new Diffuse(new CheckerTexture(new ConstantTexture(Vec3(0.5, 0.5, 0.5)), new ConstantTexture(Vec3(0.6, 0.9, 0.4))));
    Object *ground = new Sphere(1000, Vec3(0, -1000, 0), ground_material);
    world.add(ground);
    world.add(new Sphere(1000, Vec3(-1005, 0, 0), ground_material1));
    world.add(new Sphere(1000, Vec3(0, 1003, 0), ground_material1));
    world.add(new Sphere(1000, Vec3(0, 0, 1010), ground_material));
    world.add(new Sphere(1000, Vec3(0, 0, -1003), ground_material));
    int width, height, comp;
    unsigned char* data = stbi_load("images/car.jpeg", &width, &height, &comp, 0);
    auto material2 = new Diffuse(new ImageTexture(data, width, height));
    world.add(new Sphere(1, Vec3(1, 1.5, 0), material2));
    auto material3 = new DiffuseLight(new ConstantTexture(Vec3(90, 90, 90)));
    world.add (new Triangle(Vec3(2, 0, -1), Vec3(2, 1, -1), Vec3(0, 0, -2), material3));
    // world.add(new Sphere(1, Vec3(1, 1.5, 3.5), material3));
    return world;
}


Vec3 get_color(const Ray &r, Object *objs, Vec3 bg, int depth)
{
    Hit hit;
    if (objs->intersect(r, 0.001, MAX_double, hit))
    {
        Ray s_ray;
        Vec3 dir = r.direction();
        // fprintf(stderr ,"%f %f %F\n", dir.x, dir.y, dir.z);
        Vec3 attenuation;
        Vec3 illuminated = hit.material->illuminate(hit.u, hit.v, hit.p);
        if (depth > 0 && hit.material->scatter(r, hit, attenuation, s_ray))
        {
            Vec3 new_color = get_color(s_ray, objs, bg, depth - 1).mult(attenuation);
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
    if (argc != 2) {
        fprintf(stderr, "Usage: ./main <samp:int>\n");
        return 1;
    }
    // SceneParser parser = SceneParser(argv[1]);
    
    // Camera* camera = parser.getCamera();
    int w = 1024, h = 648, samps = atoi(argv[1]); // # samples
    Image image(w, h);
    image.setAllPixel(Vec3());
    // ObjectList world = moving_scene();
    ObjectList world = moving_scene();
    // ObjectList* world = &perlin_scene();
    // ObjectList* world = parser.getGroup();
    Camera* camera = getCam(w, h);

    Vec3 color;
#pragma omp parallel for schedule(dynamic, 1) private(color) // OpenMP

    for (int y = 0; y < h; y++)
    { // Loop over image rows
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps, 100. * y / (h - 1));
        for (unsigned short x = 0; x < w; x++)
        { // Loop cols
            color = Vec3();
            // fprintf(stderr, "start looping x%d y%d\n", x, y);
            for (int s = 0; s < samps; s++)
            {
                double u = double(x + drand48()) / double(w - 1);
                double v = double(y + drand48()) / double(h - 1);
                // fprintf(stderr, "\nstart_ray");
                Ray ray = camera->generate_ray(u, v);
                // fprintf(stderr, "origin %f %f %f, dir %f %f %f\n", ray.o.x, ray.o.y, ray.o.z, ray.d.x, ray.d.y, ray.d.z);
                // fprintf(stderr, "\nfinish_ray");
                
                color += get_color(ray, &world, Vec3(1,1, 1.0), max_depth);
            }
            image.setPixel(x, y, color / samps);
        }
    }
    image.SaveImage("spheres.png");
    return 0;
}