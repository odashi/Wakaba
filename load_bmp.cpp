// wakaba/Load_bmp.cpp
#include "imagefile.h"
#include "exception.h"
#include "format_bmp.h"
#include <cstdio>

using namespace Wakaba;

namespace {
	// 8 ビット値を 16 ビット値に変換
	inline UInt16 cvt8to16(UInt8 n) { return (static_cast<UInt16>(n) << 8) + n; }

	// BMPパレットの読み込み
	void LoadBMPPalette(std::FILE *fp, RawColor16 palette[], UInt32 num, bool is_os2)
	{
		if (is_os2) {
			// OS2 BMP ... RGBTRIPLE
			BMP::RGBTriple rgb;
			for (UInt32 i = 0; i < num; ++i) {
				if (std::fread(&rgb, 1, sizeof(BMP::RGBTriple), fp) < sizeof(BMP::RGBTriple))
					throw FormatException();
				palette[i].b = cvt8to16(rgb.b);
				palette[i].g = cvt8to16(rgb.g);
				palette[i].r = cvt8to16(rgb.r);
				palette[i].a = 0xffff;
			}
		} else {
			// Windows BMP ... RGBQUAD
			BMP::RGBQuad rgb;
			for (UInt32 i = 0; i < num; ++i) {
				if (std::fread(&rgb, 1, sizeof(BMP::RGBQuad), fp) < sizeof(BMP::RGBQuad))
					throw FormatException();
				palette[i].b = cvt8to16(rgb.b);
				palette[i].g = cvt8to16(rgb.g);
				palette[i].r = cvt8to16(rgb.r);
				palette[i].a = 0xffff;
			}
		}
	}

	// BMP画像データの読み込み
	// Palette 1/2/4/8 bits
	void LoadBMPFromPalette(std::FILE *fp, Bitmap16 &bitmap, BMP::BitmapFileHeader &file_header, BMP::BitmapInfoHeader &info_header)
	{
		// パレット情報のバッファ
		RawColor16 *palette = new RawColor16[1 << info_header.bit_count];

		// パレット情報の読み込み
		std::fseek(fp, sizeof(BMP::BitmapFileHeader) + info_header.struct_size, SEEK_SET);
		try {
			LoadBMPPalette(fp, palette, 1 << info_header.bit_count, info_header.struct_size == sizeof(BMP::BitmapCoreHeader));
		} catch (...) {
			delete[] palette;
			throw;
		}

		// 1列分のバイト数
		UInt32 scanline_size = bitmap.Width() * info_header.bit_count;
		scanline_size = (scanline_size >> 3) + ((scanline_size & 0x07) ? 1 : 0);
		scanline_size += (4 - (scanline_size & 0x3)) & 0x3;
	
		// 一時的にビットマップ情報を格納するバッファ
		UInt8 *scanline = new UInt8[scanline_size];

		// ビットマップの格納先
		RawColor16 *buffer = reinterpret_cast<RawColor16 *>(bitmap.Buffer());

		// スキップ量
		Int32 skip = bitmap.Width();
		if (info_header.height > 0) {
			buffer += bitmap.Width() * (bitmap.Height() - 1);
			skip = -skip;
		}

		UInt32 pixels_per_byte = 8 / info_header.bit_count; // 1 byteあたりのピクセル数
		UInt32 shift = 8 - info_header.bit_count; // 1ピクセル読むたびにデータをシフトさせる量
		UInt8 mask = ((1 << info_header.bit_count) - 1) << shift; // 1ピクセルを読み込むためのマスク

		// ビットマップ読み込み
		std::fseek(fp, file_header.offset, SEEK_SET);
		for (UInt32 y = 0; y < bitmap.Height(); ++y) {
			// 1列読み込み
			if (std::fread(scanline, 1, scanline_size, fp) < scanline_size) {
				delete[] palette;
				delete[] scanline;
				throw FormatException();
			}

			UInt8 *s = scanline; // 現在のバイト（scanlineに格納された値を破壊しながら読み進む）
			UInt32 n = 0; // 現在のバイトで何ピクセル読んだか

			// ビットマップが左右反転しているかどうかで処理を分ける
			if (info_header.width > 0) {
				for (UInt32 x = 0; x < bitmap.Width(); ++x) {
					// 現在のバイトをすべて読んだら次へ
					if (n >= pixels_per_byte)
						n = 0, ++s;

					// パレットから色を格納
					buffer[x] = palette[(*s & mask) >> shift];

					// 次のパレット情報へ
					*s <<= info_header.bit_count;
					++n;
				}
			} else {
				for (UInt32 x = bitmap.Width() - 1; x >= 0; x--) {
					// 現在のバイトをすべて読んだら次へ
					if (n >= pixels_per_byte)
						n = 0, ++s;

					// パレットから色を格納
					buffer[x] = palette[(*s & mask) >> shift];

					// 次のパレット情報へ
					*s <<= info_header.bit_count;
					++n;
				}
			}

			// バッファを1列分進める
			buffer += skip;
		}

		delete[] palette;
		delete[] scanline;
	}

	// BMP画像データの読み込み
	// RGB 24/32 bits
	void LoadBMPFromData(std::FILE *fp, Bitmap16 &bitmap, BMP::BitmapFileHeader &file_header, BMP::BitmapInfoHeader &info_header)
	{
		// ビット数からバイト数を計算
		UInt32 byte_count = info_header.bit_count >> 3;

		// 1列分のバイト数
		UInt32 scanline_size = bitmap.Width() * byte_count + ((4 - ((bitmap.Width() * byte_count) & 0x3)) & 0x3);

		// 一時的にビットマップ情報を格納するバッファ
		UInt8 *scanline = new UInt8[scanline_size];

		// ビットマップの格納先
		RawColor16 *buffer = reinterpret_cast<RawColor16 *>(bitmap.Buffer());

		// スキップ量
		Int32 skip = bitmap.Width();
		if (info_header.height > 0) {
			buffer += bitmap.Width() * (bitmap.Height() - 1);
			skip = -skip;
		}

		// ビットマップ読み込み
		std::fseek(fp, file_header.offset, SEEK_SET);
		for (UInt32 y = 0; y < bitmap.Height(); ++y) {
			// 1列読み込み
			if (std::fread(scanline, 1, scanline_size, fp) < scanline_size) {
				delete[] scanline;
				throw FormatException();
			}

			// ビットマップが左右反転しているかどうかで処理を分ける
			if (info_header.width > 0)
				for (UInt32 x = 0, xx = 0; x < bitmap.Width(); ++x, xx += byte_count) {
					buffer[x].b = cvt8to16(scanline[xx]);
					buffer[x].g = cvt8to16(scanline[xx+1]);
					buffer[x].r = cvt8to16(scanline[xx+2]);
					buffer[x].a = 0xffff;
				}
			else
				for (UInt32 x = bitmap.Width()-1, xx = (bitmap.Width()-1) * byte_count; x >= 0; x--, xx -= byte_count) {
					buffer[x].b = cvt8to16(scanline[xx]);
					buffer[x].g = cvt8to16(scanline[xx+1]);
					buffer[x].r = cvt8to16(scanline[xx+2]);
					buffer[x].a = 0xffff;
				}

			// バッファを1列分進める
			buffer += skip;
		}

		delete[] scanline;
	}

	// OS2 BMP形式の読み込み
	Bitmap16 *LoadBMPOs2(std::FILE *fp, BMP::BitmapFileHeader &file_header)
	{
		BMP::BitmapCoreHeader core_header;
		BMP::BitmapInfoHeader info_header;

		// 情報ヘッダの読み込み・確認
		if (std::fread(&core_header, 1, sizeof(BMP::BitmapCoreHeader), fp) < sizeof(BMP::BitmapCoreHeader))
			throw FormatException();
		if (core_header.width == 0 || core_header.height == 0 || core_header.planes != 1)
			throw FormatException();
	
		// BMP::BitmapInfoHeaderに情報をコピー
		info_header.struct_size = core_header.struct_size;
		info_header.width = core_header.width;
		info_header.height = core_header.height;
		info_header.planes = core_header.planes;
		info_header.bit_count = core_header.bit_count;
		info_header.compression = BMP::COMPRESSION_RGB;
		info_header.image_size = 0;
		info_header.x_pixels_per_meter = 0;
		info_header.y_pixels_per_meter = 0;
		info_header.used_color = 0;
		info_header.important_color = 0;
	
		// オブジェクトの生成
		Bitmap16 *bitmap = Bitmap16::New(info_header.width, info_header.height);

		// 各ビットごとの処理
		try {
			switch (info_header.bit_count) {
			case 1:
			case 4:
			case 8:
				LoadBMPFromPalette(fp, *bitmap, file_header, info_header);
				break;
			case 24:
				LoadBMPFromData(fp, *bitmap, file_header, info_header);
				break;
			default: // 2, 16, 32
				throw FormatException();
			}
		} catch (...) {
			bitmap->Dispose();
			throw;
		}
	
		return bitmap;
	}

	// Windows BMP形式の読み込み
	Bitmap16 *LoadBMPWin(std::FILE *fp, BMP::BitmapFileHeader &file_header)
	{
		BMP::BitmapInfoHeader info_header;

		// 情報ヘッダの読み込み・確認
		if (std::fread(&info_header, 1, sizeof(BMP::BitmapInfoHeader), fp) < sizeof(BMP::BitmapInfoHeader))
			throw FormatException();
		if (info_header.width == 0 || info_header.height == 0 || info_header.planes != 1 || info_header.compression != BMP::COMPRESSION_RGB)
			throw FormatException();
	
		// オブジェクトの生成
		Bitmap16 *bitmap = Bitmap16::New(info_header.width, info_header.height);

		// 各ビットごとの処理
		try {
			switch (info_header.bit_count)
			{
			case 1:
			case 2:
			case 4:
			case 8:
				LoadBMPFromPalette(fp, *bitmap, file_header, info_header);
				break;
			case 24:
			case 32:
				LoadBMPFromData(fp, *bitmap, file_header, info_header);
				break;
			default: // 16
				throw FormatException();
			}
		} catch (...) {
			bitmap->Dispose();
			throw;
		}
	
		return bitmap;
	}
}

// BMP形式の読み込み
Bitmap16 *ImageFile::LoadBMP(const std::string &filepath)
{
	BMP::BitmapFileHeader file_header;
	UInt32 info_size;
	Bitmap16 *bitmap;

	// ファイルを開く
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();

	try {
		// ファイルヘッダの読み込み
		if (std::fread(&file_header, 1, sizeof(BMP::BitmapFileHeader), fp) < sizeof(BMP::BitmapFileHeader))
			throw FormatException();

		// ファイルヘッダの確認
		if (file_header.signature[0] != 'B' || file_header.signature[1] != 'M')
			throw FormatException();

		// 情報ヘッダのサイズを取得
		if (std::fread(&info_size, 1, sizeof(UInt32), fp) < sizeof(UInt32))
			throw FormatException();

		std::fseek(fp, -static_cast<Int32>(sizeof(UInt32)), SEEK_CUR);
		
		// 情報ヘッダのサイズごとに処理を振り分ける
		if (info_size == sizeof(BMP::BitmapCoreHeader))
			// OS2 BMP
			bitmap = LoadBMPOs2(fp, file_header);
		else if (info_size == sizeof(BMP::BitmapInfoHeader))
			// Windows BMP
			bitmap = LoadBMPWin(fp, file_header);
		else
			throw FormatException();
	} catch (...) {
		std::fclose(fp);
		throw;
	}
	
	// unknown
	std::fclose(fp);
	return bitmap;
}
