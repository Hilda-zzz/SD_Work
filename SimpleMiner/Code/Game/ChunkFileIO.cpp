#include "ChunkFileIO.hpp"
#include <vector>
#include <Engine/Core/FileUtils.hpp>
#include "Chunk.hpp"

bool ChunkFileIO::LoadChunk(std::string const& saveFolder, std::string const& filename, Chunk* chunk)
{
	std::vector<uint8_t> fileBuffer;
	int bytesRead = FileReadToBuffer(fileBuffer, saveFolder+filename);

	if (bytesRead == -1)
	{
		printf("Failed to read chunk file: %s\n", filename.c_str());
		return false;
	}

	// Check minimum file size
	if (fileBuffer.size() < sizeof(ChunkFileHeader))
	{
		printf("Chunk file too small: %s (size: %zu, minimum: %zu)\n",
			filename.c_str(), fileBuffer.size(), sizeof(ChunkFileHeader));
		return false;
	}

	// Read and validate header
	ChunkFileHeader header;
	size_t offset = 0;

	// Copy header data from buffer
	header.m_fourCC[0] = fileBuffer[offset++];
	header.m_fourCC[1] = fileBuffer[offset++];
	header.m_fourCC[2] = fileBuffer[offset++];
	header.m_fourCC[3] = fileBuffer[offset++];
	header.m_version = fileBuffer[offset++];
	header.m_chunkBitsX = fileBuffer[offset++];
	header.m_chunkBitsY = fileBuffer[offset++];
	header.m_chunkBitsZ = fileBuffer[offset++];

	// Validate header
	if (header.m_fourCC[0] != 'G' || header.m_fourCC[1] != 'C' ||
		header.m_fourCC[2] != 'H' || header.m_fourCC[3] != 'K')
	{
		printf("Invalid chunk file header (bad fourCC): %s\n", filename.c_str());
		return false;
	}

	if (header.m_version != 1)
	{
		printf("Unsupported chunk file version: %d in file %s\n",
			header.m_version, filename.c_str());
		return false;
	}

	if (header.m_chunkBitsX != 4 || header.m_chunkBitsY != 4 || header.m_chunkBitsZ != 7)
	{
		printf("Chunk dimensions mismatch in file %s (got %d,%d,%d, expected 4,4,7)\n",
			filename.c_str(), header.m_chunkBitsX, header.m_chunkBitsY, header.m_chunkBitsZ);
		return false;
	}

	// Decode RLE data
	int blockIndex = 0;
	//size_t totalRuns = (fileBuffer.size() - sizeof(ChunkFileHeader)) / sizeof(ChunkFileRun);

	while (offset < fileBuffer.size() && blockIndex < BLOCKS_PER_CHUNK)
	{
		// Check if we have enough bytes for a complete run
		if (offset + 1 >= fileBuffer.size())
		{
			printf("Incomplete run data in chunk file: %s\n", filename.c_str());
			return false;
		}

		// Read run data
		ChunkFileRun currentRun;
		currentRun.blockType = fileBuffer[offset++];
		currentRun.runLength = fileBuffer[offset++];

		// Validate run length
		if (currentRun.runLength == 0)
		{
			printf("Invalid run length (0) in chunk file: %s\n", filename.c_str());
			return false;
		}

		// Check if run would exceed chunk bounds
		if (blockIndex + currentRun.runLength > BLOCKS_PER_CHUNK)
		{
			printf("Run extends beyond chunk bounds in file %s (index: %d, length: %d, max: %d)\n",
				filename.c_str(), blockIndex, currentRun.runLength, BLOCKS_PER_CHUNK);
			return false;
		}

		// Apply run to blocks
		for (int i = 0; i < currentRun.runLength; i++)
		{
			Block curBlock = Block(currentRun.blockType);
			chunk->SetBlock(blockIndex, curBlock);
			blockIndex++;
		}
	}

	if (blockIndex != BLOCKS_PER_CHUNK)
	{
		printf("Block count mismatch in chunk file %s (got %d blocks, expected %d)\n",
			filename.c_str(), blockIndex, BLOCKS_PER_CHUNK);
		return false;
	}

	if (offset < fileBuffer.size())
	{
		printf("Warning: Extra data at end of chunk file: %s (%zu extra bytes)\n",
			filename.c_str(), fileBuffer.size() - offset);
	}

	printf("Successfully loaded chunk from file: %s\n", filename.c_str());
	return true;
}

bool ChunkFileIO::SaveChunk(std::string const& saveFolder, std::string const& filename, Chunk const* chunk)
{
	std::vector<uint8_t> byteBuffer;

	//== header ==
	ChunkFileHeader header;
	header.m_fourCC[0] = 'G';
	header.m_fourCC[1] = 'C';
	header.m_fourCC[2] = 'H';
	header.m_fourCC[3] = 'K';
	header.m_version = 1;
	header.m_chunkBitsX = CHUNK_BITS_X;
	header.m_chunkBitsY = CHUNK_BITS_Y;
	header.m_chunkBitsZ = CHUNK_BITS_Z;

	byteBuffer.push_back(header.m_fourCC[0]);
	byteBuffer.push_back(header.m_fourCC[1]);
	byteBuffer.push_back(header.m_fourCC[2]);
	byteBuffer.push_back(header.m_fourCC[3]);
	byteBuffer.push_back(header.m_version);
	byteBuffer.push_back(header.m_chunkBitsX);
	byteBuffer.push_back(header.m_chunkBitsY);
	byteBuffer.push_back(header.m_chunkBitsZ);

	ChunkFileRun currentRun;
	currentRun.blockType = chunk->m_blocks[0].GetTypeIndex();
	currentRun.runLength = 0;

	// Loop over each block in the chunk
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; blockIndex++)
	{
		uint8_t currentBlockType = chunk->m_blocks[blockIndex].GetTypeIndex();

		// Check if run is complete
		bool runComplete = false;

		if (currentBlockType != currentRun.blockType)
		{
			// Current block type is not equal to the block type of the run
			runComplete = true;
		}
		else if (blockIndex == BLOCKS_PER_CHUNK - 1)
		{
			// We are at the end of the chunk
			currentRun.runLength++;  // Increment for the current block
			runComplete = true;
		}
		else if (currentRun.runLength >= 255)
		{
			// The run length is at the max of 255
			runComplete = true;
		}
		else
		{
			// Increment the length of the run
			currentRun.runLength++;
		}

		// When the run is complete, push back its member variables into the byte buffer vector
		if (runComplete)
		{
			byteBuffer.push_back(currentRun.blockType);
			byteBuffer.push_back(currentRun.runLength);

			// Start a new run by setting the type to the current block type and zeroing out the length
			if (blockIndex < BLOCKS_PER_CHUNK - 1)  // Don't start new run if we're at the end
			{
				currentRun.blockType = currentBlockType;
				currentRun.runLength = 1;  // Start with 1 for the current block
			}
		}
	}

	// Generate filename: "Chunk(x,y).chunk"
	//std::string filename = saveFolder + "/Chunk(" + std::to_string(chunk->m_chunkCoords.x) + "," + std::to_string(chunk->m_chunkCoords.y) + ").chunk";

	// Call FileWriteFromBuffer with the byte buffer vector
	return FileWriteFromBuffer(byteBuffer, saveFolder+filename);

	return 0;
}
