#include "FileUtils.hpp"
#include <filesystem>
#include "ErrorWarningAssert.hpp"
#include "StringUtils.hpp"

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

bool FileExists(std::string const& filename)
{
	struct stat buffer;
	return (stat(filename.c_str(), &buffer) == 0);
}

int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename)
{
	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, filename.c_str(), "wb"); // create file automatically it not exist
	if (err != 0 || fp == nullptr)
	{
		printf("File opening failed");
		return -1;
	}

	size_t bytesWritten = fwrite(inBuffer.data(), sizeof(uint8_t), inBuffer.size(), fp);
	fclose(fp);

	return (int)bytesWritten;
}

bool FolderExists(std::string const& folderName)
{
	return std::filesystem::is_directory(folderName);
}

bool CreateFolder(std::string const& folderName)
{
	if (!std::filesystem::create_directory(folderName))
	{
		ERROR_AND_DIE(Stringf("Could not create folder: \"%s\"", folderName.c_str()));
		return false;
	}
	return true;
}
