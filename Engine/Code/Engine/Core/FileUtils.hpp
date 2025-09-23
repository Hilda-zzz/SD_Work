#pragma once
#include <vector>
#include <string>

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
int FileReadToString(std::string& outString, const std::string& filename);

bool FileExists(std::string const& filename);
int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename);
