#pragma once
int Unpack565(uint8_t const* packed, uint8_t* colour)
{
    // build the packed value
    int value = (int)packed[0] | ((int)packed[1] << 8);

    // get the components in the stored range
    uint8_t red = (uint8_t)((value >> 11) & 0x1f);
    uint8_t green = (uint8_t)((value >> 5) & 0x3f);
    uint8_t blue = (uint8_t)(value & 0x1f);

    // scale up to 8 bits
    colour[0] = (red << 3) | (red >> 2);
    colour[1] = (green << 2) | (green >> 4);
    colour[2] = (blue << 3) | (blue >> 2);
    colour[3] = 255;

    // return the value
    return value;
}

void DecompressColour(uint8_t* rgba, void const* block)
{
    // get the block bytes
    uint8_t const* bytes = reinterpret_cast<uint8_t const*>(block);

    // unpack the endpoints
    uint8_t codes[16];
    int a = Unpack565(bytes, codes);
    int b = Unpack565(bytes + 2, codes + 4);

    // generate the midpoints
    for (int i = 0; i < 3; ++i)
    {
        int c = codes[i];
        int d = codes[4 + i];

        if (a <= b)
        {
            codes[8 + i] = (uint8_t)((c + d) / 2);
            codes[12 + i] = 0;
        }
        else
        {
            codes[8 + i] = (uint8_t)((2 * c + d) / 3);
            codes[12 + i] = (uint8_t)((c + 2 * d) / 3);
        }
    }

    // fill in alpha for the intermediate values
    codes[8 + 3] = 255;
    codes[12 + 3] = (a <= b) ? 0 : 255;

    // unpack the indices
    uint8_t indices[16];
    for (int i = 0; i < 4; ++i)
    {
        uint8_t* ind = indices + 4 * i;
        uint8_t packed = bytes[4 + i];

        ind[0] = packed & 0x3;
        ind[1] = (packed >> 2) & 0x3;
        ind[2] = (packed >> 4) & 0x3;
        ind[3] = (packed >> 6) & 0x3;
    }

    // store out the colours
    for (int i = 0; i < 16; ++i)
    {
        uint8_t offset = 4 * indices[i];
        for (int j = 0; j < 4; ++j)
            rgba[4 * i + j] = codes[offset + j];
    }
}

void Decompress(uint8_t* rgba, void const* block)
{
    // get the block locations
    void const* colourBlock = block;
    void const* alphaBock = block;

    // decompress colour
    DecompressColour(rgba, colourBlock);
}

void DecompressImage(uint8_t* rgba, int width, int height, void const* blocks)
{
    // initialise the block input
    uint8_t const* sourceBlock = reinterpret_cast<uint8_t const*>(blocks);
    int bytesPerBlock = 8;

    // loop over blocks
    for (int y = 0; y < height; y += 4)
    {
        for (int x = 0; x < width; x += 4)
        {
            // decompress the block
            uint8_t targetRgba[4 * 16];
            Decompress(targetRgba, sourceBlock);

            // write the decompressed pixels to the correct image locations
            uint8_t const* sourcePixel = targetRgba;
            for (int py = 0; py < 4; ++py)
            {
                for (int px = 0; px < 4; ++px)
                {
                    // get the target location
                    int sx = x + px;
                    int sy = y + py;
                    if (sx < width && sy < height)
                    {
                        uint8_t* targetPixel = rgba + 4 * (width * sy + sx);

                        // copy the rgba value
                        for (int i = 0; i < 4; ++i)
                            *targetPixel++ = *sourcePixel++;
                    }
                    else
                    {
                        // skip this pixel as its outside the image
                        sourcePixel += 4;
                    }
                }
            }

            // advance
            sourceBlock += bytesPerBlock;
        }
    }
}