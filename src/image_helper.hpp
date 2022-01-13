#ifndef _IMAGE_HELPER_
#define _IMAGE_HELPER_
#include "OneTool.hpp"

bool LoadTextureFromFile (const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

#endif // !_IMAGE_HELPER_

