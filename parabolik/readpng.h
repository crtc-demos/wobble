#ifndef READPNG_H
#define READPNG_H 1

extern char* readpng_image(char* filename, int* width, int* height, int* alpha);
extern int readpng_bindgl2d(char* pixels, int w, int h, int alpha);

#endif
