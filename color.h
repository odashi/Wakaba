// wakaba/color.h
#ifndef WAKABA_COLOR_H
#define WAKABA_COLOR_H

#include "basetype.h"

namespace Wakaba {

	/// ARGB 色情報
	template<typename ElementT>
	struct Color {
		/// 不透明度
		ElementT a;
		/// 赤色の強さ
		ElementT r;
		/// 緑色の強さ
		ElementT g;
		/// 青色の強さ
		ElementT b;
	};

	/// データ格納順が Bitmap のバッファと等しい ARGB 色情報
	template<typename ElementT>
	struct RawColor {
		/// 青色の強さ
		ElementT b;
		/// 緑色の強さ
		ElementT g;
		/// 赤色の強さ
		ElementT r;
		/// 不透明度
		ElementT a;
	};

	/// 8 ビット ARGB 色情報
	typedef Color<UInt8> Color8;
	/// 16 ビット ARGB 色情報
	typedef Color<UInt16> Color16;
	/// 浮動小数点数 ARGB 色情報
	typedef Color<float> ColorF;

	/// 格納順が実際のデータと等しい 8 ビット ARGB 色情報
	typedef RawColor<UInt8> RawColor8;
	/// 格納順が実際のデータと等しい 16 ビット ARGB 色情報
	typedef RawColor<UInt16> RawColor16;
	/// 格納順が実際のデータと等しい浮動小数点数 ARGB 色情報
	typedef RawColor<float> RawColorF;
}

#endif
