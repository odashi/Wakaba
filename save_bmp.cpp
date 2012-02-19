// wakaba/save_bmp.cpp
#include "basetype.h"
#include "imagefile.h"
#include "exception.h"
#include "format_bmp.h"

using namespace Wakaba;

namespace {
	// BMP�`���̏����o�� (��������)
	void SaveBMP_inner(const Bitmap16 &bitmap, const std::string &filepath, const BMP::BitmapInfoHeader &info_header)
	{
		if (info_header.bit_count != 24 && info_header.bit_count != 32)
			throw FormatException();
	
		Int32 w = bitmap.Width();
		Int32 h = bitmap.Height();
	
		// �T���v��������̃o�C�g��
		UInt32 bytes_per_sample = info_header.bit_count >> 3;
	
		// �X�L�������C��������̃o�C�g���i4�o�C�g���E���܂ށj
		UInt32 bytes_per_scanline = bytes_per_sample * w + ((4 - (bytes_per_sample * w & 0x3)) & 0x3);
	
		// BITMAPFILEHEADER�̐ݒ�
		BMP::BitmapFileHeader file_header;
		file_header.signature[0] = 'B';
		file_header.signature[1] = 'M';
		file_header.file_size = sizeof(BMP::BitmapFileHeader) + sizeof(BMP::BitmapInfoHeader) + bytes_per_scanline * h;
		for (Int32 i = 0; i < 4; ++i)
			file_header.reserved[i] = 0;
		file_header.offset = sizeof(BMP::BitmapFileHeader) + sizeof(BMP::BitmapInfoHeader);

		// �t�@�C�����J��
		std::FILE *fp = std::fopen(filepath.c_str(), "wb");
		if (!fp)
			throw IOException();

		// �X�L�������C�����i�[���郁����
		UInt8 *scanline = nullptr;

		try {
			scanline = new UInt8[bytes_per_scanline];

			// �w�b�_�̏�������
			if (std::fwrite(&file_header, 1, sizeof(BMP::BitmapFileHeader), fp) < sizeof(BMP::BitmapFileHeader))
				throw IOException();
			if (std::fwrite(&info_header, 1, sizeof(BMP::BitmapInfoHeader), fp) < sizeof(BMP::BitmapInfoHeader))
				throw IOException();
	
			// 4�o�C�g���E������������
			for (Int32 i = 0; i < 3; ++i)
				scanline[bytes_per_scanline - i - 1] = 0;
	
			// �r�b�g�}�b�v���̏�������
			const RawColor16 *buffer = reinterpret_cast<const RawColor16 *>(bitmap.Buffer()) + w * (h-1);
			Int32 skip = -2 * w;
	
			switch (info_header.bit_count) {
			case 24:
				for (Int32 y = 0; y < h; ++y, buffer += skip) {
					// �X�L�������C���̃f�[�^���i�[
					UInt8 *ptr = scanline;
					for (Int32 x = 0; x < w; ++x, buffer++) {
						*ptr++ = static_cast<UInt8>(buffer->b >> 8);
						*ptr++ = static_cast<UInt8>(buffer->g >> 8);
						*ptr++ = static_cast<UInt8>(buffer->r >> 8);
					}
					// ��������
					if (std::fwrite(scanline, 1, bytes_per_scanline, fp) < bytes_per_scanline)
						throw IOException();
				}
				break;
			case 32:
				for (Int32 y = 0; y < h; ++y, buffer += skip) {
					// �X�L�������C���̃f�[�^���i�[
					UInt8 *ptr = scanline;
					for (Int32 x = 0; x < w; ++x, buffer++) {
						*ptr++ = static_cast<UInt8>(buffer->b >> 8);
						*ptr++ = static_cast<UInt8>(buffer->g >> 8);
						*ptr++ = static_cast<UInt8>(buffer->r >> 8);
						*ptr++ = 0x00;
					}
					// ��������
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

		// �����o���I��
		delete[] scanline;
		std::fclose(fp);
	}
}

// BMP�`���̏����o��
void ImageFile::SaveBMP(const Bitmap16 &bitmap, const std::string &filepath)
{
	// �摜���
	BMP::BitmapInfoHeader info_header;
	info_header.struct_size = sizeof(BMP::BitmapInfoHeader);
	info_header.width = bitmap.Width();
	info_header.height = bitmap.Height();
	info_header.planes = 1;
	info_header.bit_count = 24; // ����ł�24bpp�`���ŕۑ�����.
	info_header.compression = BMP::COMPRESSION_RGB;
	info_header.image_size = 0;
	info_header.x_pixels_per_meter = 0;
	info_header.y_pixels_per_meter = 0;
	info_header.used_color = 0;
	info_header.important_color = 0;

	// �ۑ�
	::SaveBMP_inner(bitmap, filepath, info_header);
}
