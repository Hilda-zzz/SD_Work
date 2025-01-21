#pragma once

struct Rgba8
{
public: 
	unsigned char r=255;
	unsigned char g=255;
	unsigned char b=255;
	unsigned char a=255;

	static const	Rgba8 BLACK;
	static const	Rgba8 WHITE;
	static const	Rgba8 RED;
	static const	Rgba8 GREEN;
	static const	Rgba8 BLUE;
	static const	Rgba8 CYAN;
	static const	Rgba8 YELLOW;
	static const	Rgba8 MAGNETA;
	static const	Rgba8 HILDA;
	static const	Rgba8 TRANSP_BLACK;
	~Rgba8() {};
	Rgba8();
	explicit Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	void SetFromText(char const* text);

	bool		operator==(Rgba8 const& compare) const;
};

Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionOfEnd);
