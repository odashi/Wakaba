// wakaba/converter.cpp
#include "converter.h"
#include "exception.h"

using namespace Wakaba;

// Bitmap16 to Bitmap8
void Converter::Convert(Bitmap8 &dest, const Bitmap16 &src)
{
	if (dest.Width() != src.Width() || dest.Height() != src.Height())
		throw ParameterException();

	UInt8 *d = dest.Buffer();
	const UInt16 *s = src.Buffer();
	UInt32 size = 4*dest.Width()*dest.Height();
	
	for (UInt32 i = 0; i < size; ++i)
		d[i] = static_cast<UInt8>(s[i] >> 8);
}

// BitmapF to Bitmap8
void Converter::Convert(Bitmap8 &dest, const BitmapF &src)
{
	if (dest.Width() != src.Width() || dest.Height() != src.Height())
		throw ParameterException();

	UInt8 *d = dest.Buffer();
	const float *s = src.Buffer();
	UInt32 size = 4*dest.Width()*dest.Height();
	
	for (UInt32 i = 0; i < size; ++i)
		d[i] = static_cast<UInt8>(s[i] * 255.0f + 0.5f);
}

// Bitmap8 to Bitmap16
void Converter::Convert(Bitmap16 &dest, const Bitmap8 &src)
{
	if (dest.Width() != src.Width() || dest.Height() != src.Height())
		throw ParameterException();

	UInt16 *d = dest.Buffer();
	const UInt8 *s = src.Buffer();
	UInt32 size = 4*dest.Width()*dest.Height();
	
	// s[i] * 0x0101 = (s[i] << 8) + s[i]
	for (UInt32 i = 0; i < size; ++i)
		d[i] = (static_cast<UInt16>(s[i]) << 8) + s[i];
}

// BitmapF to Bitmap16
void Converter::Convert(Bitmap16 &dest, const BitmapF &src)
{
	if (dest.Width() != src.Width() || dest.Height() != src.Height())
		throw ParameterException();

	UInt16 *d = dest.Buffer();
	const float *s = src.Buffer();
	UInt32 size = 4*dest.Width()*dest.Height();
	
	for (UInt32 i = 0; i < size; ++i)
		d[i] = static_cast<UInt16>(s[i] * 65535.0f + 0.5f);
}

// Bitmap8 to BitmapF
void Converter::Convert(BitmapF &dest, const Bitmap8 &src)
{
	if (dest.Width() != src.Width() || dest.Height() != src.Height())
		throw ParameterException();

	float *d = dest.Buffer();
	const UInt8 *s = src.Buffer();
	UInt32 size = 4*dest.Width()*dest.Height();
	const float k = 1.0f / 255.0f;

	for (UInt32 i = 0; i < size; ++i)
		d[i] = static_cast<float>(s[i]) * k;
}

// Bitmap16 to BitmapF
void Converter::Convert(BitmapF &dest, const Bitmap16 &src)
{
	if (dest.Width() != src.Width() || dest.Height() != src.Height())
		throw ParameterException();

	float *d = dest.Buffer();
	const UInt16 *s = src.Buffer();
	UInt32 size = 4*dest.Width()*dest.Height();
	const float k = 1.0f / 65535.0f;

	for (UInt32 i = 0; i < size; ++i)
		d[i] = static_cast<float>(s[i]) * k;
}
