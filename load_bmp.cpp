// wakaba/Load_bmp.cpp
#include "imagefile.h"
#include "exception.h"
#include "format_bmp.h"
#include <cstdio>

using namespace Wakaba;

namespace {
	// 8 �r�b�g�l�� 16 �r�b�g�l�ɕϊ�
	inline UInt16 cvt8to16(UInt8 n) { return (static_cast<UInt16>(n) << 8) + n; }

	// BMP�p���b�g�̓ǂݍ���
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

	// BMP�摜�f�[�^�̓ǂݍ���
	// Palette 1/2/4/8 bits
	void LoadBMPFromPalette(std::FILE *fp, Bitmap16 &bitmap, BMP::BitmapFileHeader &file_header, BMP::BitmapInfoHeader &info_header)
	{
		// �p���b�g���̃o�b�t�@
		RawColor16 *palette = new RawColor16[1 << info_header.bit_count];

		// �p���b�g���̓ǂݍ���
		std::fseek(fp, sizeof(BMP::BitmapFileHeader) + info_header.struct_size, SEEK_SET);
		try {
			LoadBMPPalette(fp, palette, 1 << info_header.bit_count, info_header.struct_size == sizeof(BMP::BitmapCoreHeader));
		} catch (...) {
			delete[] palette;
			throw;
		}

		// 1�񕪂̃o�C�g��
		UInt32 scanline_size = bitmap.Width() * info_header.bit_count;
		scanline_size = (scanline_size >> 3) + ((scanline_size & 0x07) ? 1 : 0);
		scanline_size += (4 - (scanline_size & 0x3)) & 0x3;
	
		// �ꎞ�I�Ƀr�b�g�}�b�v�����i�[����o�b�t�@
		UInt8 *scanline = new UInt8[scanline_size];

		// �r�b�g�}�b�v�̊i�[��
		RawColor16 *buffer = reinterpret_cast<RawColor16 *>(bitmap.Buffer());

		// �X�L�b�v��
		Int32 skip = bitmap.Width();
		if (info_header.height > 0) {
			buffer += bitmap.Width() * (bitmap.Height() - 1);
			skip = -skip;
		}

		UInt32 pixels_per_byte = 8 / info_header.bit_count; // 1 byte������̃s�N�Z����
		UInt32 shift = 8 - info_header.bit_count; // 1�s�N�Z���ǂނ��тɃf�[�^���V�t�g�������
		UInt8 mask = ((1 << info_header.bit_count) - 1) << shift; // 1�s�N�Z����ǂݍ��ނ��߂̃}�X�N

		// �r�b�g�}�b�v�ǂݍ���
		std::fseek(fp, file_header.offset, SEEK_SET);
		for (UInt32 y = 0; y < bitmap.Height(); ++y) {
			// 1��ǂݍ���
			if (std::fread(scanline, 1, scanline_size, fp) < scanline_size) {
				delete[] palette;
				delete[] scanline;
				throw FormatException();
			}

			UInt8 *s = scanline; // ���݂̃o�C�g�iscanline�Ɋi�[���ꂽ�l��j�󂵂Ȃ���ǂݐi�ށj
			UInt32 n = 0; // ���݂̃o�C�g�ŉ��s�N�Z���ǂ񂾂�

			// �r�b�g�}�b�v�����E���]���Ă��邩�ǂ����ŏ����𕪂���
			if (info_header.width > 0) {
				for (UInt32 x = 0; x < bitmap.Width(); ++x) {
					// ���݂̃o�C�g�����ׂēǂ񂾂玟��
					if (n >= pixels_per_byte)
						n = 0, ++s;

					// �p���b�g����F���i�[
					buffer[x] = palette[(*s & mask) >> shift];

					// ���̃p���b�g����
					*s <<= info_header.bit_count;
					++n;
				}
			} else {
				for (UInt32 x = bitmap.Width() - 1; x >= 0; x--) {
					// ���݂̃o�C�g�����ׂēǂ񂾂玟��
					if (n >= pixels_per_byte)
						n = 0, ++s;

					// �p���b�g����F���i�[
					buffer[x] = palette[(*s & mask) >> shift];

					// ���̃p���b�g����
					*s <<= info_header.bit_count;
					++n;
				}
			}

			// �o�b�t�@��1�񕪐i�߂�
			buffer += skip;
		}

		delete[] palette;
		delete[] scanline;
	}

	// BMP�摜�f�[�^�̓ǂݍ���
	// RGB 24/32 bits
	void LoadBMPFromData(std::FILE *fp, Bitmap16 &bitmap, BMP::BitmapFileHeader &file_header, BMP::BitmapInfoHeader &info_header)
	{
		// �r�b�g������o�C�g�����v�Z
		UInt32 byte_count = info_header.bit_count >> 3;

		// 1�񕪂̃o�C�g��
		UInt32 scanline_size = bitmap.Width() * byte_count + ((4 - ((bitmap.Width() * byte_count) & 0x3)) & 0x3);

		// �ꎞ�I�Ƀr�b�g�}�b�v�����i�[����o�b�t�@
		UInt8 *scanline = new UInt8[scanline_size];

		// �r�b�g�}�b�v�̊i�[��
		RawColor16 *buffer = reinterpret_cast<RawColor16 *>(bitmap.Buffer());

		// �X�L�b�v��
		Int32 skip = bitmap.Width();
		if (info_header.height > 0) {
			buffer += bitmap.Width() * (bitmap.Height() - 1);
			skip = -skip;
		}

		// �r�b�g�}�b�v�ǂݍ���
		std::fseek(fp, file_header.offset, SEEK_SET);
		for (UInt32 y = 0; y < bitmap.Height(); ++y) {
			// 1��ǂݍ���
			if (std::fread(scanline, 1, scanline_size, fp) < scanline_size) {
				delete[] scanline;
				throw FormatException();
			}

			// �r�b�g�}�b�v�����E���]���Ă��邩�ǂ����ŏ����𕪂���
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

			// �o�b�t�@��1�񕪐i�߂�
			buffer += skip;
		}

		delete[] scanline;
	}

	// OS2 BMP�`���̓ǂݍ���
	Bitmap16 *LoadBMPOs2(std::FILE *fp, BMP::BitmapFileHeader &file_header)
	{
		BMP::BitmapCoreHeader core_header;
		BMP::BitmapInfoHeader info_header;

		// ���w�b�_�̓ǂݍ��݁E�m�F
		if (std::fread(&core_header, 1, sizeof(BMP::BitmapCoreHeader), fp) < sizeof(BMP::BitmapCoreHeader))
			throw FormatException();
		if (core_header.width == 0 || core_header.height == 0 || core_header.planes != 1)
			throw FormatException();
	
		// BMP::BitmapInfoHeader�ɏ����R�s�[
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
	
		// �I�u�W�F�N�g�̐���
		Bitmap16 *bitmap = Bitmap16::New(info_header.width, info_header.height);

		// �e�r�b�g���Ƃ̏���
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

	// Windows BMP�`���̓ǂݍ���
	Bitmap16 *LoadBMPWin(std::FILE *fp, BMP::BitmapFileHeader &file_header)
	{
		BMP::BitmapInfoHeader info_header;

		// ���w�b�_�̓ǂݍ��݁E�m�F
		if (std::fread(&info_header, 1, sizeof(BMP::BitmapInfoHeader), fp) < sizeof(BMP::BitmapInfoHeader))
			throw FormatException();
		if (info_header.width == 0 || info_header.height == 0 || info_header.planes != 1 || info_header.compression != BMP::COMPRESSION_RGB)
			throw FormatException();
	
		// �I�u�W�F�N�g�̐���
		Bitmap16 *bitmap = Bitmap16::New(info_header.width, info_header.height);

		// �e�r�b�g���Ƃ̏���
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

// BMP�`���̓ǂݍ���
Bitmap16 *ImageFile::LoadBMP(const std::string &filepath)
{
	BMP::BitmapFileHeader file_header;
	UInt32 info_size;
	Bitmap16 *bitmap;

	// �t�@�C�����J��
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();

	try {
		// �t�@�C���w�b�_�̓ǂݍ���
		if (std::fread(&file_header, 1, sizeof(BMP::BitmapFileHeader), fp) < sizeof(BMP::BitmapFileHeader))
			throw FormatException();

		// �t�@�C���w�b�_�̊m�F
		if (file_header.signature[0] != 'B' || file_header.signature[1] != 'M')
			throw FormatException();

		// ���w�b�_�̃T�C�Y���擾
		if (std::fread(&info_size, 1, sizeof(UInt32), fp) < sizeof(UInt32))
			throw FormatException();

		std::fseek(fp, -static_cast<Int32>(sizeof(UInt32)), SEEK_CUR);
		
		// ���w�b�_�̃T�C�Y���Ƃɏ�����U�蕪����
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
