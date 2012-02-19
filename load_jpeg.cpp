// wakaba/load_jpeg.cpp
#include "imagefile.h"
#include "exception.h"
#include <jpeg/jpeglib.h>
#include <cstdio>
#include <csetjmp>

using namespace Wakaba;

namespace {
	// 独自定義ののエラーマネージャ
	struct MyErrorMgr {
		jpeg_error_mgr pub;
		std::jmp_buf setjmp_buffer;
	};

	// 独自定義のエラーハンドラ
	void MyErrorExit(j_common_ptr cinfo)
	{
		// 何もしないでリターンアドレスまで戻る
		std::longjmp(((MyErrorMgr *)cinfo->err)->setjmp_buffer, 1);
	}
}

// libjpegによるJPEG形式の読み込み
Bitmap16 *ImageFile::LoadJPEG(const std::string &filepath)
{
	::jpeg_decompress_struct cinfo;
	::MyErrorMgr jerr;
	::JSAMPLE **scanline;
	
	Bitmap16 *bitmap;
	
	// ファイルを開く
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();
	
	try {
		// setjmp
#pragma warning(push)
#pragma warning(disable:4611)
		if (::setjmp(jerr.setjmp_buffer))
			throw FormatException();
#pragma warning(pop)

		// jpegライブラリ初期化
		cinfo.err = ::jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = ::MyErrorExit;
		::jpeg_create_decompress(&cinfo);

		// ファイル設定
		::jpeg_stdio_src(&cinfo, fp);
	
		// ヘッダ読み込み
		::jpeg_read_header(&cinfo, TRUE);
	
		// ビットマップ生成
		bitmap = Bitmap16::New(cinfo.image_width, cinfo.image_height);
			
		// 処理の開始
		Int32 scanline_size = cinfo.num_components * cinfo.image_width;
		RawColor16 *buf = reinterpret_cast<RawColor16 *>(bitmap->Buffer());
		::jpeg_start_decompress(&cinfo);
		scanline = (*cinfo.mem->alloc_sarray)((::j_common_ptr)&cinfo, JPOOL_IMAGE, scanline_size, 1);

		switch (cinfo.out_color_components) {
		case 1: // Gray-scale JPEG
			while (cinfo.output_scanline < cinfo.output_height) {
				// 伸長
				::jpeg_read_scanlines(&cinfo, scanline, 1);
				
				// スキャンラインの読み込み
				for (::JDIMENSION x = 0; x < cinfo.output_width; ++x, ++buf) {
					register UInt16 k = scanline[0][x];
					buf->b = buf->g = buf->r = (k << 8) + k;
					buf->a = 0xffff;
				}
			}
			break;
		case 3: // RGB JPEG
			while (cinfo.output_scanline < cinfo.output_height) {
				// 伸長
				::jpeg_read_scanlines(&cinfo, scanline, 1);
				
				// スキャンラインの読み込み
				::JSAMPLE *s = scanline[0];
				for (::JDIMENSION x = 0; x < cinfo.output_width; ++x, ++buf) {
					buf->r = (*s << 8) + *s, ++s;
					buf->g = (*s << 8) + *s, ++s;
					buf->b = (*s << 8) + *s, ++s;
					buf->a = 0xffff;
				}
			}
			break;
		default: // 未知のJPEG
			throw FormatException();	
		}
	} catch (...) {
		::jpeg_destroy_decompress(&cinfo);
		if (bitmap)
			bitmap->Dispose();
		std::fclose(fp);
		throw;
	}
		
	// 読み込み終了
	::jpeg_finish_decompress(&cinfo);
	::jpeg_destroy_decompress(&cinfo);
	std::fclose(fp);
	return bitmap;
}
