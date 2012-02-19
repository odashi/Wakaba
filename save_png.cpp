// wakaba/save_png.cpp
#include "basetype.h"
#include "imagefile.h"
#include "exception.h"
#include <png/png.h>
#include <cstdio>
#include <csetjmp>

using namespace Wakaba;

namespace {
	// PNG形式の書き出し (内部処理)
	void SavePNG_inner(const Bitmap16 &bitmap, const std::string &filepath)
	{
		// ファイルを開く
		std::FILE *fp = std::fopen(filepath.c_str(), "wb");
		if (!fp)
			throw IOException();

		::png_structp png_ptr = nullptr;
		::png_infop info_ptr = nullptr;

		::png_byte *buffer = nullptr;
		::png_bytep *rows = nullptr;

		try {
			// 初期化
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

			// ファイルポインタの設定
			::png_init_io(png_ptr, fp);

			// 画像情報
			::png_set_IHDR(png_ptr, info_ptr,
				bitmap.Width(),
				bitmap.Height(),
				8, // 現状では32bpp形式で保存する.
				PNG_COLOR_TYPE_RGBA, // アルファを含む
				PNG_INTERLACE_NONE, // インターレースなし
				PNG_COMPRESSION_TYPE_DEFAULT, // Deflate(zlib)圧縮
				PNG_FILTER_TYPE_DEFAULT); // Adaptationフィルタ

			UInt32 buffer_size = 4*bitmap.Width()*bitmap.Height();
			
			// 一時バッファとその先頭ポインタ
			buffer = new ::png_byte[buffer_size];
			rows = new ::png_bytep[bitmap.Height()];
			for (UInt32 y = 0; y < bitmap.Height(); ++y)
				rows[y] = buffer + 4*bitmap.Width()*y;
			::png_set_rows(png_ptr, info_ptr, rows);

			// データ格納
			const UInt16 *src = bitmap.Buffer();
			for (UInt32 i = 0; i < buffer_size; ++i)
				buffer[i] = static_cast<UInt8>(src[i] >> 8);

			// 圧縮, 保存
			::png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, nullptr);
		} catch (...) {
			if (rows) delete[] rows;
			if (buffer) delete[] buffer;
			// png_ptr -> info_ptr の順でメモリが確保されている.
			if (info_ptr) ::png_destroy_write_struct(&png_ptr, &info_ptr);
			else if (png_ptr) ::png_destroy_write_struct(&png_ptr, nullptr);
			std::fclose(fp);
			std::remove(filepath.c_str());
			throw;
		}

		// 書き出し終了
		delete[] rows;
		delete[] buffer;
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		std::fclose(fp);
	}
}

// libpngを用いたPNG形式の書き出し
void ImageFile::SavePNG(const Bitmap16 &bitmap, const std::string &filepath)
{
	::SavePNG_inner(bitmap, filepath);
}
