APP_ABI := arm64-v8a
APP_PLATFORM := android-21
APP_STL := c++_static
APP_OPTIM := release
APP_CFLAGS := -std=c11 -O2 -flto -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1
APP_CPPFLAGS := -std=c++20 -O2 -flto -fno-exceptions -fno-rtti -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1
APP_LDFLAGS := -Wl,--gc-sections -Wl,--strip-all -flto
