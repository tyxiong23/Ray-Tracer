#ifndef __SCENE_PARSER_H__
#define __SCENE_PARSER_H__

#include <cassert>
#include "utils.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "shape.hpp"
#include "mesh.hpp"
#include "bbox.hpp"
#include "bvh.hpp"
#include "curve.hpp"
#include <vector>
#include "constant_medium.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

// class Light;
// class DirectionalLight;
// class PointLight;

#define MAX_PARSER_TOKEN_LENGTH 1024

class SceneParser {
public:

    SceneParser() = delete;
    SceneParser(const char *filename);

    ~SceneParser();

    Camera *getCamera() const {
        return camera;
    }

    Vec3 getBackgroundColor() const {
        return background_color;
    }

    // int getNumLights() const {
    //     return num_lights;
    // }

    // Light *getLight(int i) const {
    //     assert(i >= 0 && i < num_lights);
    //     return lights[i];
    // }

    int getNumMaterials() const {
        return num_materials;
    }

    Material *getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }

    ObjectList *getGroup() const {
        return group;
    }

private:

    void parseFile();
    void parseCamera();
    void parseBackground();
    // void parseLights();
    // Light *parsePointLight();
    // Light *parseDirectionalLight();
    void parseMaterials();
    Material *parseDiffuse();
    Material *parseSpecular();
    Material *parseRefract();
    Material *parseDiffuseLight();
    Object *parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    ObjectList *parseGroup();
    Sphere *parseSphere();
    MovSphere *parseMovSphere();
    Triangle *parseTriangle();
    Rectangle *parseRectangle();
    Object *parseTriangleMesh();
    Texture* parseTexture();
    Texture* parseConstantTexture();
    Texture* parseCheckerTexture();
    Texture* parseNoiseTexture();
    Texture* parseImageTexture();
    RevSurface *parseRevSurface();
    Curve *parseBezierCurve(const Vec3& scale);
    ConstantMedium* parseMedium();

    // Transform *parseTransform();

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);

    Vec3 readVec3();

    double readDouble();
    int readInt();

    FILE *file;
    Camera *camera;
    Vec3 background_color;
    // int num_lights;
    // Light **lights;
    int num_materials;
    Material **materials;
    Material *current_material;
    ObjectList *group;
};

inline double DegreesToRadians(double x) {
    return acos(-1) * x / 180.0f;
}

SceneParser::SceneParser(const char *filename) {

    // initialize some reasonable default values
    group = nullptr;
    camera = nullptr;
    background_color = Vec3();
    // num_lights = 0;
    // lights = nullptr;
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    // if (num_lights == 0) {
    //     printf("WARNING:    No lights specified\n");
    // }
}

SceneParser::~SceneParser() {

    delete group;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    // for (i = 0; i < num_lights; i++) {
    //     delete lights[i];
    // }
    // delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "Camera")) {
            parseCamera();
        } else if (!strcmp(token, "Background")) {
            parseBackground();
        } 
        // else if (!strcmp(token, "Lights")) {
        //     parseLights();
        // } 
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        } else if (!strcmp(token, "Group")) {
            group = parseGroup();
        } else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseCamera() {
    fprintf(stderr, "start camera\n");
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "from"));
    Vec3 from = readVec3();
    getToken(token);
    assert (!strcmp(token, "at"));
    Vec3 at = readVec3();
    getToken(token);
    assert (!strcmp(token, "up"));
    Vec3 up = readVec3();
    getToken(token);
    assert (!strcmp(token, "angle"));
    double angle_degrees = readDouble();
    double angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert (!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert (!strcmp(token, "height"));
    int height = readInt();
    getToken(token);
    assert (!strcmp(token, "aperture"));
    double aperture = readDouble();
    getToken(token);
    assert (!strcmp(token, "focus_dist"));
    double focus_dist = readDouble();
    getToken(token);
    assert (!strcmp(token, "time0"));
    double time0 = readDouble();
    getToken(token);
    assert (!strcmp(token, "time1"));
    double time1 = readDouble();
    getToken(token);
    assert (!strcmp(token, "}"));
    camera = new Camera(width, height, from, at, up, 
                        angle_radians, aperture, focus_dist, time0, time1);
                        fprintf(stderr, "finish camera\n");
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        } else if (!strcmp(token, "color")) {
            background_color = readVec3();
        } else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

// void SceneParser::parseLights() {
//     char token[MAX_PARSER_TOKEN_LENGTH];
//     getToken(token);
//     assert (!strcmp(token, "{"));
//     // read in the number of objects
//     getToken(token);
//     assert (!strcmp(token, "numLights"));
//     num_lights = readInt();
//     lights = new Light *[num_lights];
//     // read in the objects
//     int count = 0;
//     while (num_lights > count) {
//         getToken(token);
//         if (strcmp(token, "DirectionalLight") == 0) {
//             lights[count] = parseDirectionalLight();
//         } else if (strcmp(token, "PointLight") == 0) {
//             lights[count] = parsePointLight();
//         } else {
//             printf("Unknown token in parseLight: '%s'\n", token);
//             exit(0);
//         }
//         count++;
//     }
//     getToken(token);
//     assert (!strcmp(token, "}"));
// }

// Light *SceneParser::parseDirectionalLight() {
//     char token[MAX_PARSER_TOKEN_LENGTH];
//     getToken(token);
//     assert (!strcmp(token, "{"));
//     getToken(token);
//     assert (!strcmp(token, "direction"));
//     Vec3 direction = readVec3();
//     getToken(token);
//     assert (!strcmp(token, "color"));
//     Vec3 color = readVec3();
//     getToken(token);
//     assert (!strcmp(token, "}"));
//     return new DirectionalLight(direction, color);
// }

// Light *SceneParser::parsePointLight() {
//     char token[MAX_PARSER_TOKEN_LENGTH];
//     getToken(token);
//     assert (!strcmp(token, "{"));
//     getToken(token);
//     assert (!strcmp(token, "position"));
//     Vec3 position = readVec3();
//     getToken(token);
//     assert (!strcmp(token, "color"));
//     Vec3 color = readVec3();
//     getToken(token);
//     assert (!strcmp(token, "}"));
//     return new PointLight(position, color);
// }
// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material *[num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Diffuse")) {
            materials[count] = parseDiffuse();
        } else if (!strcmp(token, "Specular")){
            materials[count] = parseSpecular();
        } else if (!strcmp(token, "Refract")) {
            materials[count] = parseRefract();
        } else if (!strcmp(token, "DiffuseLight")) {
            materials[count] = parseDiffuseLight();
        } 
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Material *SceneParser::parseDiffuse() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Texture* new_texture;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "Texture") == 0) {
            new_texture = parseTexture();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    Material* new_material = new Diffuse(new_texture);
    return new_material;
}

Material *SceneParser::parseSpecular() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Texture* new_texture;
    double fuzz = 0.0;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "Texture") == 0) {
            new_texture = parseTexture();
        } else if (strcmp(token, "fuzz") == 0) {
            fuzz = readDouble();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    Material* new_material = new Specular(new_texture, fuzz);
    return new_material;
}


Material *SceneParser::parseRefract() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    double ref = 1.0;
    Vec3 color(1, 1, 1);
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "ref") == 0) {
            ref = readDouble();
        } else if (strcmp(token, "color") == 0) {
            color = readVec3();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    auto *answer = new Refract(ref, color);
    return answer;
}

Material *SceneParser::parseDiffuseLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    Texture* new_texture;
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "Texture") == 0) {
            new_texture = parseTexture();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    Material* new_material = new DiffuseLight(new_texture);
    return new_material;
}


Texture *SceneParser::parseTexture() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Texture* text;
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    if (strcmp(token, "Constant") == 0) {
        text = parseConstantTexture();
    } else if (strcmp(token, "Checker") == 0) {
        text = parseCheckerTexture();
    } else if (strcmp(token, "Noise") == 0) {
        text = parseNoiseTexture();
    } else if (strcmp(token, "Image") == 0) {
        // Optional: read in texture and draw it.
        text = parseImageTexture();
    }
    getToken(token);
    // fprintf(stderr, "texture token %s\n", token);
    assert (!strcmp(token, "}"));
    return text;
}


Texture *SceneParser::parseConstantTexture() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3 color;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "color") == 0) {
            color = readVec3();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    return new ConstantTexture(color);
}

Texture *SceneParser::parseCheckerTexture() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3 color1, color2;
    double scale;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "color1") == 0) {
            color1 = readVec3();
        } else if (strcmp(token, "color2") == 0) {
            color2 = readVec3();
        } else if (strcmp(token, "scale") == 0) {
            scale = readDouble();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    return new CheckerTexture(new ConstantTexture(color1), new ConstantTexture(color2), scale);
}

Texture *SceneParser::parseNoiseTexture() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3 color1(1, 1, 1), color2(1, 1, 1);
    double scale = 1.0;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "scale") == 0) {
            scale = readDouble();
        } else if (strcmp(token, "color1") == 0) {
            color1 = readVec3();
        } else if (strcmp(token, "color2") == 0) {
            color2 = readVec3();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    return new NoiseTexture(scale, color1, color2);
}

Texture* SceneParser::parseImageTexture() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3 color;
    double scale;
    getToken(token);
    assert (!strcmp(token, "{"));
    unsigned char* data;
    int w, h, comp;
    while (true) {
        getToken(token);
        if (strcmp(token, "src") == 0) {
            getToken(token);
            data = stbi_load(token, &w, &h, &comp, 0);
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    return new ImageTexture(data, w, h);
}

RevSurface *SceneParser::parseRevSurface() {
    // fprintf(stderr, "parser REV\n");
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vec3 center = readVec3();
    getToken(token);
    assert (!strcmp(token, "scale"));
    Vec3 scale = readVec3();
    getToken(token);
    assert (!strcmp(token, "profile"));
    Curve* profile;
    getToken(token);
    if (!strcmp(token, "BezierCurve")) {
        profile = parseBezierCurve(scale);
    } 
    else {
        printf("Unknown profile type in parseRevSurface: '%s'\n", token);
        exit(0);
    }
    getToken(token);
    assert (!strcmp(token, "}"));
    auto *answer = new RevSurface(profile, current_material, center);
    return answer;
}

Curve *SceneParser::parseBezierCurve(const Vec3& scale) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<Vec3> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVec3().mult(scale));
            getToken(token);
            assert (!strcmp(token, "]"));
        } else if (!strcmp(token, "}")) {
            break;
        } else {
            printf("Incorrect format for BezierCurve!\n");
            exit(0);
        }
    }
    // fprintf(stderr, "parser Bezier numpoint %d\n", controls.size());
    Curve *answer = new BezierCurve(controls);
    return answer;
}

// ====================================================================
// ====================================================================

Object *SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object *answer = nullptr;
    if (!strcmp(token, "Group")) {
        answer = (Object *) parseGroup();
    } else if (!strcmp(token, "Sphere")) {
        answer = (Object *) parseSphere();
    }  else if (!strcmp(token, "MovSphere")) {
        answer = (Object *) parseMovSphere();
    } else if (!strcmp(token, "Rectangle")) {
        answer = (Object *) parseRectangle();
    } 
    else if (!strcmp(token, "Triangle")) {
        answer = (Object *) parseTriangle();
    } else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object *) parseTriangleMesh();
    } else if (!strcmp(token, "RevSurface")) {
        answer = (Object*) parseRevSurface();
    }
    else if (!strcmp(token, "Medium")) {
        answer = (Object *) parseMedium();
    } 
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

ObjectList *SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto *answer = new ObjectList();

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        } else {
            Object *object = parseObject(token);
            assert (object != nullptr);
            answer->add(object);

            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere *SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vec3 center = readVec3();
    getToken(token);
    assert (!strcmp(token, "radius"));
    double radius = readDouble();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Sphere(radius, center, current_material);
}

MovSphere *SceneParser::parseMovSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center0"));
    Vec3 center0 = readVec3();
    getToken(token);
    assert (!strcmp(token, "center1"));
    Vec3 center1 = readVec3();
    getToken(token);
    assert (!strcmp(token, "t0"));
    double time0 = readDouble();
    getToken(token);
    assert (!strcmp(token, "t1"));
    double time1 = readDouble();
    getToken(token);
    assert (!strcmp(token, "radius"));
    double radius = readDouble();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new MovSphere(center0, center1, time0, time1, radius, current_material);
}


Triangle *SceneParser::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "vertex0"));
    Vec3 v0 = readVec3();
    getToken(token);
    assert (!strcmp(token, "vertex1"));
    Vec3 v1 = readVec3();
    getToken(token);
    assert (!strcmp(token, "vertex2"));
    Vec3 v2 = readVec3();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Triangle(v0, v1, v2, current_material);
}

Rectangle *SceneParser::parseRectangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "vertex0"));
    Vec3 v0 = readVec3();
    getToken(token);
    assert (!strcmp(token, "vertex1"));
    Vec3 v1 = readVec3();
    getToken(token);
    assert (!strcmp(token, "vertex2"));
    Vec3 v2 = readVec3();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Rectangle(v0, v1, v2, current_material);
}

Object *SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    Vec3 center, scale(1, 1, 1);
    double rotate_Y = 0;
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "obj_file"));
    getToken(filename);
    while (true) {
        getToken(token);
        if (!strcmp(token, "scale")) {
            scale = readVec3();
        } else if (!strcmp(token, "center")){
            center = readVec3();
        } else if (!strcmp(token, "rotateY")){
            rotate_Y = readDouble();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    Mesh *answer = new Mesh(filename, current_material, center, scale, rotate_Y);
    ObjectList* tris = answer->get_all_triangles();
    std::vector<Object*> li = tris->getList();
    BVH_node* root = new BVH_node(li, 0, int(li.size()), 0, 0);
    // fprintf(stderr, "list size %d \n", int(li.size()) );
    // fprintf(stderr, "traingle mesh %d\n", answer->get_all_triangles()->getList().size());
    return root;
}

ConstantMedium *SceneParser::parseMedium() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    Object* obj = parseObject(token);
    getToken(token);
    assert (!strcmp(token, "dense"));
    double dense = readDouble();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vec3 color = readVec3();
    getToken(token);
    assert (!strcmp(token, "}"));
    ConstantMedium* cm = new ConstantMedium(obj, dense, color);
    return cm;
}


// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert (file != nullptr);
    int success = fscanf(file, "%s ", token);
    // fprintf(stderr, "[token] %s\n", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vec3 SceneParser::readVec3() {
    double x, y, z;
    int count = fscanf(file, "%lf %lf %lf", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 doubles to make a Vec3\n");
        assert (0);
    }
    return Vec3(x, y, z);
}


double SceneParser::readDouble() {
    double answer;
    int count = fscanf(file, "%lf", &answer);
    if (count != 1) {
        printf("Error trying to read 1 double\n");
        assert (0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert (0);
    }
    return answer;
}


#endif
