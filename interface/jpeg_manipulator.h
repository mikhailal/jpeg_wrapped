#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include <jpeglib.h>
}

typedef struct {
    int h;
    int v;
} JpegSampleFactors;

typedef struct {
    unsigned int width;
    unsigned int height;
    int components;
    J_COLOR_SPACE colorspace;
    JpegSampleFactors jpegSampleFactors[MAX_COMPONENTS];
    bool buffer_is_valid = false;
} JpegMetaData;

class JpegManipulator {
public:
public:
    // default ctor
    JpegManipulator();

    // default dtor
    ~JpegManipulator();

    // Loads JPEG data from external file. Should initialize buffers and metadata.
    void LoadFromJpegFile(const char *filename);

    // Loads JPEG data from uncompressed pixels buffer, and from provided metadata
    void LoadFromBuffer(const uint8_t *buffer, const JpegMetaData *metadata);

    // Saves JPEG data to an external JPEG file
    void SaveToJpegFile(const char *filename) const;

    // Returns raw uncompressed pixel buffer (allows modifications)
    uint8_t *GetDataBuffer();

    // Returns JPEG metadata (allows modification)
    JpegMetaData *GetMetadata();

    // Use to change some params before saving to file (like colorspace)
    void SetMetaData(JpegMetaData *metaData);

private:
    // metadata, buffers, etc.
    JpegMetaData metaData{};
    // raw data
    uint8_t *dataBuffer;

    // check metadata
    static bool MetadataValid(int width, int height, int components,
                              J_COLOR_SPACE colorspace, jpeg_component_info *comp_data);

    // fill metadata
    void FillMetadata(int width, int height, int components,
                      J_COLOR_SPACE colorspace, jpeg_component_info *comp_data);

    // convenient wrapper
    bool CheckAndFillMetadata(int width, int height, int components,
                              J_COLOR_SPACE colorspace,
                              jpeg_component_info *compressData);

    // more wrappers
    void FillCompressStruct(jpeg_compress_struct *jpegCompressStruct,
                            jpeg_component_info *compressData) const;

    void SetCompression(jpeg_compress_struct *jpegCompressStruct) const;
};
