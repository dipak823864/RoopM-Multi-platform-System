#ifndef BL_STATIC
#define BL_STATIC 1
#endif
#define BL_BUILD_NO_JIT 1
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define BL_BUILD_OPT_SSE2 1
#define BL_BUILD_OPT_SSSE3 1
#define BL_BUILD_OPT_SSE4_2 1
#elif defined(__aarch64__) || defined(__arm64__)
#define BL_BUILD_OPT_ASIMD 1
#endif
#elif defined(__aarch64__) || defined(__arm64__)
#define BL_BUILD_OPT_ASIMD 1
#endif

#include "vendor/blend2d/core/api-build_p.h"

// Core
#include "vendor/blend2d/core/api-globals.cpp"
#include "vendor/blend2d/core/array.cpp"
#include "vendor/blend2d/core/bitarray.cpp"
#include "vendor/blend2d/core/bitset.cpp"
#include "vendor/blend2d/core/compopinfo.cpp"
#include "vendor/blend2d/core/context.cpp"
#include "vendor/blend2d/core/filesystem.cpp"
#include "vendor/blend2d/core/font.cpp"
#include "vendor/blend2d/core/fontdata.cpp"
#include "vendor/blend2d/core/fontface.cpp"
#include "vendor/blend2d/core/fontfeaturesettings.cpp"
#include "vendor/blend2d/core/fontmanager.cpp"
#include "vendor/blend2d/core/fonttagdataids.cpp"
#include "vendor/blend2d/core/fonttagdatainfo.cpp"
#include "vendor/blend2d/core/fonttagset.cpp"
#include "vendor/blend2d/core/fontvariationsettings.cpp"
#include "vendor/blend2d/core/format.cpp"
#include "vendor/blend2d/core/glyphbuffer.cpp"
#include "vendor/blend2d/core/gradient.cpp"
#include "vendor/blend2d/core/image.cpp"
#include "vendor/blend2d/core/imagecodec.cpp"
#include "vendor/blend2d/core/imagedecoder.cpp"
#include "vendor/blend2d/core/imageencoder.cpp"
#include "vendor/blend2d/core/imagescale.cpp"
#include "vendor/blend2d/core/matrix.cpp"
#include "vendor/blend2d/core/matrix_sse2.cpp"
#include "vendor/blend2d/core/object.cpp"
#include "vendor/blend2d/core/path.cpp"
#include "vendor/blend2d/core/pathstroke.cpp"
#include "vendor/blend2d/core/pattern.cpp"
#include "vendor/blend2d/core/pixelconverter.cpp"
#include "vendor/blend2d/core/pixelconverter_sse2.cpp"
#include "vendor/blend2d/core/random.cpp"
#include "vendor/blend2d/core/runtime.cpp"
#include "vendor/blend2d/core/runtimescope.cpp"
#include "vendor/blend2d/core/string.cpp"
#include "vendor/blend2d/core/trace.cpp"
#include "vendor/blend2d/core/var.cpp"

// Codecs
#include "vendor/blend2d/codec/bmpcodec.cpp"
#include "vendor/blend2d/codec/jpegcodec.cpp"
#include "vendor/blend2d/codec/jpeghuffman.cpp"
#include "vendor/blend2d/codec/jpegops.cpp"
#include "vendor/blend2d/codec/jpegops_sse2.cpp"
#include "vendor/blend2d/codec/pngcodec.cpp"
#include "vendor/blend2d/codec/pngops.cpp"
#include "vendor/blend2d/codec/pngops_sse2.cpp"
#include "vendor/blend2d/codec/qoicodec.cpp"

// Compression & Checksum
#include "vendor/blend2d/compression/checksum.cpp"
#include "vendor/blend2d/compression/checksum_sse2.cpp"
#include "vendor/blend2d/compression/deflatedecoder.cpp"
#include "vendor/blend2d/compression/deflatedecoderfast.cpp"
#include "vendor/blend2d/compression/deflatedecoderutils.cpp"
#include "vendor/blend2d/compression/deflatedefs.cpp"
#include "vendor/blend2d/compression/deflateencoder.cpp"

// OpenType
#include "vendor/blend2d/opentype/otcff.cpp"
#include "vendor/blend2d/opentype/otcmap.cpp"
#include "vendor/blend2d/opentype/otcore.cpp"
#include "vendor/blend2d/opentype/otface.cpp"
#include "vendor/blend2d/opentype/otglyf.cpp"
#include "vendor/blend2d/opentype/otglyfsimddata.cpp"
#include "vendor/blend2d/opentype/otkern.cpp"
#include "vendor/blend2d/opentype/otlayout.cpp"
#include "vendor/blend2d/opentype/otmetrics.cpp"
#include "vendor/blend2d/opentype/otname.cpp"

// Pipeline (Software Reference Pipeline)
#include "vendor/blend2d/pipeline/pipedefs.cpp"
#include "vendor/blend2d/pipeline/piperuntime.cpp"
#include "vendor/blend2d/pipeline/reference/fixedpiperuntime.cpp"

// PixelOps & Raster Engine
#include "vendor/blend2d/pixelops/funcs.cpp"
#include "vendor/blend2d/pixelops/interpolation.cpp"
#include "vendor/blend2d/pixelops/interpolation_sse2.cpp"
#include "vendor/blend2d/raster/rastercontext.cpp"
#include "vendor/blend2d/raster/rastercontextops.cpp"
#include "vendor/blend2d/raster/renderfetchdata.cpp"
#include "vendor/blend2d/raster/rendertargetinfo.cpp"
#include "vendor/blend2d/raster/workdata.cpp"
#include "vendor/blend2d/raster/workermanager.cpp"
#include "vendor/blend2d/raster/workerproc.cpp"
#include "vendor/blend2d/raster/workersynchronization.cpp"

// Support & Threading
#include "vendor/blend2d/support/arenaallocator.cpp"
#include "vendor/blend2d/support/arenahashmap.cpp"
#include "vendor/blend2d/support/math.cpp"
#include "vendor/blend2d/support/scopedallocator.cpp"
#include "vendor/blend2d/support/zeroallocator.cpp"
#include "vendor/blend2d/tables/tables.cpp"
#include "vendor/blend2d/threading/futex.cpp"
#include "vendor/blend2d/threading/thread.cpp"
#include "vendor/blend2d/threading/threadpool.cpp"
#include "vendor/blend2d/threading/uniqueidgenerator.cpp"
#include "vendor/blend2d/unicode/unicode.cpp"
