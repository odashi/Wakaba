// wakaba/save_bmp.cpp
#include "basetype.h"
#include "imagefile.h"
#include "exception.h"
#include "format_bmp.h"

using namespace Wakaba;

namespace {
	// BMP形式の書き出し (内部処理)
	void SaveBMP_inner(const Bitmap16 &bitmap, const std::string &filepath, const BMP::BitmapInfoHeader &info_header)
	{
		if (info_header.bit_count != 24 && info_header.bit_count != 32)
			throw FormatException();
	
		Int32 w = bitmap.Width();
		Int32 h = bitmap.Height();
	
		// サンプルあたりのバイト数
		UInt32 bytes_per_sample = info_header.bit_count >> 3;
	
		// スキャンラインあたりのバイト数（4バイト境界を含む）
		UInt32 bytes_per_scanline = bytes_per_sample * w + ((4 - (bytes_per_sample * w & 0x3)) & 0x3);
	
		// BITMAPFILEHEADERの設定
		BMP::BitmapFileHeader file_header;
		file_header.signature[0] = 'B';
		file_header.signature[1] = 'M';
		file_header.file_size = sizeof(BMP::BitmapFileHeader) + sizeof(BMP::BitmapInfoHeader) + bytes_per_scanline * h;
		for (Int32 i = 0; i < 4; ++i)
			file_header.reserved[i] = 0;
		file_header.offset = sizeof(BMP::BitmapFileHeader) + sizeof(BMP::BitmapInfoHeader);

		// ファイルを開く
		std::FILE *fp = std::fopen(filepath.c_str(), "wb");
		if (!fp)
			throw IOException();

		// スキャンラインを格納するメモリ
		UInt8 *scanline = nullptr;

		try {
			scanline = new UInt8[bytes_per_scanline];

			// ヘッダの書き込み
			if (std::fwrite(&file_header, 1, sizeof(BMP::BitmapFileHeader), fp) < sizeof(BMP::BitmapFileHeader))
				throw IOException();
			if (std::fwrite(&info_header, 1, sizeof(BMP::BitmapInfoHeader), fp) < sizeof(BMP::BitmapInfoHeader))
				throw IOException();
	
			// 4バイト境界部分を初期化
			for (Int32 i = 0; i < 3; ++i)
				scanline[bytes_per_scanline - i - 1] = 0;
	
			// ビットマップ情報の書き込み
			const RawColor16 *buffer = reinterpret_cast<const RawColor16 *>(bitmap.Buffer()) + w * (h-1);
			Int32 skip = -2 * w;
	
			switch (info_header.bit_count) {
			case 24:
				for (Int32 y = 0; y < h; ++y, buffer += skip) {
					// スキャンラインのデータを格納
					UInt8 *ptr = scanline;
					for (Int32 x = 0; x < w; ++x, buffer++) {
						*ptr++ = static_cast<UInt8>(buffer->b >> 8);
						*ptr++ = static_cast<UInt8>(buffer->g >> 8);
						*ptr++ = static_cast<UInt8>(buffer->r >> 8);
					}
					// 書き込み
					if (std::fwrite(scanline, 1, bytes_per_scanline, fp) < bytes_per_scanline)
						throw IOException();
				}
				break;
			case 32:
				for (Int32 y = 0; y < h; ++y, buffer += skip) {
					// スキャンラインのデータを格納
					UInt8 *ptr = scanline;
					for (Int32 x = 0; x < w; ++x, buffer++) {
						*ptr++ = static_cast<UInt8>(buffer->b >> 8);
						*ptr++ = static_cast<UInt8>(buffer->g >> 8);
						*ptr++ = static_cast<UInt8>(buffer->r >> 8);
						*ptr++ = 0x00;
					}
					// 書き込み
					if (std::fwrite(scanline, 1, bytes_per_scanline, fp) < bytes_per_scanline)
						throw IOException();
				}
				break;
			}
		} catch (...) {
			if (scanline)
				delete[] scanline;
			std::fclose(fp);
			std::remove(filepath.c_str());
			throw;
		}

		// 書き出し終了
		delete[] scanline;
		std::fclose(fp);
	}
}

// BMP形式の書き出し
void ImageFile::SaveBMP(const Bitmap16 &bitmap, const std::string &filepath)
{
	// 画像情報
	BMP::BitmapInfoHeader info_header;
	info_header.struct_size = sizeof(BMP::BitmapInfoHeader);
	info_header.width = bitmap.Width();
	info_header.height = bitmap.Height();
	info_header.planes = 1;
	info_header.bit_count = 24; // 現状では24bpp形式で保存する.
	info_header.compression = BMP::COMPRESSION_RGB;
	info_header.image_size = 0;
	info_header.x_pixels_per_meter = 0;
	info_header.y_pixels_per_meter = 0;
	info_header.used_color = 0;
	info_header.important_color = 0;

	// 保存
	::SaveBMP_inner(bitmap, filepath, info_header);
}
