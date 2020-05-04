#ifndef PNG2N64_H
#define PNG2N64_H

#define BINARY_IMAGE_FORMAT_RGBA32 0
#define BINARY_IMAGE_FORMAT_RGBA16 1
#define BINARY_IMAGE_FORMAT_IA16 2
#define BINARY_IMAGE_FORMAT_IA8 3
#define BINARY_IMAGE_FORMAT_IA4 4
#define BINARY_IMAGE_FORMAT_I8 5
#define BINARY_IMAGE_FORMAT_I4 6
#define BINARY_IMAGE_FORMAT_CI8 7
#define BINARY_IMAGE_FORMAT_CI4 8
#define BINARY_IMAGE_FORMAT_1BPP 9

#define RGBA32_PIXEL_SIZE 4

typedef struct
{
    int width;
    int height;
    unsigned char* pixels;
    int format;
} bin_image_t;

int check_arguments(int argc);
int get_format_from_parameter(char* parameter);
unsigned short get_rgba16_color_from_rgba32(unsigned int color32b);
void convert_image_to_rgba16(bin_image_t* source);

#endif