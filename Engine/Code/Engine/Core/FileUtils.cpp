#include "FileUtils.hpp"

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, filename.c_str(), "rb");
	if (err != 0 || fp == nullptr)
	{
		printf("File opening failed");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	outBuffer.resize(fileSize);

	size_t bytesRead = fread(outBuffer.data(), 1, fileSize, fp);
	fclose(fp);

	return (int)bytesRead;
}

int FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> outBuffer;
	if (FileReadToBuffer(outBuffer, filename) == -1)
	{
		printf("File opening failed when read it as binary file");
		return -1;
	}
	outBuffer.push_back('\0');
	const char* cstr = reinterpret_cast<const char*>(outBuffer.data());
	outString = std::string(cstr);
	return (int)outString.size();
}
