#pragma once
#include <string>

class Chunk;

namespace ChunkFileIO
{
	bool LoadChunk(std::string const& saveFolder, std::string const& filename, Chunk* chunk);

	bool SaveChunk(std::string const& saveFolder, std::string const& filename, Chunk const* chunk);

}

