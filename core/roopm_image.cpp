#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>

// Include stb_image with full implementation
#define STB_IMAGE_IMPLEMENTATION
#include "core/stb_image.h"
#include "core/roopm_image.h"

static std::map<std::string, RoopmImage> g_imageCache;
static std::map<std::string, RoopmImage> g_memImageCache;

extern "C" RoopmImage roopm_load_image_from_memory(const unsigned char *buf, int len) {
    if (!buf || len <= 0) return { NULL, 0, 0, false };
    int w = 0, h = 0, channels = 0;
    unsigned char *data = stbi_load_from_memory(buf, len, &w, &h, &channels, 4);
    if (!data || w <= 0 || h <= 0) return { NULL, 0, 0, false };

    uint32_t *argbPixels = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!argbPixels) {
        stbi_image_free(data);
        return { NULL, 0, 0, false };
    }
    for (int i = 0; i < w * h; i++) {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];
        argbPixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    stbi_image_free(data);
    return { argbPixels, w, h, true };
}

extern "C" RoopmImage roopm_load_image_from_memory_cached(const char *key, const unsigned char *buf, int len) {
    if (!key || !buf || len <= 0) return { NULL, 0, 0, false };
    std::string sKey(key);
    if (g_memImageCache.find(sKey) != g_memImageCache.end()) {
        return g_memImageCache[sKey];
    }
    RoopmImage img = roopm_load_image_from_memory(buf, len);
    if (img.isValid) {
        g_memImageCache[sKey] = img;
    }
    return img;
}

extern "C" RoopmImage roopm_load_image(const char *filepath) {
    if (!filepath || strlen(filepath) == 0) return { NULL, 0, 0, false };

    std::string key(filepath);
    if (g_imageCache.find(key) != g_imageCache.end()) {
        return g_imageCache[key];
    }

    int w = 0, h = 0, channels = 0;
    unsigned char *data = stbi_load(filepath, &w, &h, &channels, 4);
    if (!data) {
        std::string wsPath = std::string("workspace/") + filepath;
        data = stbi_load(wsPath.c_str(), &w, &h, &channels, 4);
    }

    if (!data || w <= 0 || h <= 0) {
        return { NULL, 0, 0, false };
    }

    uint32_t *argbPixels = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!argbPixels) {
        stbi_image_free(data);
        return { NULL, 0, 0, false };
    }

    for (int i = 0; i < w * h; i++) {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];
        argbPixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    stbi_image_free(data);
    RoopmImage img = { argbPixels, w, h, true };
    g_imageCache[key] = img;
    return img;
}

extern "C" void roopm_free_image(RoopmImage *img) {
    if (img && img->pixels) {
        free(img->pixels);
        img->pixels = NULL;
        img->isValid = false;
    }
}
