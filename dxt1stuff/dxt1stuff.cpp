#include <Windows.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <iomanip>

#include "ddsparser.h"
#include "dxt1decompress.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "dxt5decompress.h"

void parse(const char* name)
{
    std::ifstream file(name, std::ios::in | std::ios::binary | std::ios::ate);

    auto size = file.tellg();
    auto memblock = new char[size];
    file.seekg(0, std::ios::beg);
    file.read(memblock, size);
    file.close();

    auto image = LoadDDS((unsigned char*)memblock, size);
    printf("%s data: %dx%d %s\n", name, image.width, image.height, formatNames[image.format]);

    std::string fileName = std::string(name) + ".png";

    if(std::ifstream(fileName))
        std::remove(fileName.c_str());

    uint8_t* bitmap = (uint8_t*)malloc(image.width * image.height * 4);
    
    if (image.format == PixelFormat::PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        printf("DXT1 decompressing\n");
        BlockDecompressImageDXT1(image.width, image.height, (uint8_t*)image.data, (unsigned long*)bitmap);
    }
    else
    {
        printf("DXT5 decompressing\n");
        BlockDecompressImageDXT5(image.width, image.height, (uint8_t*)image.data, (unsigned long*)bitmap);
    }

    stbi_write_png(fileName.c_str(), image.width, image.height, 4, bitmap, image.width * 4);
}

int main()
{
    parse("normal_headshot.dds");
    parse("transparent_headshot.dds");
}