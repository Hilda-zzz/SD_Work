#include "ChunkFileIO.hpp"
#include <vector>
#include <Engine/Core/FileUtils.hpp>
#include "Chunk.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

bool ChunkFileIO::LoadChunk(std::string const& saveFolder, std::string const& filename, Chunk* chunk)
{
	std::vector<uint8_t> fileBuffer;
	int bytesRead = FileReadToBuffer(fileBuffer, saveFolder + filename);

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

	if (header.m_chunkBitsX != CHUNK_BITS_X || header.m_chunkBitsY != CHUNK_BITS_Y || header.m_chunkBitsZ != CHUNK_BITS_Z)
	{
		printf("Chunk dimensions mismatch in file %s (got %d,%d,%d, expected %d,%d,%d)\n",
			filename.c_str(), header.m_chunkBitsX, header.m_chunkBitsY, header.m_chunkBitsZ,
			CHUNK_BITS_X, CHUNK_BITS_Y, CHUNK_BITS_Z);
		return false;
	}

	// Decode RLE data
	int blockIndex = 0;

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

	// Validate chunk block count
	if (chunk->m_blocks.size() != BLOCKS_PER_CHUNK)
	{
		printf("Error: Chunk has wrong number of blocks (%zu, expected %d)\n",
			chunk->m_blocks.size(), BLOCKS_PER_CHUNK);
		return false;
	}

	// RLE encoding - Start with first block
	ChunkFileRun currentRun;
	currentRun.blockType = chunk->m_blocks[0].GetTypeIndex();
	currentRun.runLength = 1;

	// Loop over each block in the chunk (starting from second block)
	for (int blockIndex = 1; blockIndex < BLOCKS_PER_CHUNK; blockIndex++)
	{
		uint8_t currentBlockType = chunk->m_blocks[blockIndex].GetTypeIndex();

		// 检查是否需要完成当前 run
		if (currentBlockType != currentRun.blockType || currentRun.runLength == 255)
		{
			// 完成当前 run
			byteBuffer.push_back(currentRun.blockType);
			byteBuffer.push_back(currentRun.runLength);

			// 开始新 run
			currentRun.blockType = currentBlockType;
			currentRun.runLength = 1;
		}
		else
		{
			// 继续当前 run
			currentRun.runLength++;
		}
	}

	// Write final run
	byteBuffer.push_back(currentRun.blockType);
	byteBuffer.push_back(currentRun.runLength);

	// Write to file
	bool success = FileWriteFromBuffer(byteBuffer, saveFolder + filename);

	if (success)
	{
		printf("Successfully saved chunk to file: %s\n", filename.c_str());
	}
	else
	{
		printf("Failed to save chunk to file: %s\n", filename.c_str());
	}

	return success;
}
