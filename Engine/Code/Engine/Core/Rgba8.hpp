#pragma once

struct Rgba8
{
public: 
	unsigned char r=255;
	unsigned char g=255;
	unsigned char b=255;
	unsigned char a=255;
	
	~Rgba8() {};
	Rgba8();
	explicit Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

};