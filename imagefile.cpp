// wakaba/imagefile.cpp
#include "imagefile.h"
#include "exception.h"
#include <cstdio>
#include <cctype>
#include <algorithm>

using namespace Wakaba;

// 画像ファイルの読み込み
Bitmap16 *ImageFile::Load(const std::string &filepath)
{
	// ファイルの先頭情報を読み込む
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();
	UInt8 signature[8] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
	std::fread(signature, 1, 8, fp);
	std::fclose(fp);

	// 生成されたビットマップの格納先
	Bitmap16 *bitmap;

	// BMP形式
	if (signature[0] == 'B' && signature[1] == 'M')
		bitmap = LoadBMP(filepath);
	// PNG形式
	else if (signature[1] == 'P' && signature[2] == 'N' && signature[3] == 'G')
		bitmap = LoadPNG(filepath);
	// JPEG形式
	else if (signature[0] == 0xff && signature[1] == 0xd8 && signature[2] == 0xff)
		bitmap = LoadJPEG(filepath);
	// 不明
	else
		throw FormatException();

	return bitmap;
}

// 画像ファイルの書き出し
void ImageFile::Save(const Bitmap16 &bitmap, const std::string &filepath, const std::string &format)
{
	// 文字の大小は無視する
	std::string fmt = format;
	std::transform(fmt.begin(), fmt.end(), fmt.begin(), std::toupper);

	if (fmt == "BMP") SaveBMP(bitmap, filepath);
	else if (fmt == "PNG") SavePNG(bitmap, filepath);
	else if (fmt == "JPEG" || fmt == "JPG") SaveJPEG(bitmap, filepath);
	else throw FormatException();
}
