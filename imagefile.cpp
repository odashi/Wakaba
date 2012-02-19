// wakaba/imagefile.cpp
#include "imagefile.h"
#include "exception.h"
#include <cstdio>
#include <cctype>
#include <algorithm>

using namespace Wakaba;

// �摜�t�@�C���̓ǂݍ���
Bitmap16 *ImageFile::Load(const std::string &filepath)
{
	// �t�@�C���̐擪����ǂݍ���
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();
	UInt8 signature[8] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
	std::fread(signature, 1, 8, fp);
	std::fclose(fp);

	// �������ꂽ�r�b�g�}�b�v�̊i�[��
	Bitmap16 *bitmap;

	// BMP�`��
	if (signature[0] == 'B' && signature[1] == 'M')
		bitmap = LoadBMP(filepath);
	// PNG�`��
	else if (signature[1] == 'P' && signature[2] == 'N' && signature[3] == 'G')
		bitmap = LoadPNG(filepath);
	// JPEG�`��
	else if (signature[0] == 0xff && signature[1] == 0xd8 && signature[2] == 0xff)
		bitmap = LoadJPEG(filepath);
	// �s��
	else
		throw FormatException();

	return bitmap;
}

// �摜�t�@�C���̏����o��
void ImageFile::Save(const Bitmap16 &bitmap, const std::string &filepath, const std::string &format)
{
	// �����̑召�͖�������
	std::string fmt = format;
	std::transform(fmt.begin(), fmt.end(), fmt.begin(), std::toupper);

	if (fmt == "BMP") SaveBMP(bitmap, filepath);
	else if (fmt == "PNG") SavePNG(bitmap, filepath);
	else if (fmt == "JPEG" || fmt == "JPG") SaveJPEG(bitmap, filepath);
	else throw FormatException();
}
