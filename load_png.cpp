// wakaba/load_png.cpp
#include "imagefile.h"
#include "exception.h"
#include <png/png.h>
#include <cstdio>
#include <csetjmp>

using namespace Wakaba;

// libpng�ɂ��PNG�`���̓ǂݍ���
Bitmap16 *ImageFile::LoadPNG(const std::string &filepath)
{
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();

	Bitmap16 *bitmap;
	::png_structp png_ptr = nullptr;
	::png_infop info_ptr = nullptr, end_ptr = nullptr;

	try {
		// �V�O�l�`���̊m�F
		UInt8 signature[8];
		std::fread(signature, 1, 8, fp);
		if (::png_sig_cmp(signature, 0, 8))
			throw FormatException();

		// ������
		png_ptr = ::png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!png_ptr) throw InternalException();
		info_ptr = ::png_create_info_struct(png_ptr);
		if (!info_ptr) throw InternalException();
		end_ptr = ::png_create_info_struct(png_ptr);
		if (!end_ptr) throw InternalException();

		// setjmp
#pragma warning(push)
#pragma warning(disable:4611)
		if (setjmp(png_jmpbuf(png_ptr)))
			throw FormatException();
#pragma warning(pop)

		// �t�@�C���|�C���^�̐ݒ�
		::png_init_io(png_ptr, fp);
		::png_set_sig_bytes(png_ptr, 8);

		// PNG�t�@�C����ǂݍ���
		Int32 transforms =
			PNG_TRANSFORM_PACKING | // ���d���f�[�^(1,2,4bits)��W�J
			PNG_TRANSFORM_BGR | // RGB->BGR, RGBA->BGRA
			PNG_TRANSFORM_SWAP_ENDIAN | // �G���f�B�A���ύX
			PNG_TRANSFORM_GRAY_TO_RGB | // G->RGB
			PNG_TRANSFORM_EXPAND_16; // 16bit��
		::png_read_png(png_ptr, info_ptr, transforms, nullptr);

		// IHDR�`�����N��ǂݍ���
		UInt32 width, height;
		Int32 bit_depth, color_type, interlace_type;
		::png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr);

		// �r�b�g�}�b�v����
		bitmap = Bitmap16::New(width, height);
		::png_bytepp rows = ::png_get_rows(png_ptr, info_ptr);
		UInt16 *buf = bitmap->Buffer();

		// �s�N�Z���̃R�s�[
		if (color_type & PNG_COLOR_MASK_ALPHA) {
			// �A���t�@���܂ރs�N�Z����
			for (UInt32 y = 0; y < height; ++y) {
				UInt16 *src = reinterpret_cast<UInt16 *>(rows[y]);
				UInt32 ww = 4*width;
				for (UInt32 x = 0; x < ww; ++x)
					*buf++ = *src++;
			}
		} else {
			// �A���t�@���܂܂Ȃ��s�N�Z����
			for (UInt32 y = 0; y < height; ++y) {
				UInt16 *src = reinterpret_cast<UInt16 *>(rows[y]);
				for (UInt32 x = 0; x < width; ++x) {
					*buf++ = *src++;
					*buf++ = *src++;
					*buf++ = *src++;
					*buf++ = 0xffff;
				}
			}
		}
	} catch (...) {
		// png_ptr -> info_ptr -> end_ptr �̏��Ń��������m�ۂ���Ă���.
		if (end_ptr) ::png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
		if (info_ptr) ::png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		else if (png_ptr) ::png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		std::fclose(fp);
		throw;
	}

	// �ǂݍ��ݏI��
	::png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	std::fclose(fp);
	return bitmap;
}
