// wakaba/save_jpeg.cpp
#include "basetype.h"
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

	// JPEG形式の書き出し (内部処理)
	void SaveJPEG_inner(const Bitmap16 &bitmap, const std::string &filepath, Int32 quality)
	{
		::jpeg_compress_struct cinfo;
		MyErrorMgr jerr;

		const Int32 pixel_size = 3;
		Int32 scanline_size = pixel_size * bitmap.Width();
		const RawColor16 *color_buffer = reinterpret_cast<const RawColor16 *>(bitmap.Buffer());
	
		// ファイルを開く
		std::FILE *fp = std::fopen(filepath.c_str(), "wb");
		if (!fp)
			throw IOException();
	
		// スキャンライン作成
		::JSAMPLE *scanline;
		try {
			scanline = new ::JSAMPLE[scanline_size];
		} catch (...) {
			std::fclose(fp);
			std::remove(filepath.c_str());
			throw;
		}

		try {
			// setjmp
#pragma warning(push)
#pragma warning(disable:4611)
			if (::setjmp(jerr.setjmp_buffer))
				throw InternalException();
#pragma warning(pop)
	
			// jpegライブラリ初期化
			cinfo.err = ::jpeg_std_error(&jerr.pub);
			jerr.pub.error_exit = MyErrorExit;
			::jpeg_create_compress(&cinfo);
	
			// 出力ファイル設定
			::jpeg_stdio_dest(&cinfo, fp);
	
			// 画像情報設定
			cinfo.image_width = (::JDIMENSION)bitmap.Width();
			cinfo.image_height = (::JDIMENSION)bitmap.Height();
			cinfo.input_components = pixel_size;
			cinfo.in_color_space = ::JCS_RGB;
			::jpeg_set_defaults(&cinfo);
	
			// 精度設定
			::jpeg_set_quality(&cinfo, quality < 0 ? 0 : quality > 100 ? 100 : quality, TRUE);
	
			// 処理の開始
			::jpeg_start_compress(&cinfo, TRUE);
	
			while (cinfo.next_scanline < cinfo.image_height) {
				// スキャンラインの格納
				::JSAMPLE *s = scanline;
				for (::JDIMENSION x = 0; x < cinfo.image_width; ++x, color_buffer++) {
					*s++ = static_cast<::JSAMPLE>(color_buffer->r >> 8);
					*s++ = static_cast<::JSAMPLE>(color_buffer->g >> 8);
					*s++ = static_cast<::JSAMPLE>(color_buffer->b >> 8);
				}
				// 圧縮, 書き出し
				::jpeg_write_scanlines(&cinfo, &scanline, 1);
			}
		} catch (...) {
			::jpeg_destroy_compress(&cinfo);
			delete[] scanline;
			std::fclose(fp);
			std::remove(filepath.c_str());
			throw;
		}
	
		// 書き出し終了
		::jpeg_finish_compress(&cinfo);
		::jpeg_destroy_compress(&cinfo);
		delete[] scanline;
		std::fclose(fp);
	}
}

// libjpegによるJPEG形式の書き出し
void ImageFile::SaveJPEG(const Bitmap16 &bitmap, const std::string &filepath)
{
	::SaveJPEG_inner(bitmap, filepath, 90);
}
