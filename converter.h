// wakaba/converter.h
#ifndef WAKABA_CONVERTER_H
#define WAKABA_CONVERTER_H

#include "dllexport.h"
#include "bitmap.h"

namespace Wakaba {
	/// ビットマップの型変換
	class WAKABA_API Converter {
	public:
		/// Bitmap16 から Bitmap8 へ変換
		/**
		 * 以下の計算式を用いて, Bitmap16 から Bitmap8 への変換処理を行います.
		 * 
		 * col8 = col16 >> 8
		 * @param dest 変換後のデータを格納する Bitmap8.
		 * @param src 変換対象のデータを格納する Bitmap16.
		 * @exception ParameterException dest と src の画像サイズが異なります.
		 * @remarks dest と src は幅と高さがそれぞれ一致している必要があります.
		 */
		static void Convert(Bitmap8 &dest, const Bitmap16 &src);

		/// BitmapF から Bitmap8 へ変換
		/**
		 * 以下の計算式を用いて, BitmapF から Bitmap8 への変換処理を行います.
		 * 
		 * col8 = floor(colF * 255 + 0.5)
		 * @param dest 変換後のデータを格納する Bitmap8.
		 * @param src 変換対象のデータを格納する BitmapF.
		 * @exception ParameterException dest と src の画像サイズが異なります.
		 * @remarks dest と src は幅と高さがそれぞれ一致している必要があります.
		 */
		static void Convert(Bitmap8 &dest, const BitmapF &src);

		/// Bitmap8 から Bitmap16 へ変換
		/**
		 * 以下の計算式を用いて, Bitmap8 から Bitmap16 への変換処理を行います.
		 * 
		 * col16 = col8 * 0x0101
		 * @param dest 変換後のデータを格納する Bitmap16.
		 * @param src 変換対象のデータを格納する Bitmap8.
		 * @exception ParameterException dest と src の画像サイズが異なります.
		 * @remarks dest と src は幅と高さがそれぞれ一致している必要があります.
		 */
		static void Convert(Bitmap16 &dest, const Bitmap8 &src);

		/// BitmapF から Bitmap16 へ変換
		/**
		 * 以下の計算式を用いて, BitmapF から Bitmap16 への変換処理を行います.
		 * 
		 * col16 = floor(colF * 65535 + 0.5)
		 * @param dest 変換後のデータを格納する Bitmap16.
		 * @param src 変換対象のデータを格納する BitmapF.
		 * @exception ParameterException dest と src の画像サイズが異なります.
		 * @remarks dest と src は幅と高さがそれぞれ一致している必要があります.
		 */
		static void Convert(Bitmap16 &dest, const BitmapF &src);

		/// Bitmap8 から BitmapF へ変換
		/**
		 * 以下の計算式を用いて, Bitmap8 から BitmapF への変換処理を行います.
		 * 
		 * colF = col8 / 255
		 * @param dest 変換後のデータを格納する BitmapF.
		 * @param src 変換対象のデータを格納する Bitmap8.
		 * @exception ParameterException dest と src の画像サイズが異なります.
		 * @remarks dest と src は幅と高さがそれぞれ一致している必要があります.
		 */
		static void Convert(BitmapF &dest, const Bitmap8 &src);

		/// Bitmap16 から BitmapF へ変換
		/**
		 * 以下の計算式を用いて, Bitmap16 から BitmapF への変換処理を行います.
		 * 
		 * colF = col16 / 65535
		 * @param dest 変換後のデータを格納する BitmapF.
		 * @param src 変換対象のデータを格納する Bitmap16.
		 * @exception ParameterException dest と src の画像サイズが異なります.
		 * @remarks dest と src は幅と高さがそれぞれ一致している必要があります.
		 */
		static void Convert(BitmapF &dest, const Bitmap16 &src);
	};
}

#endif
