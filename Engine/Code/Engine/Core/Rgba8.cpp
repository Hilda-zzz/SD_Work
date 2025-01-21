#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
const Rgba8 Rgba8::WHITE = Rgba8(255, 255, 255);
const Rgba8 Rgba8::BLACK = Rgba8(0, 0, 0);
const Rgba8 Rgba8::RED = Rgba8(255, 0, 0);
const Rgba8 Rgba8::GREEN = Rgba8(0, 255, 0);
const Rgba8 Rgba8::BLUE = Rgba8(0, 0, 255);
const Rgba8 Rgba8::CYAN = Rgba8(0, 255, 255);
const Rgba8 Rgba8::MAGNETA = Rgba8(255, 0, 255);
const Rgba8 Rgba8::YELLOW = Rgba8(255, 255, 0);
const Rgba8 Rgba8::HILDA = Rgba8(152, 161, 242);
const Rgba8 Rgba8::TRANSP_BLACK = Rgba8(0, 0, 0,120);

Rgba8::Rgba8():r(255), g(255), b(255), a(255) {}

Rgba8::Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a )
	: r(r), g(g), b(b), a(a)
{

}

void Rgba8::SetFromText(char const* text)
{
	Strings result = SplitStringOnDelimiterIgnoreSpace(text, ',');
	r = (unsigned char)atoi(result[0].c_str());
	g = (unsigned char)atoi(result[1].c_str());
	b = (unsigned char)atoi(result[2].c_str());
	if (result.size() >= 4)
	{
		a= (unsigned char)atoi(result[3].c_str());
	}
	else
	{
		a = (unsigned char)255;
	}
}

bool Rgba8::operator==(Rgba8 const& compare) const
{
	if (r == compare.r && g == compare.g && b == compare.b && a == compare.a)
	{
		return true;
	}
	return false;
}

Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionOfEnd)
{
	float r = Interpolate(NormalizeByte(start.r), NormalizeByte(end.r), fractionOfEnd);
	float g = Interpolate(NormalizeByte(start.g), NormalizeByte(end.g), fractionOfEnd);
	float b = Interpolate(NormalizeByte(start.b), NormalizeByte(end.b), fractionOfEnd);
	float a = Interpolate(NormalizeByte(start.a), NormalizeByte(end.a), fractionOfEnd);
	return Rgba8((unsigned char)DenormalizeByte(r), (unsigned char)DenormalizeByte(g), 
		(unsigned char)DenormalizeByte(b), (unsigned char)DenormalizeByte(a));
}


