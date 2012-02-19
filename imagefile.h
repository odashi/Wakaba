// wakaba/imagefile.h
#ifndef WAKABA_IMAGEFILE_H
#define WAKABA_IMAGEFILE_H

#include "dllexport.h"
#include "bitmap.h"
#include <string>

namespace Wakaba {
	/// 画像ファイルの入出力処理
	class WAKABA_API ImageFile {
		// フォーマットごとの読み込み関数
		static Bitmap16 *LoadBMP(const std::string &filepath);
		static Bitmap16 *LoadPNG(const std::string &filepath);
		static Bitmap16 *LoadJPEG(const std::string &filepath);

		// フォーマットごとの書き出し関数
		static void SaveBMP(const Bitmap16 &bitmap, const std::string &filepath);
		static void SavePNG(const Bitmap16 &bitmap, const std::string &filepath);
		static void SaveJPEG(const Bitmap16 &bitmap, const std::string &filepath);
	public:
		/// ファイルから画像を読み込む
		/**
		 * 画像ファイルからビットマップを生成します.
		 * 
		 * ファイルフォーマットは自動的に判別されます.
		 * 
		 * @param filepath 読み込む対象のファイルパス.
		 * @retval Bitmap16* 画像情報を格納した Bitmap16.
		 * @exception IOException ファイル操作に失敗しました.
		 * @exception FormatException 非対応のファイルフォーマットか, データが不正です.
		 * @exception InternalException 内部処理でエラーが発生しました.
		 * @remarks 現在この関数が読み込めるのは以下のファイルフォーマットです.
		 * - BMP(Windows/OS2, 16bpp以外の非圧縮形式)
		 * - PNG
		 * - JPEG(JFIF/Exif)
		 */
		static Bitmap16 *Load(const std::string &filepath);

		/// ファイルへ画像を書き出す
		/**
		 * ビットマップを指定したフォーマットの画像ファイルへ書き出します.
		 * 
		 * @param bitmap 画像情報.
		 * @param filepath 書き出す対象のファイルパス.
		 * @param format ファイルフォーマット (remarks 参照).
		 * @exception IOException ファイル操作に失敗しました.
		 * @exception FormatException 非対応のファイルフォーマットが指定されました.
		 * @exception InternalException 内部処理でエラーが発生しました.
		 * @remarks 現在この関数が対応しているファイルフォーマット, およびフォーマット指定のための format の値は以下の通りです.
		 * - BMP(Windows, 24bpp) …… "BMP"
		 * - PNG(RGBA 32bpp) …… "PNG"
		 * - JPEG(JFIF) …… "JPEG" "JPG"
		 * @remarks format の値は大文字, 小文字を問いません.
		 */
		static void Save(const Bitmap16 &bitmap, const std::string &filepath, const std::string &format);
	};
}

#endif
