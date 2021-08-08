#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "utils.hpp"
#include <cstring>
#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"




class Image{
    Vec3* rec = nullptr;
    int toInt(double f){
        return int(255.99 * f);
    }
    unsigned char toUC(double f) {
        return (unsigned char)(int(255.99 * f));
    }
public:
    Image (int w, int h) {
        width = w;
        height = h;
        rec = new Vec3[width * height];
    }
    ~Image() {
        delete [] rec;
    }
    void setPixel(int x, int y, const Vec3& color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        rec[y * width + x] = color;
    }
    void setAllPixel(const Vec3& color) {
        for (int i = 0; i < width * height; ++i) {
            rec[i] = color;
        }
    }
    void savePPM(const char* fil_name) {
        FILE *f = fopen(fil_name, "w"); // Write image to PPM file.
        fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
        for (int y = height - 1; y >= 0; y--) 
            for (int x=0; x < width; x++) {
                int i = y * width + x;
                fprintf(f, "%d %d %d ", toInt(rec[i].x), toInt(rec[i].y), toInt(rec[i].z));
            }
        fclose(f);
    }
    int saveBMP(const char* filename) {
        unsigned char *Int_rec = new unsigned char [3 * width * height];
        for (int y = height - 1; y >= 0; y--) 
            for (int x=0; x < width; x++) {
                int i = y * width + x;
                int k = (height - y - 1) * width + x;
                for (int j = 0; j < 3; j++)
                    Int_rec[3 * i + j] = toUC((rec[k])[j]);
            }
        int res  = stbi_write_bmp(filename, width, height, 3, Int_rec);
        delete [] Int_rec;
        return res;
    }
    int savePNG(const char* filename) {
        unsigned char *Int_rec = new unsigned char [3 * width * height];
        for (int y = height - 1; y >= 0; y--) 
            for (int x=0; x < width; x++) {
                int i = y * width + x;
                int k = (height - y - 1) * width + x;
                for (int j = 0; j < 3; j++)
                    Int_rec[3 * i + j] = toUC((rec[k])[j]);
            }
        int res  = stbi_write_png(filename, width, height, 3, Int_rec, 0);
        delete [] Int_rec;
        return res;
    }
    void SaveImage(const char * filename) {
        int len = strlen(filename);
        if(strcmp(".bmp", filename+len-4)==0){
            saveBMP(filename);
        }else if(strcmp(".png", filename+len-4)==0){
            savePNG(filename);
        }else{
            savePPM(filename);
        }
    }

    int width, height;

};

#endif