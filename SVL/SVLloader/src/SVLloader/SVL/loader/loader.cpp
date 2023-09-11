#include "loader.h"

#include <SVL/graphics/texture.h>

SVL::Loader::~Loader()
{
	for(auto tex : textures)
		delete tex.second;
}
