// wakaba/bitmap.cpp
#include "bitmap.h"
#include "exception.h"
#include <cstring>
#ifdef _MSC_VER
#include <malloc.h>
#else
#include <cstdlib>
#endif

using namespace Wakaba;



/*--------------------------------------------------------------
	class Wakaba::Bitmap8
--------------------------------------------------------------*/

// c-tor
Bitmap8::Bitmap8(UInt32 width, UInt32 height)
	: width_(width), height_(height)
{
	if (width == 0 || height == 0)
		throw ParameterException();

#ifdef _MSC_VER
	mem_ = reinterpret_cast<UInt8 *>(::_aligned_malloc(4*width*height*sizeof(UInt8), 16));
#else
	mem_ = new UInt8[4*width*height];
#endif
}

Bitmap8::~Bitmap8()
{
#ifdef _MSC_VER
	::_aligned_free(mem_);
#else
	delete[] mem_;
#endif
}

// ���ƍ������w�肵�ĐV�K�r�b�g�}�b�v�𐶐�
Bitmap8 *Bitmap8::New(UInt32 width, UInt32 height)
{
	return new Bitmap8(width, height);
}

// �r�b�g�}�b�v�̔j��
void Bitmap8::Dispose()
{
	// nullptr �łȂ���Ή������
	if (this)
		delete this;
}

// �w�肵���F�őS�̂��N���A
void Bitmap8::Clear(const Color8 &color)
{
	UInt32 size = 4*width_*height_;

	for (UInt32 i = 0; i < size; ) {
		mem_[i++] = color.b;
		mem_[i++] = color.g;
		mem_[i++] = color.r;
		mem_[i++] = color.a;
	}
}

// �r�b�g�}�b�v�̃R�s�[
void Bitmap8::CopyFrom(const Bitmap8 &src)
{
	if (src.width_ != width_ || src.height_ != height_)
		throw FormatException();

	std::memcpy(mem_, src.mem_, 4*width_*height_*sizeof(UInt8));
}



/*--------------------------------------------------------------
	class Wakaba::Bitmap16
--------------------------------------------------------------*/

// c-tor
Bitmap16::Bitmap16(UInt32 width, UInt32 height)
	: width_(width), height_(height)
{
	if (width == 0 || height == 0)
		throw ParameterException();

#ifdef _MSC_VER
	mem_ = reinterpret_cast<UInt16 *>(::_aligned_malloc(4*width*height*sizeof(UInt16), 16));
#else
	mem_ = new UInt16[4*width*height];
#endif
}

Bitmap16::~Bitmap16()
{
#ifdef _MSC_VER
	::_aligned_free(mem_);
#else
	delete[] mem_;
#endif
}

// ���ƍ������w�肵�ĐV�K�r�b�g�}�b�v�𐶐�
Bitmap16 *Bitmap16::New(UInt32 width, UInt32 height)
{
	return new Bitmap16(width, height);
}

// �r�b�g�}�b�v�̔j��
void Bitmap16::Dispose()
{
	// nullptr �łȂ���Ή������
	if (this)
		delete this;
}

// �w�肵���F�őS�̂��N���A
void Bitmap16::Clear(const Color16 &color)
{
	UInt32 size = 4*width_*height_;

	for (UInt32 i = 0; i < size; ) {
		mem_[i++] = color.b;
		mem_[i++] = color.g;
		mem_[i++] = color.r;
		mem_[i++] = color.a;
	}
}

// �r�b�g�}�b�v�̃R�s�[
void Bitmap16::CopyFrom(const Bitmap16 &src)
{
	if (src.width_ != width_ || src.height_ != height_)
		throw FormatException();

	std::memcpy(mem_, src.mem_, 4*width_*height_*sizeof(UInt16));
}



/*--------------------------------------------------------------
	class Wakaba::BitmapF
--------------------------------------------------------------*/

// c-tor
BitmapF::BitmapF(UInt32 width, UInt32 height)
	: width_(width), height_(height)
{
	if (width == 0 || height == 0)
		throw ParameterException();

#ifdef _MSC_VER
	mem_ = reinterpret_cast<float *>(::_aligned_malloc(4*width*height*sizeof(float), 16));
#else
	mem_ = new float[4*width*height];
#endif
}

BitmapF::~BitmapF()
{
#ifdef _MSC_VER
	::_aligned_free(mem_);
#else
	delete[] mem_;
#endif
}

// ���ƍ������w�肵�ĐV�K�r�b�g�}�b�v�𐶐�
BitmapF *BitmapF::New(UInt32 width, UInt32 height)
{
	return new BitmapF(width, height);
}

// �r�b�g�}�b�v�̔j��
void BitmapF::Dispose()
{
	// nullptr �łȂ���Ή������
	if (this)
		delete this;
}

// �w�肵���F�őS�̂��N���A
void BitmapF::Clear(const ColorF &color)
{
	UInt32 size = 4*width_*height_;

	for (UInt32 i = 0; i < size; ) {
		mem_[i++] = color.b;
		mem_[i++] = color.g;
		mem_[i++] = color.r;
		mem_[i++] = color.a;
	}
}

// �r�b�g�}�b�v�̃R�s�[
void BitmapF::CopyFrom(const BitmapF &src)
{
	if (src.width_ != width_ || src.height_ != height_)
		throw FormatException();

	std::memcpy(mem_, src.mem_, 4*width_*height_*sizeof(float));
}
