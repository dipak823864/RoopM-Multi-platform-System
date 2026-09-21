LOCAL_PATH := $(call my-dir)

# ==========================================================
# Module 1: blend2d_asimd (Dedicated ARM64 ASIMD Kernels)
# ==========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := blend2d_asimd
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../.. \
                   $(LOCAL_PATH)/../../vendor \
                   $(LOCAL_PATH)/../../vendor/blend2d
LOCAL_CFLAGS := -std=c11 -O2 -flto -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DBL_TARGET_OPT_ASIMD -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden
LOCAL_CPPFLAGS := -std=c++20 -O2 -flto -fno-exceptions -fno-rtti -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DBL_TARGET_OPT_ASIMD -DNDEBUG
LOCAL_SRC_FILES := ../../vendor/blend2d/codec/pngops_asimd.cpp \
                   ../../vendor/blend2d/compression/checksum_asimd.cpp \
                   ../../vendor/blend2d/opentype/otglyf_asimd.cpp
include $(BUILD_STATIC_LIBRARY)

# ==========================================================
# Module 2: roopm_app (Complete Web Engine with Blend2D)
# ==========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := roopm_app

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../.. \
                   $(LOCAL_PATH)/../../core \
                   $(LOCAL_PATH)/../../vendor \
                   $(LOCAL_PATH)/../../vendor/quickjs \
                   $(LOCAL_PATH)/../../vendor/yoga \
                   $(LOCAL_PATH)/../../vendor/lexbor \
                   $(LOCAL_PATH)/../../vendor/blend2d

LOCAL_CFLAGS := -std=c11 -O2 -flto -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DCONFIG_VERSION=\"2024-01-13\" -DLEXBOR_STATIC -D_GNU_SOURCE
LOCAL_CPPFLAGS := -std=c++20 -O2 -flto -fno-exceptions -fno-rtti -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DCONFIG_VERSION=\"2024-01-13\" -DLEXBOR_STATIC
LOCAL_LDFLAGS := -Wl,--gc-sections -Wl,--strip-all -flto -u ANativeActivity_onCreate

LOCAL_SRC_FILES := ../../core/roopm_render.cpp \
                   ../../core/roopm_font.cpp \
                   ../../core/roopm_image.cpp \
                   ../../platform/android/platform_android.cpp \
                   ../../app/app.cpp \
                   ../../tests_src/quickjs_unity.c \
                   ../../tests_src/lexbor_unity.c \
                   ../../tests_src/blend2d_unity.cpp \
                   ../../vendor/blend2d/geometry/sizetable.cpp \
                   ../../vendor/yoga/yoga/YGConfig.cpp \
                   ../../vendor/yoga/yoga/YGEnums.cpp \
                   ../../vendor/yoga/yoga/YGNode.cpp \
                   ../../vendor/yoga/yoga/YGNodeLayout.cpp \
                   ../../vendor/yoga/yoga/YGNodeStyle.cpp \
                   ../../vendor/yoga/yoga/YGPixelGrid.cpp \
                   ../../vendor/yoga/yoga/YGValue.cpp \
                   ../../vendor/yoga/yoga/algorithm/AbsoluteLayout.cpp \
                   ../../vendor/yoga/yoga/algorithm/Baseline.cpp \
                   ../../vendor/yoga/yoga/algorithm/Cache.cpp \
                   ../../vendor/yoga/yoga/algorithm/CalculateLayout.cpp \
                   ../../vendor/yoga/yoga/algorithm/FlexLine.cpp \
                   ../../vendor/yoga/yoga/algorithm/PixelGrid.cpp \
                   ../../vendor/yoga/yoga/config/Config.cpp \
                   ../../vendor/yoga/yoga/debug/AssertFatal.cpp \
                   ../../vendor/yoga/yoga/debug/Log.cpp \
                   ../../vendor/yoga/yoga/event/event.cpp \
                   ../../vendor/yoga/yoga/node/LayoutResults.cpp \
                   ../../vendor/yoga/yoga/node/Node.cpp

LOCAL_STATIC_LIBRARIES := blend2d_asimd android_native_app_glue
LOCAL_LDLIBS := -llog -landroid -lm
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
