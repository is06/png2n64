#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "png2n64.h"

int main(int argc, char* argv[])
{
    if (check_arguments(argc) != 0) {
        return 1;
    }

    FILE* png_file = fopen(argv[1], "rb");
    if (png_file == NULL) {
        printf("Unable to open the file %s\n", argv[1]);
        return 1;
    }

    png_byte header[8];
    unsigned long header_size = sizeof(header);
    if (fread(header, 1, header_size, png_file) != header_size) {
        printf("Error reading the file %s\n", argv[1]);
        return 1;
    }

    if (png_sig_cmp(header, 0, header_size)) {
        printf("File %s is not a valid PNG file\n", argv[1]);
        return 1;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        printf("Unable to initialize PNG reading\n");
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("Unable to initialize PNG info\n");
        return 1;
    }

    png_init_io(png_ptr, png_file);
    png_set_sig_bytes(png_ptr, header_size);

    png_read_info(png_ptr, info_ptr);

    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);

    if (color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
        printf("PNG format not supported for conversion.\n");
    }

    // Create a RGBA32 binary image
    bin_image_t* binary_image = (bin_image_t*)malloc(sizeof(bin_image_t));

    // Read PNG format
    png_get_IHDR(
        png_ptr,
        info_ptr,
        (png_uint_32*)(&binary_image->width),
        (png_uint_32*)(&binary_image->height),
        &bit_depth,
        &color_type,
        NULL,
        NULL,
        NULL
    );

    // Allocate pixels in rgba32 binary image
    binary_image->pixels = (unsigned char*)malloc(sizeof(unsigned char) * binary_image->width * binary_image->height * RGBA32_PIXEL_SIZE);

    // Format from parameter
    binary_image->format = BINARY_IMAGE_FORMAT_RGBA32;
    if (argc == 3) {
        binary_image->format = get_format_from_parameter(argv[2]);
    }

    // Row pointers for png read
    png_bytep* row_pointers;
    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * binary_image->height);
    for (int i = 0; i < binary_image->height; ++i) {
        row_pointers[i] = (png_bytep)(binary_image->pixels + (i * binary_image->width * RGBA32_PIXEL_SIZE));
    }
    png_read_image(png_ptr, row_pointers);
    free(row_pointers);

    // Close PNG file
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(png_file);

    int output_pixel_size = 4;

    // Convert the binary image before writing to file
    switch (binary_image->format) {
        case BINARY_IMAGE_FORMAT_RGBA16:
            printf("Convert to rgba16...\n");
            convert_image_to_rgba16(binary_image);
            output_pixel_size = 2;
            break;
        default:
            printf("No need for conversion, keep rgba32\n");
    }

    // Writing binary image file
    FILE* bin_file = fopen("result.bin", "wb");

    int binary_image_size = binary_image->width * binary_image->height * output_pixel_size;

    printf("Binary image size: %i\n", binary_image_size);

    fwrite(binary_image->pixels, output_pixel_size, binary_image_size, bin_file);
    
    fclose(bin_file);

    return 0;
}

int check_arguments(int argc)
{
    if (argc < 2 || argc > 3) {
        printf("Usage: png2n64 file [--format=format]\n");
        printf("--format    can be: rgba32 or rgba16\n");
        return 1;
    }

    return 0;
}

int get_format_from_parameter(char* parameter)
{
    if (strcmp(parameter, "rgba16")) {
        return BINARY_IMAGE_FORMAT_RGBA16;
    }

    return 0;
}

unsigned short get_rgba16_color_from_rgba32(unsigned int color32b)
{
    unsigned char red = (color32b & 0xff000000) >> 24;
    unsigned char green = (color32b & 0xff0000) >> 16;
    unsigned char blue = (color32b & 0xff00) >> 8;
    unsigned char alpha = color32b & 0xff;

    red = red / 8;
    green = green / 8;
    blue = blue / 8;
    alpha = alpha == 0 ? 0 : 1;

    return (red << 11) | (green << 6) | (blue << 1) | alpha;
}

void convert_image_to_rgba16(bin_image_t* image)
{
    int rgba32_image_length = image->width * image->height * 4;
    int rgba16_image_length = image->width * image->height * 2;

    unsigned char* rgba16_pixels = (unsigned char*)malloc(sizeof(unsigned char) * rgba16_image_length);
    unsigned char* rgba32_pixels = image->pixels;

    int j = 0;
    for (int i = 0; i < rgba32_image_length; i += 4) {
        unsigned char red = image->pixels[i];
        unsigned char green = image->pixels[i+1];
        unsigned char blue = image->pixels[i+2];
        unsigned char alpha = image->pixels[i+3];

        unsigned int rgba32_color = red << 24 | green << 16 | blue << 8 | alpha;
        unsigned short rgba16_color = get_rgba16_color_from_rgba32(rgba32_color);

        rgba16_pixels[j] = (rgba16_color & 0xff00) >> 8;
        rgba16_pixels[j+1] = rgba16_color & 0xff;

        j += 2;
    }

    image->pixels = rgba16_pixels;
    free(rgba32_pixels);
}
