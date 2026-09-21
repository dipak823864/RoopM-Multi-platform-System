#ifndef ROOPM_IMAGE_H
#define ROOPM_IMAGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
    bool isValid;
} RoopmImage;

RoopmImage roopm_load_image(const char *filepath);
RoopmImage roopm_load_image_from_memory(const unsigned char *buf, int len);
RoopmImage roopm_load_image_from_memory_cached(const char *key, const unsigned char *buf, int len);
void roopm_free_image(RoopmImage *img);

#ifdef __cplusplus
}
#endif

#endif // ROOPM_IMAGE_H
