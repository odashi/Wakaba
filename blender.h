// wakaba/blender.h
#ifndef WAKABA_BLENDER_H
#define WAKABA_BLENDER_H

#include "dllexport.h"
#include "basetype.h"
#include "bitmap.h"

namespace Wakaba {
	/// ビットマップの合成処理
	class WAKABA_API Blender {
	public:
		/// ビットマップ合成演算の種類
		enum Operation {
			/// 単純コピー
			/**
			 * result = src
			 * 
			 * この演算では dest のアルファ, および BlendInfo::alpha は無視されます.
			 */
			OPERATION_COPY,
			/// dest を上にしてアルファ合成
			/**
			 * result = dest
			 * 
			 * この演算では BlendInfo::alpha は dest に積算されます.
			 */
			OPERATION_DESTALPHA,
			/// アルファ合成
			/**
			 * result = src
			 * 
			 * Photoshop の「通常」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_SRCALPHA,
			/// 乗算
			/**
			 * result = dest * src
			 * 
			 * Photoshop の「乗算」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_MULTIPLE,
			/// 反転減算
			/**
			 * result = max(dest - (1 - src), 0)
			 * 
			 * Photoshop の「焼き込み(リニア)」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_SUBTRACT,
			/// 比較して暗い方
			/**
			 * result = min(dest, src)
			 * 
			 * Photoshop の「比較(暗)」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_LOWER,
			/// 反転乗算
			/**
			 * result = 1 - (1 - dest) * (1 - src)
			 * 
			 * Photoshop の「スクリーン」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_SCREEN,
			/// 加算
			/**
			 * result = min(dest + src, 1)
			 * 
			 * Photoshop の「覆い焼き(リニア)」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_ADD,
			/// 比較して明るい方
			/**
			 * result = max(dest, src)
			 * 
			 * Photoshop の「比較(明)」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_HIGHER,
			/// dest < 50% で乗算, dest > 50% で反転乗算
			/**
			 * result = dest <= 0.5 ? 2 * dest * src : 1 - 2 * (1 - dest) * (1 - src)
			 * 
			 * Photoshop の「オーバーレイ」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_OVERLAY,
			/// 差の絶対値
			/**
			 * result = abs(dest - src)
			 * 
			 * Photoshop の「差の絶対値」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_DIFFERENCE,
			/// 排他的和
			/**
			 * result = (1 - dest) * src + dest * (1 - src)
			 * 
			 * Photoshop の「除外」と同様の演算です.
			 * この演算では BlendInfo::alpha は src に積算されます.
			 */
			OPERATION_EXCLUSION
		};

		/// 画像合成の設定情報
		struct BlendInfo {
			/// dest の左上 X 座標
			Int32 dest_x;
			/// dest の左上 Y 座標
			Int32 dest_y;
			/// src の左上 X 座標
			Int32 src_x;
			/// src の左上 Y 座標
			Int32 src_y;
			/// 合成を行う領域の幅
			Int32 width;
			/// 合成を行う領域の高さ
			Int32 height;
			/// 不透明度 (0.0f ~ 1.0f)
			/**
			 * この値は, 演算の種類によって dest か src の不透明度に事前に積算されます.
			 */
			float alpha;
			/// 演算の種類
			Operation operation;
		};

		/// ビットマップを合成
		/**
		 * 2 枚のビットマップを指定した演算で合成して 1 枚にします.
		 * @param dest 適用先の BitmapF.
		 * @param src 合成する BitmapF.
		 * @param info 設定情報を表す Blender::BlendInfo.
		 * @exception ParameterException 非対応の演算が指定されたか, dest と src に同じオブジェクトが指定されました.
		 */
		static void Blend(BitmapF &dest, const BitmapF &src, const BlendInfo &info);
	};
}

#endif
