// wakaba/save_png.cpp
#include "basetype.h"
#include "imagefile.h"
#include "exception.h"
#include <png/png.h>
#include <cstdio>
#include <csetjmp>

using namespace Wakaba;

namespace {
	// PNG�`���̏����o�� (��������)
	void SavePNG_inner(const Bitmap16 &bitmap, const std::string &filepath)
	{
		// �t�@�C�����J��
		std::FILE *fp = std::fopen(filepath.c_str(), "wb");
		if (!fp)
			throw IOException();

		::png_structp png_ptr = nullptr;
		::png_infop info_ptr = nullptr;

		::png_byte *buffer = nullptr;
		::png_bytep *rows = nullptr;

		try {
			// ������
			png_ptr = ::png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			if (!png_ptr) throw InternalException();
			info_ptr = ::png_create_info_struct(png_ptr);
			if (!info_ptr) throw InternalException();

			// setjmp
#pragma warning(push)
#pragma warning(disable:4611)
			if (setjmp(png_jmpbuf(png_ptr)))
				throw InternalException();
#pragma warning(pop)

			// �t�@�C���|�C���^�̐ݒ�
			::png_init_io(png_ptr, fp);

			// �摜���
			::png_set_IHDR(png_ptr, info_ptr,
				bitmap.Width(),
				bitmap.Height(),
				8, // ����ł�32bpp�`���ŕۑ�����.
				PNG_COLOR_TYPE_RGBA, // �A���t�@���܂�
				PNG_INTERLACE_NONE, // �C���^�[���[�X�Ȃ�
				PNG_COMPRESSION_TYPE_DEFAULT, // Deflate(zlib)���k
				PNG_FILTER_TYPE_DEFAULT); // Adaptation�t�B���^

			UInt32 buffer_size = 4*bitmap.Width()*bitmap.Height();
			
			// �ꎞ�o�b�t�@�Ƃ��̐擪�|�C���^
			buffer = new ::png_byte[buffer_size];
			rows = new ::png_bytep[bitmap.Height()];
			for (UInt32 y = 0; y < bitmap.Height(); ++y)
				rows[y] = buffer + 4*bitmap.Width()*y;
			::png_set_rows(png_ptr, info_ptr, rows);

			// �f�[�^�i�[
			const UInt16 *src = bitmap.Buffer();
			for (UInt32 i = 0; i < buffer_size; ++i)
				buffer[i] = static_cast<UInt8>(src[i] >> 8);

			// ���k, �ۑ�
			::png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, nullptr);
		} catch (...) {
			if (rows) delete[] rows;
			if (buffer) delete[] buffer;
			// png_ptr -> info_ptr �̏��Ń��������m�ۂ���Ă���.
			if (info_ptr) ::png_destroy_write_struct(&png_ptr, &info_ptr);
			else if (png_ptr) ::png_destroy_write_struct(&png_ptr, nullptr);
			std::fclose(fp);
			std::remove(filepath.c_str());
			throw;
		}

		// �����o���I��
		delete[] rows;
		delete[] buffer;
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		std::fclose(fp);
	}
}

// libpng��p����PNG�`���̏����o��
void ImageFile::SavePNG(const Bitmap16 &bitmap, const std::string &filepath)
{
	::SavePNG_inner(bitmap, filepath);
}
