#include <png.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef struct
{
    int width;
    int height;
    char* pixels;
    int format;
} bin_image_t;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: png2n64 file\n");
        return 1;
    }

    FILE* png_file = fopen(argv[1], "rb");
    if (png_file == NULL) {
        printf("Unable to open the file %s\n", argv[1]);
        return 1;
    }

    png_byte header[8];
    int header_size = sizeof(header);
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

    bin_image_t* binary_image = (bin_image_t*)malloc(sizeof(bin_image_t));

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

    // Allocate pixels in struct
    binary_image->pixels = (char*)malloc(sizeof(char) * binary_image->width * binary_image->height * 4);

    // Row pointers for png read
    png_bytep* row_pointers;
    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * binary_image->height);
    for (int i = 0; i < binary_image->height; ++i) {
        row_pointers[i] = (png_bytep)(binary_image->pixels + (i * binary_image->width * 4));
    }
    png_read_image(png_ptr, row_pointers);
    free(row_pointers);

    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(png_file);

    // Writing binary image file
    FILE* bin_file = fopen("result.bin", "wb");
    fwrite(binary_image->pixels, 4, binary_image->width * binary_image->height, bin_file);
    fclose(bin_file);

    return 0;
}