#pragma once
typedef enum {
    PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
    PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        // 8*2 bpp (2 channels)
    PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
    PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
    PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
    PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
    PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
    PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
    PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
    PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
    PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
    PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
    PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
    PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
    PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
    PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
    PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
    PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
    PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
    PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
} PixelFormat;

const char* formatNames[] = {
    "",
    "PIXELFORMAT_UNCOMPRESSED_GRAYSCALE",         // 8 bit per pixel (no alpha)
    "PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA",        // 8*2 bpp (2 channels)
    "PIXELFORMAT_UNCOMPRESSED_R5G6B5",            // 16 bpp
    "PIXELFORMAT_UNCOMPRESSED_R8G8B8",            // 24 bpp
    "PIXELFORMAT_UNCOMPRESSED_R5G5B5A1",          // 16 bpp (1 bit alpha)
    "PIXELFORMAT_UNCOMPRESSED_R4G4B4A4",          // 16 bpp (4 bit alpha)
    "PIXELFORMAT_UNCOMPRESSED_R8G8B8A8",          // 32 bpp
    "PIXELFORMAT_UNCOMPRESSED_R32",               // 32 bpp (1 channel - float)
    "PIXELFORMAT_UNCOMPRESSED_R32G32B32",         // 32*3 bpp (3 channels - float)
    "PIXELFORMAT_UNCOMPRESSED_R32G32B32A32",      // 32*4 bpp (4 channels - float)
    "PIXELFORMAT_COMPRESSED_DXT1_RGB",            // 4 bpp (no alpha)
    "PIXELFORMAT_COMPRESSED_DXT1_RGBA",           // 4 bpp (1 bit alpha)
    "PIXELFORMAT_COMPRESSED_DXT3_RGBA",           // 8 bpp
    "PIXELFORMAT_COMPRESSED_DXT5_RGBA",           // 8 bpp
    "PIXELFORMAT_COMPRESSED_ETC1_RGB",            // 4 bpp
    "PIXELFORMAT_COMPRESSED_ETC2_RGB",            // 4 bpp
    "PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA",       // 8 bpp
    "PIXELFORMAT_COMPRESSED_PVRT_RGB",            // 4 bpp
    "PIXELFORMAT_COMPRESSED_PVRT_RGBA",           // 4 bpp
    "PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA",       // 8 bpp
    "PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA"       // 2 bpp
};

// Image type, bpp always RGBA (32bit)
// NOTE: Data stored in CPU memory (RAM)
typedef struct Image {
    void* data;             // Image raw data
    int width;              // Image base width
    int height;             // Image base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)
} Image;

Image LoadDDS(const unsigned char* fileData, unsigned int fileSize)
{
    unsigned char* fileDataPtr = (unsigned char*)fileData;

    // Required extension:
    // GL_EXT_texture_compression_s3tc

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT      0x83F0
    // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT     0x83F1
    // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT     0x83F2
    // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT     0x83F3

#define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII

// DDS Pixel Format
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int fourCC;
        unsigned int rgbBitCount;
        unsigned int rBitMask;
        unsigned int gBitMask;
        unsigned int bBitMask;
        unsigned int aBitMask;
    } DDSPixelFormat;

    // DDS Header (124 bytes)
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int height;
        unsigned int width;
        unsigned int pitchOrLinearSize;
        unsigned int depth;
        unsigned int mipmapCount;
        unsigned int reserved1[11];
        DDSPixelFormat ddspf;
        unsigned int caps;
        unsigned int caps2;
        unsigned int caps3;
        unsigned int caps4;
        unsigned int reserved2;
    } DDSHeader;

    Image image = { 0 };

    if (fileDataPtr != NULL)
    {
        // Verify the type of file
        unsigned char* ddsHeaderId = fileDataPtr;
        fileDataPtr += 4;

        if ((ddsHeaderId[0] != 'D') || (ddsHeaderId[1] != 'D') || (ddsHeaderId[2] != 'S') || (ddsHeaderId[3] != ' '))
        {
            printf("IMAGE: DDS file data not valid\n");
        }
        else
        {
            DDSHeader* ddsHeader = (DDSHeader*)fileDataPtr;

            /*printf("IMAGE: DDS file data info:\n");
            printf("    > Header size:        %i\n", sizeof(DDSHeader));
            printf("    > Pixel format size:  %i\n", ddsHeader->ddspf.size);
            printf("    > Pixel format flags: 0x%x\n", ddsHeader->ddspf.flags);
            printf("    > File format:        0x%x\n", ddsHeader->ddspf.fourCC);
            printf("    > File bit count:     0x%x\n", ddsHeader->ddspf.rgbBitCount);*/

            fileDataPtr += sizeof(DDSHeader);   // Skip header

            image.width = ddsHeader->width;
            image.height = ddsHeader->height;

            if (ddsHeader->mipmapCount == 0) image.mipmaps = 1;      // Parameter not used
            else image.mipmaps = ddsHeader->mipmapCount;

            if (ddsHeader->ddspf.rgbBitCount == 16)     // 16bit mode, no compressed
            {
                if (ddsHeader->ddspf.flags == 0x40)         // no alpha channel
                {
                    int dataSize = image.width * image.height * sizeof(unsigned short);
                    image.data = (unsigned short*)malloc(dataSize);

                    memcpy(image.data, fileDataPtr, dataSize);

                    image.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
                }
                else if (ddsHeader->ddspf.flags == 0x41)        // with alpha channel
                {
                    if (ddsHeader->ddspf.aBitMask == 0x8000)    // 1bit alpha
                    {
                        int dataSize = image.width * image.height * sizeof(unsigned short);
                        image.data = (unsigned short*)malloc(dataSize);

                        memcpy(image.data, fileDataPtr, dataSize);

                        unsigned char alpha = 0;

                        // NOTE: Data comes as A1R5G5B5, it must be reordered to R5G5B5A1
                        for (int i = 0; i < image.width * image.height; i++)
                        {
                            alpha = ((unsigned short*)image.data)[i] >> 15;
                            ((unsigned short*)image.data)[i] = ((unsigned short*)image.data)[i] << 1;
                            ((unsigned short*)image.data)[i] += alpha;
                        }

                        image.format = PIXELFORMAT_UNCOMPRESSED_R5G5B5A1;
                    }
                    else if (ddsHeader->ddspf.aBitMask == 0xf000)   // 4bit alpha
                    {
                        int dataSize = image.width * image.height * sizeof(unsigned short);
                        image.data = (unsigned short*)malloc(dataSize);

                        memcpy(image.data, fileDataPtr, dataSize);

                        unsigned char alpha = 0;

                        // NOTE: Data comes as A4R4G4B4, it must be reordered R4G4B4A4
                        for (int i = 0; i < image.width * image.height; i++)
                        {
                            alpha = ((unsigned short*)image.data)[i] >> 12;
                            ((unsigned short*)image.data)[i] = ((unsigned short*)image.data)[i] << 4;
                            ((unsigned short*)image.data)[i] += alpha;
                        }

                        image.format = PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;
                    }
                }
            }
            else if (ddsHeader->ddspf.flags == 0x40 && ddsHeader->ddspf.rgbBitCount == 24)   // DDS_RGB, no compressed
            {
                int dataSize = image.width * image.height * 3 * sizeof(unsigned char);
                image.data = (unsigned short*)malloc(dataSize);

                memcpy(image.data, fileDataPtr, dataSize);

                image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
            }
            else if (ddsHeader->ddspf.flags == 0x41 && ddsHeader->ddspf.rgbBitCount == 32) // DDS_RGBA, no compressed
            {
                int dataSize = image.width * image.height * 4 * sizeof(unsigned char);
                image.data = (unsigned short*)malloc(dataSize);

                memcpy(image.data, fileDataPtr, dataSize);

                unsigned char blue = 0;

                // NOTE: Data comes as A8R8G8B8, it must be reordered R8G8B8A8 (view next comment)
                // DirecX understand ARGB as a 32bit DWORD but the actual memory byte alignment is BGRA
                // So, we must realign B8G8R8A8 to R8G8B8A8
                for (int i = 0; i < image.width * image.height * 4; i += 4)
                {
                    blue = ((unsigned char*)image.data)[i];
                    ((unsigned char*)image.data)[i] = ((unsigned char*)image.data)[i + 2];
                    ((unsigned char*)image.data)[i + 2] = blue;
                }

                image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            }
            else if (((ddsHeader->ddspf.flags == 0x04) || (ddsHeader->ddspf.flags == 0x05)) && (ddsHeader->ddspf.fourCC > 0)) // Compressed
            {
                int dataSize = 0;

                // Calculate data size, including all mipmaps
                if (ddsHeader->mipmapCount > 1) dataSize = ddsHeader->pitchOrLinearSize * 2;
                else dataSize = ddsHeader->pitchOrLinearSize;

                image.data = (unsigned char*)malloc(dataSize * sizeof(unsigned char));

                memcpy(image.data, fileDataPtr, dataSize);

                switch (ddsHeader->ddspf.fourCC)
                {
                case FOURCC_DXT1:
                {
                    if (ddsHeader->ddspf.flags == 0x04) image.format = PIXELFORMAT_COMPRESSED_DXT1_RGB;
                    else image.format = PIXELFORMAT_COMPRESSED_DXT1_RGBA;
                } break;
                case FOURCC_DXT3: image.format = PIXELFORMAT_COMPRESSED_DXT3_RGBA; break;
                case FOURCC_DXT5: image.format = PIXELFORMAT_COMPRESSED_DXT5_RGBA; break;
                default: break;
                }
            }
        }
    }

    return image;
}