#ifndef ROOPM_IVFS_H
#define ROOPM_IVFS_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

namespace UIEngine {

// 🌟 Universal W3C Virtual File System Interface (Zero-Hack, Cross-Platform)
class IVFS {
public:
    virtual ~IVFS() = default;
    virtual std::string readString(const std::string &path) = 0;
    virtual std::vector<uint8_t> readBinary(const std::string &path) = 0;
    virtual bool exists(const std::string &path) = 0;
};

// 🌟 Desktop Native Filesystem (Windows / Linux / macOS)
class StdioVFS : public IVFS {
public:
    std::string readString(const std::string &path) override {
        std::ifstream f(path, std::ios::in | std::ios::binary);
        if (!f.is_open()) return "";
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    std::vector<uint8_t> readBinary(const std::string &path) override {
        std::ifstream f(path, std::ios::in | std::ios::binary);
        if (!f.is_open()) return {};
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }

    bool exists(const std::string &path) override {
        std::ifstream f(path);
        return f.good();
    }
};

#ifdef __ANDROID__
// 🌟 Official Android AAssetManager Streaming VFS (Zero-Disk Write, Deflate Supported)
class AndroidAssetVFS : public IVFS {
private:
    AAssetManager *m_mgr = nullptr;

    std::string sanitizePath(const std::string &path) const {
        std::string p = path;
        // Asset paths in AAssetManager must not have leading '/'
        while (!p.empty() && (p.front() == '/' || p.front() == '\\')) p.erase(p.begin());
        return p;
    }

public:
    AndroidAssetVFS(AAssetManager *mgr) : m_mgr(mgr) {}

    std::string readString(const std::string &path) override {
        std::string out;
        if (!m_mgr || path.empty()) return out;

        std::string cleanPath = sanitizePath(path);
        // 🌟 Windows AAPT2 stores paths with backslash on Windows PC: e.g. "workspace\index.html"
        std::string winPath = cleanPath;
        for (char &c : winPath) { if (c == '/') c = '\\'; }

        AAsset *asset = AAssetManager_open(m_mgr, cleanPath.c_str(), AASSET_MODE_STREAMING);
        if (!asset) asset = AAssetManager_open(m_mgr, winPath.c_str(), AASSET_MODE_STREAMING);
        if (!asset) asset = AAssetManager_open(m_mgr, cleanPath.c_str(), AASSET_MODE_UNKNOWN);
        if (!asset) asset = AAssetManager_open(m_mgr, winPath.c_str(), AASSET_MODE_UNKNOWN);
        if (!asset) {
            std::string p2w = "workspace\\" + cleanPath;
            asset = AAssetManager_open(m_mgr, p2w.c_str(), AASSET_MODE_STREAMING);
        }
        if (!asset) return out;

        // AAsset_getLength64 returns exact uncompressed size even for DEFLATE entries
        off64_t len = AAsset_getLength64(asset);
        if (len > 0) out.reserve(static_cast<size_t>(len));

        std::vector<char> chunk(32 * 1024);
        while (true) {
            int n = AAsset_read(asset, chunk.data(), chunk.size());
            if (n > 0) {
                out.append(chunk.data(), static_cast<size_t>(n));
            } else if (n == 0) {
                break; // Clean EOF
            } else {
                out.clear(); // Read/decompress error
                break;
            }
        }
        AAsset_close(asset);
        return out;
    }

    std::vector<uint8_t> readBinary(const std::string &path) override {
        std::vector<uint8_t> out;
        if (!m_mgr || path.empty()) return out;

        std::string cleanPath = sanitizePath(path);
        AAsset *asset = AAssetManager_open(m_mgr, cleanPath.c_str(), AASSET_MODE_STREAMING);
        if (!asset) return out;

        off64_t len = AAsset_getLength64(asset);
        if (len > 0) out.reserve(static_cast<size_t>(len));

        std::vector<uint8_t> chunk(32 * 1024);
        while (true) {
            int n = AAsset_read(asset, chunk.data(), chunk.size());
            if (n > 0) {
                out.insert(out.end(), chunk.data(), chunk.data() + n);
            } else {
                break;
            }
        }
        AAsset_close(asset);
        return out;
    }

    bool exists(const std::string &path) override {
        if (!m_mgr || path.empty()) return false;
        std::string cleanPath = sanitizePath(path);
        std::string winPath = cleanPath;
        for (char &c : winPath) { if (c == '/') c = '\\'; }

        AAsset *asset = AAssetManager_open(m_mgr, cleanPath.c_str(), AASSET_MODE_STREAMING);
        if (!asset) asset = AAssetManager_open(m_mgr, winPath.c_str(), AASSET_MODE_STREAMING);
        if (asset) {
            AAsset_close(asset);
            return true;
        }
        return false;
    }
};
#endif

} // namespace UIEngine

#endif // ROOPM_IVFS_H
