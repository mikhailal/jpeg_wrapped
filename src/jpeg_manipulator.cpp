#include "jpeg_manipulator.h"

JpegManipulator::JpegManipulator() {
    dataBuffer = nullptr;
    metaData.height = 0;
    metaData.width = 0;
    metaData.components = 0;
    metaData.colorspace = JCS_UNKNOWN;
    for (auto &jpegSampleFactor : metaData.jpegSampleFactors) {
        jpegSampleFactor.h = 0;
        jpegSampleFactor.v = 0;
    }
    metaData.buffer_is_valid = false;
}

JpegManipulator::~JpegManipulator() {
    delete[] dataBuffer;
    metaData.buffer_is_valid = false;
}

void JpegManipulator::LoadFromJpegFile(const char *filename) {
    struct jpeg_decompress_struct jpegDecompressStruct{};
    struct jpeg_error_mgr jpegErrorMgr{};
    jpegDecompressStruct.err = jpeg_std_error(&jpegErrorMgr);
    jpeg_create_decompress(&jpegDecompressStruct);

    FILE *currentFile = fopen(filename, "rb");
    if (currentFile == nullptr) {
        perror("LoadFromJpegFile: Error opening file: ");
        metaData.buffer_is_valid = false;
        jpeg_destroy_decompress(&jpegDecompressStruct);
        return;
    } else {
        jpeg_stdio_src(&jpegDecompressStruct, currentFile);
        if (jpeg_read_header(&jpegDecompressStruct, TRUE) != JPEG_HEADER_OK) {
            perror("LoadFromJpegFile: Error reading JPEG header. ");
            metaData.buffer_is_valid = false;
            jpeg_destroy_decompress(&jpegDecompressStruct);
            return;
        } else {
            // if (YUV operation); it is always on
            jpegDecompressStruct.out_color_space = JCS_YCbCr;
            if (CheckAndFillMetadata(jpegDecompressStruct.image_width, jpegDecompressStruct.image_height,
                                     jpegDecompressStruct.num_components, jpegDecompressStruct.out_color_space,
                                     jpegDecompressStruct.comp_info)) {
                jpeg_start_decompress(&jpegDecompressStruct);
                unsigned int rowStride = jpegDecompressStruct.output_width * jpegDecompressStruct.output_components;
                JSAMPARRAY buffer = (*jpegDecompressStruct.mem->alloc_sarray)(
                        (j_common_ptr) &jpegDecompressStruct, JPOOL_IMAGE, rowStride, 1);
                delete[] dataBuffer;
                dataBuffer = new uint8_t[rowStride * jpegDecompressStruct.output_height];
                unsigned int writeOffset = 0;
                while (jpegDecompressStruct.output_scanline < jpegDecompressStruct.output_height) {
                    jpeg_read_scanlines(&jpegDecompressStruct, buffer, 1);
                    memcpy(dataBuffer + writeOffset, buffer[0], rowStride);
                    writeOffset += rowStride;
                }
                jpeg_finish_decompress(&jpegDecompressStruct);
                if (fclose(currentFile) != 0) {
                    perror("LoadFromJpegFile: fclose error: ");
                    // it's an unreal case, but...
                    // we'll consider the reading process failed
                    metaData.buffer_is_valid = false;
                    return;
                }
            } else {
                printf("LoadFromJpegFile: Metadata is incorrect");
                metaData.buffer_is_valid = false;
                return;
            }
        }
        metaData.buffer_is_valid = true;
        jpeg_destroy_decompress(&jpegDecompressStruct);
        return;
    }
}

void JpegManipulator::LoadFromBuffer(const uint8_t *buffer,
                                     const JpegMetaData *metadata) {

    if ((buffer == nullptr) || (metadata == nullptr)) {
        printf("Invalid pointers, LoadFromBuffer failed");
    }

    delete[] dataBuffer;
    metaData = *metadata;
    dataBuffer = new uint8_t[metadata->width * metadata->height *
                                    metadata->components];
    memcpy(dataBuffer, buffer,
           metadata->width * metadata->height * metadata->components);
    // here we will consider the buffer legal
    metaData.buffer_is_valid = true;
}

void JpegManipulator::SaveToJpegFile(const char *filename) const {
    struct jpeg_compress_struct jpegCompressStruct{};
    struct jpeg_error_mgr jpegErrorMgr{};

    if (dataBuffer == nullptr) {
        return;
    } else {
        jpeg_create_compress(&jpegCompressStruct);
        jpegCompressStruct.err = jpeg_std_error(&jpegErrorMgr);

        // Here we'll suppose we have no more than 10 color components as libjpeg does
        auto *componentInfo = new jpeg_component_info[MAX_COMPONENTS* sizeof(jpeg_component_info)];
        jpegCompressStruct.comp_info = componentInfo;
        FillCompressStruct(&jpegCompressStruct, componentInfo);
        FILE *currentFile = fopen(filename, "wb");
        jpeg_stdio_dest(&jpegCompressStruct, currentFile);
        jpeg_set_defaults(&jpegCompressStruct);
        jpeg_start_compress(&jpegCompressStruct, TRUE);

        JSAMPROW rowPointer[1];
        const unsigned int bytes_in_row = metaData.width * metaData.components;

        while (jpegCompressStruct.next_scanline < jpegCompressStruct.image_height) {
            rowPointer[0] =
                    (JSAMPLE *) (dataBuffer + jpegCompressStruct.next_scanline * bytes_in_row);
            jpeg_write_scanlines(&jpegCompressStruct, rowPointer, 1);
        }

        jpeg_finish_compress(&jpegCompressStruct);
        jpeg_destroy_compress(&jpegCompressStruct);
        if (fclose(currentFile) != 0) {
            perror("fclose error: ");
            return;
        }
    }
}

uint8_t *JpegManipulator::GetDataBuffer() { return dataBuffer; }

JpegMetaData *JpegManipulator::GetMetadata() { return &metaData; }

void JpegManipulator::SetMetaData(JpegMetaData *metadata) {
    if (metadata != nullptr) {
        metaData = *metadata;
    } else {
        printf("null pointer, %s\n", __func__);
    }
}

bool JpegManipulator::MetadataValid(int width, int height, int components,
                                    J_COLOR_SPACE colorspace,
                                    jpeg_component_info *comp_data) {
    if ((width > 0) && (height > 0) &&
        (comp_data != nullptr) &&
        ((components == 4) || (components == 3)) &&
        (colorspace != JCS_UNKNOWN)) {
        return true;
    } else {
        return false;
    }
}

void JpegManipulator::FillMetadata(int width, int height, int components,
                                   J_COLOR_SPACE colorspace,
                                   jpeg_component_info *comp_data) {
    metaData.width = width;
    metaData.height = height;
    metaData.components = components;
    metaData.colorspace = colorspace;

    // cleanup
    for (auto &jpegSampleFactor : metaData.jpegSampleFactors) {
        jpegSampleFactor.h = 0;
        jpegSampleFactor.v = 0;
    }

    for (int i = 0; i < components; i++) {
        metaData.jpegSampleFactors[i].h = comp_data[i].h_samp_factor;
        metaData.jpegSampleFactors[i].v = comp_data[i].h_samp_factor;
    }
}

bool JpegManipulator::CheckAndFillMetadata(int width, int height,
                                           int components,
                                           J_COLOR_SPACE colorspace,
                                           jpeg_component_info *compressData) {
    if (MetadataValid(width, height, components, colorspace, compressData)) {
        FillMetadata(width, height, components, colorspace, compressData);
        return true;
    } else {
        return false;
    }
}

void JpegManipulator::FillCompressStruct(jpeg_compress_struct *jpegCompressStruct,
                                         jpeg_component_info *compressData) const {
    jpegCompressStruct->image_width = metaData.width;
    jpegCompressStruct->image_height = metaData.height;
    jpegCompressStruct->input_components = metaData.components;
    jpegCompressStruct->in_color_space = metaData.colorspace;
    jpegCompressStruct->comp_info = compressData;
    SetCompression(jpegCompressStruct);
}

void JpegManipulator::SetCompression(jpeg_compress_struct *jpegCompressStruct) const {
    for (int i = 0; i < metaData.components; i++) {
        jpegCompressStruct->comp_info[i].h_samp_factor = metaData.jpegSampleFactors[i].h;
        jpegCompressStruct->comp_info[i].v_samp_factor = metaData.jpegSampleFactors[i].v;
    }
}
