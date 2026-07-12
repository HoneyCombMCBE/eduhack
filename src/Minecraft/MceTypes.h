#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <cstring>

namespace mce {

enum class ImageFormat : int {
    None = 0,
    R8Unorm = 1,
    RGB8Unorm = 2,
    RGBA8Unorm = 3,
};

enum class ImageUsage : unsigned char {
    Unknown = 0,
    sRGB = 1,
    Data = 2,
};

class Blob {
public:
    using value_type = unsigned char;
    using size_type = size_t;
    using pointer = value_type*;
    using delete_function = void (*)(pointer);

    struct Deleter {
        delete_function mFn;
        Deleter() : mFn(defaultDeleter) {}
        Deleter(delete_function fn) : mFn(fn) {}
        void operator()(pointer x) const { mFn(x); }
    };

    using pointer_type = std::unique_ptr<value_type[], Deleter>;

    pointer_type mBlob{};
    size_type mSize{};

    Blob() = default;

    pointer data() const { return mBlob.get(); }
    size_type size() const { return mSize; }

    static void defaultDeleter(pointer ptr) { delete[] ptr; }

    static Blob fromVector(const std::vector<unsigned char>& vec) {
        Blob blob;
        blob.mSize = vec.size();
        blob.mBlob = pointer_type(new value_type[blob.mSize], Deleter());
        std::copy(vec.begin(), vec.end(), blob.mBlob.get());
        return blob;
    }
};

struct Image {
    ImageFormat imageFormat{};
    unsigned int mWidth{};
    unsigned int mHeight{};
    unsigned int mDepth{};
    ImageUsage mUsage{};
    Blob mImageBytes;

    Image() = default;
};

struct UUID {
    uint64_t mLow;
    uint64_t mHigh;
};

} // namespace mce
