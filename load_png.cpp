// wakaba/load_png.cpp
#include "imagefile.h"
#include "exception.h"
#include <png/png.h>
#include <cstdio>
#include <csetjmp>

using namespace Wakaba;

// libpngによるPNG形式の読み込み
Bitmap16 *ImageFile::LoadPNG(const std::string &filepath)
{
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();

	Bitmap16 *bitmap;
	::png_structp png_ptr = nullptr;
	::png_infop info_ptr = nullptr, end_ptr = nullptr;

	try {
		// シグネチャの確認
		UInt8 signature[8];
		std::fread(signature, 1, 8, fp);
		if (::png_sig_cmp(signature, 0, 8))
			throw FormatException();

		// 初期化
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

		// ファイルポインタの設定
		::png_init_io(png_ptr, fp);
		::png_set_sig_bytes(png_ptr, 8);

		// PNGファイルを読み込む
		Int32 transforms =
			PNG_TRANSFORM_PACKING | // 多重化データ(1,2,4bits)を展開
			PNG_TRANSFORM_BGR | // RGB->BGR, RGBA->BGRA
			PNG_TRANSFORM_SWAP_ENDIAN | // エンディアン変更
			PNG_TRANSFORM_GRAY_TO_RGB | // G->RGB
			PNG_TRANSFORM_EXPAND_16; // 16bit化
		::png_read_png(png_ptr, info_ptr, transforms, nullptr);

		// IHDRチャンクを読み込む
		UInt32 width, height;
		Int32 bit_depth, color_type, interlace_type;
		::png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr);

		// ビットマップ生成
		bitmap = Bitmap16::New(width, height);
		::png_bytepp rows = ::png_get_rows(png_ptr, info_ptr);
		UInt16 *buf = bitmap->Buffer();

		// ピクセルのコピー
		if (color_type & PNG_COLOR_MASK_ALPHA) {
			// アルファを含むピクセル列
			for (UInt32 y = 0; y < height; ++y) {
				UInt16 *src = reinterpret_cast<UInt16 *>(rows[y]);
				UInt32 ww = 4*width;
				for (UInt32 x = 0; x < ww; ++x)
					*buf++ = *src++;
			}
		} else {
			// アルファを含まないピクセル列
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
		// png_ptr -> info_ptr -> end_ptr の順でメモリが確保されている.
		if (end_ptr) ::png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
		if (info_ptr) ::png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		else if (png_ptr) ::png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		std::fclose(fp);
		throw;
	}

	// 読み込み終了
	::png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	std::fclose(fp);
	return bitmap;
}
