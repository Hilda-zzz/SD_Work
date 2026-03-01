#pragma once

class RandomNumberGenerator;

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
	static const    Rgba8 GRAY;
	~Rgba8() {};
	Rgba8();
	explicit Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	void SetFromText(char const* text);
	void GetAsFloats(float* colorAsFloats) const;
	float GetColorDistance(const Rgba8& color1, const Rgba8& color2);
	bool		operator==(Rgba8 const& compare) const;
	bool		operator!=(Rgba8 const& compare) const;

	static Rgba8 GetRandomColorInRange(Rgba8 const& min, Rgba8 const& max, RandomNumberGenerator* rng);
};

Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionOfEnd);
