// wakaba/bitmap.h
#ifndef WAKABA_BITMAP_H
#define WAKABA_BITMAP_H

#include "dllexport.h"
#include "basetype.h"
#include "color.h"

/*
 * @note
 * メモリ確保の関係でビットマップはテンプレート化が難しい.
 * どうにか回避策がほしい.
 */

namespace Wakaba {
	/// 8 ビット ARGB ビットマップ
	class WAKABA_API Bitmap8 {
		UInt32 width_;
		UInt32 height_;
		UInt8 *mem_;

		// c-tor/d-tor
		Bitmap8(UInt32 width, UInt32 height);
		~Bitmap8();

		// 禁止操作
		Bitmap8(const Bitmap8 &);
		Bitmap8 &operator =(const Bitmap8 &);
	public:
		/// 幅と高さを指定して新規ビットマップを生成
		/**
		 * 幅, 高さを指定して, 初期化されていない新しいビットマップを生成します.
		 * 
		 * @param width ビットマップの幅.
		 * @param height ビットマップの高さ.
		 * @retval Bitmap8* 新しい Bitmap8 オブジェクト.
		 * @exception ParameterException 幅か高さが不正な値です.
		 * @remarks width と heightに 0 を指定することはできません. かならず 1 以上の値である必要があります.
		 */
		static Bitmap8 *New(UInt32 width, UInt32 height);

		/// ビットマップの破棄
		/**
		 * データストリームを解放してビットマップを破棄します.
		 * @remarks この関数を呼び出すとオブジェクトが無効になります. 呼び出し後のオブジェクトを使用してはいけません.
		 * @remarks この関数は nullptr に対して呼び出すことができます. その場合は何も行いません.
		 */
		void Dispose();

		/// 指定した色で全体をクリア
		/**
		 * 指定した色でビットマップ全体を初期化します.
		 * @param color 適用する色.
		 */
		void Clear(const Color8 &color);

		/// ビットマップのコピー
		/**
		 * 指定したビットマップから情報をコピーします.
		 * 
		 * 各ビットマップは幅と高さが一致していなければなりません.
		 * 
		 * @param src コピー元となる Bitmap8 オブジェクト.
		 * @exception FormatException ビットマップの幅か高さが一致しません.
		 * @remarks 各ビットマップの幅と高さは一致していなければなりません.
		 */
		void CopyFrom(const Bitmap8 &src);

		/// ビットマップの幅を取得
		UInt32 Width() const { return width_; }
		/// ビットマップの高さを取得
		UInt32 Height() const { return height_; }
		/// データの先頭へのポインタを取得
		UInt8 *Buffer() { return mem_; }
		/// データの先頭へのポインタを取得 (const)
		const UInt8 *Buffer() const { return mem_; }
	};

	/// 16 ビット ARGB ビットマップ
	class WAKABA_API Bitmap16 {
		UInt32 width_;
		UInt32 height_;
		UInt16 *mem_;

		// c-tor/d-tor
		Bitmap16(UInt32 width, UInt32 height);
		~Bitmap16();

		// 禁止操作
		Bitmap16(const Bitmap16 &);
		Bitmap16 &operator =(const Bitmap16 &);
	public:
		/// 幅と高さを指定して新規ビットマップを生成
		/**
		 * 幅, 高さを指定して, 初期化されていない新しいビットマップを生成します.
		 * 
		 * @param width ビットマップの幅.
		 * @param height ビットマップの高さ.
		 * @retval Bitmap16* 新しい Bitmap16 オブジェクト.
		 * @exception ParameterException 幅か高さが不正な値です.
		 * @remarks width と heightに 0 を指定することはできません. かならず 1 以上の値である必要があります.
		 */
		static Bitmap16 *New(UInt32 width, UInt32 height);

		/// ビットマップの破棄
		/**
		 * データストリームを解放してビットマップを破棄します.
		 * @remarks この関数を呼び出すとオブジェクトが無効になります. 呼び出し後のオブジェクトを使用してはいけません.
		 * @remarks この関数は nullptr に対して呼び出すことができます. その場合は何も行いません.
		 */
		void Dispose();

		/// 指定した色で全体をクリア
		/**
		 * 指定した色でビットマップ全体を初期化します.
		 * @param color 適用する色.
		 */
		void Clear(const Color16 &color);

		/// ビットマップのコピー
		/**
		 * 指定したビットマップから情報をコピーします.
		 * 
		 * 各ビットマップは幅と高さが一致していなければなりません.
		 * 
		 * @param src コピー元となる Bitmap16 オブジェクト.
		 * @exception FormatException ビットマップの幅か高さが一致しません.
		 * @remarks 各ビットマップの幅と高さは一致していなければなりません.
		 */
		void CopyFrom(const Bitmap16 &src);

		/// ビットマップの幅を取得
		UInt32 Width() const { return width_; }
		/// ビットマップの高さを取得
		UInt32 Height() const { return height_; }
		/// データの先頭へのポインタを取得
		UInt16 *Buffer() { return mem_; }
		/// データの先頭へのポインタを取得 (const)
		const UInt16 *Buffer() const { return mem_; }
	};

	/// 浮動小数点数 ARGB ビットマップ
	class WAKABA_API BitmapF {
		UInt32 width_;
		UInt32 height_;
		float *mem_;

		// c-tor/d-tor
		BitmapF(UInt32 width, UInt32 height);
		~BitmapF();

		// 禁止操作
		BitmapF(const BitmapF &);
		BitmapF &operator =(const BitmapF &);
	public:
		/// 幅と高さを指定して新規ビットマップを生成
		/**
		 * 幅, 高さを指定して, 初期化されていない新しいビットマップを生成します.
		 * 
		 * @param width ビットマップの幅.
		 * @param height ビットマップの高さ.
		 * @retval BitmapF* 新しい BitmapF オブジェクト.
		 * @exception ParameterException 幅か高さが不正な値です.
		 * @remarks width と heightに 0 を指定することはできません. かならず 1 以上の値である必要があります.
		 */
		static BitmapF *New(UInt32 width, UInt32 height);

		/// ビットマップの破棄
		/**
		 * データストリームを解放してビットマップを破棄します.
		 * @remarks この関数を呼び出すとオブジェクトが無効になります. 呼び出し後のオブジェクトを使用してはいけません.
		 * @remarks この関数は nullptr に対して呼び出すことができます. その場合は何も行いません.
		 */
		void Dispose();

		/// 指定した色で全体をクリア
		/**
		 * 指定した色でビットマップ全体を初期化します.
		 * @param color 適用する色.
		 */
		void Clear(const ColorF &color);

		/// ビットマップのコピー
		/**
		 * 指定したビットマップから情報をコピーします.
		 * 
		 * 各ビットマップは幅と高さが一致していなければなりません.
		 * 
		 * @param src コピー元となる BitmapF オブジェクト.
		 * @exception FormatException ビットマップの幅か高さが一致しません.
		 * @remarks 各ビットマップの幅と高さは一致していなければなりません.
		 */
		void CopyFrom(const BitmapF &src);

		/// ビットマップの幅を取得
		UInt32 Width() const { return width_; }
		/// ビットマップの高さを取得
		UInt32 Height() const { return height_; }
		/// データの先頭へのポインタを取得
		float *Buffer() { return mem_; }
		/// データの先頭へのポインタを取得 (const)
		const float *Buffer() const { return mem_; }
	};
}

#endif
