// wakaba/blender.cpp
#include "blender.h"
#include "exception.h"

using namespace Wakaba;

// 定数
#define WIDTH_OFS 16
#define HEIGHT_OFS 20
#define PIXEL_SIZE 16

#define ONE_F 1.0f
#define HALF_F 0.5f
#define EPSILON_F 0.000001f

/*
 * @note SSE2 の使用方針
 * BitmapF のバッファは 16 バイトアライメントに合わせてあるので movaps 命令が使える.
 * それ以外のデータの転送は movups 命令を使う.
 */

namespace {
	// 設定を最適化
	bool Optimize(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		Int32 dest_w = dest.Width();
		Int32 dest_h = dest.Height();
		Int32 src_w = src.Width();
		Int32 src_h = src.Height();
	
		// アルファ値の確認
		if (info.alpha <= 0.0f && info.operation != Blender::OPERATION_COPY)
			return false;
		if (info.alpha > 1.0f)
			info.alpha = 1.0f;

		// 幅の符号調整
		if (info.width < 0) {
			info.dest_x += info.width;
			info.src_x += info.width;
			info.width = -info.width;
		}

		// 高さの符号調整
		if (info.height < 0) {
			info.dest_y += info.height;
			info.src_y += info.height;
			info.height = -info.height;
		}

		Int32 diff = 0, tmp;

		// 右端の調整
		tmp = (dest_w - info.dest_x < src_w - info.src_x) ? dest_w - info.dest_x : src_w - info.src_x;
		tmp -= info.width;
		if (tmp < 0)
			diff += tmp;

		// 左端の調整
		tmp = (info.dest_x < info.src_x) ? info.dest_x : info.src_x;
		if (tmp < 0) {
			diff += tmp;
			info.dest_x -= tmp;
			info.src_x -= tmp;
		}

		// 幅の確認
		if ((info.width += diff) <= 0)
			return false;

		diff = 0;

		// 下端の調整
		tmp = (dest_h - info.dest_y < src_h - info.src_y) ? dest_h - info.dest_y : src_h - info.src_y;
		tmp -= info.height;
		if (tmp < 0)
			diff += tmp;

		// 上端の調整
		tmp = (info.dest_y < info.src_y) ? info.dest_y : info.src_y;
		if (tmp < 0) {
			diff += tmp;
			info.dest_y -= tmp;
			info.src_y -= tmp;
		}

		// 高さの確認
		if ((info.height += diff) <= 0)
			return false;

		return true;
	}

	// destにsrcをコピー（MN_BLENDINFOのアルファ値は無視）
	// Cr = Cs
	void Blend_Copy(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込んで書き出すだけ
			movaps xmm0, [esi]
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// destを上にしてアルファブレンド
	// Cr = Ad Cd + (~Ad) As Cs
	void Blend_DestAlpha(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);

		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm5, [EPSILON] // xmm5: e
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// アルファ合成
			movaps xmm2, xmm0
			movaps xmm3, xmm1
			shufps xmm2, xmm2, 0xff
			mulps xmm2, xmm6 // xmm2: A1 = Ad' = Ad * A
			shufps xmm3, xmm3, 0xff // xmm3: As
			movaps xmm4, xmm7
			subps xmm4, xmm2 // xmm4: 1 - Ad'
			mulps xmm3, xmm4 // xmm3: A2 = As * (1 - Ad')
			mulps xmm0, xmm2 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm3 // xmm1: C2 = A2 * Cs
			addps xmm0, xmm1 // xmm0: Cr = C1 + C2
			addps xmm2, xmm3 // xmm2: Ar = A1 + A2
			divps xmm0, xmm2 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm4, xmm5
			cmpps xmm4, xmm2, 1 // xmm4: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm4
			// アルファだけ置換
			shufps xmm2, xmm0, 0xa0
			shufps xmm0, xmm2, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// srcを上にしてアルファブレンド
	// （Photoshop：通常）
	// Cr = (~As) Ad Cd + As Cs
	void Blend_SrcAlpha(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);

		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm5, [EPSILON] // xmm5: e
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// アルファ合成
			movaps xmm2, xmm0
			movaps xmm3, xmm1
			shufps xmm2, xmm2, 0xff // xmm2: Ad
			shufps xmm3, xmm3, 0xff
			mulps xmm3, xmm6 // xmm3: A2 = As' = As * A
			movaps xmm4, xmm7
			subps xmm4, xmm3 // xmm4: 1 - As'
			mulps xmm2, xmm4 // xmm2: A1 = Ad * (1 - As')
			mulps xmm0, xmm2 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm3 // xmm1: C2 = A2 * Cs
			addps xmm0, xmm1 // xmm0: Cr = C1 + C2
			addps xmm2, xmm3 // xmm2: Ar = A1 + A2
			divps xmm0, xmm2 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movaps xmm4, xmm5
			cmpps xmm4, xmm2, 1 // xmm4: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm4
			// アルファだけ置換
			shufps xmm2, xmm0, 0xa0
			shufps xmm0, xmm2, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// 乗算
	// （Photoshop：乗算）
	// Cr = Cd Cs
	void Blend_Multiple(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(乗算)
			movaps xmm2, xmm0
			mulps xmm2, xmm1 // xmm2: Co = Cd * Cs
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// 反転減算
	// （Photoshop：焼き込み（リニア））
	// Cr = max(Cd - ~Cs, 0.0)
	void Blend_Subtract(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(反転減算)
			movaps xmm2, xmm0
			addps xmm2, xmm1
			subps xmm2, xmm7 // xmm2: Cd - (1 - Cs)
			xorps xmm3, xmm3 // xmm3: 0
			maxps xmm2, xmm3 // xmm2: Co = max(Cd - (1 - Cs), 0)
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// destとsrcを比較して暗い方
	// （Photoshop：比較（暗））
	// Cr = min(Cd, Cs)
	void Blend_Lower(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(比較して暗い方)
			movaps xmm2, xmm0
			minps xmm2, xmm1 // xmm2: Co = min(Cd, Cs)
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// 反転乗算
	// （Photoshop：スクリーン）
	// Cr = ~((~Cd) (~Cs))
	void Blend_Screen(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(反転乗算)
			movaps xmm2, xmm0
			addps xmm2, xmm1
			movaps xmm3, xmm0
			mulps xmm3, xmm1
			subps xmm2, xmm3 // xmm2: Co = 1 - (1 - Cd) * (1 - Cs) = Cd + Cs - Cd * Cs
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// 加算
	// （Photoshop：覆い焼き（リニア））
	// Cr = min(Cd + Cs, 1.0)
	void Blend_Add(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(加算)
			movaps xmm2, xmm0
			movaps xmm3, xmm7
			addps xmm2, xmm1 // xmm2: Cd + Cs
			minps xmm2, xmm7 // xmm2: Co = min(Cd + Cs, 1)
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// destとsrcを比較して明るい方
	// （Photoshop：比較（明））
	// Cr = max(Cd, Cs)
	void Blend_Higher(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(比較して明るい方)
			movaps xmm2, xmm0
			maxps xmm2, xmm1 // xmm2: Co = max(Cd, Cs)
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// dest<0.5で乗算、dest>0.5で反転乗算
	// （Phosothop：オーバーレイ）
	// Cr = (Cd <= 0.5) ? 2.0 Cd Cs : ~(2.0 (~Cd) (~Cs))
	void Blend_Overlay(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF HALF = {HALF_F, HALF_F, HALF_F, HALF_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(オーバーレイ)
			movaps xmm2, xmm0
			movaps xmm3, xmm0
			mulps xmm2, xmm1 // xmm2: Cd * Cs
			addps xmm3, xmm1 // xmm3: Cd + Cs
			subps xmm3, xmm2 // xmm3: Cd + Cs - Cd * Cs
			addps xmm2, xmm2 // xmm2: Co' = 2 * Cd * Cs
			addps xmm3, xmm3 // xmm3: 2 * (Cd + Cs - Cd * Cs)
			subps xmm3, xmm7 // xmm3: Co'' = 1 - (2 * (1 - Cd) * (1 - Cs)) = 2 * (Cd + Cs - Cd * Cs) - 1
			movups xmm4, [HALF] // xmm4: 0.5
			movups xmm5, [HALF] // xmm5: 0.5
			cmpps xmm4, xmm0, 5 // xmm4: 0.5 >= Cd ? ffffffff : 00000000
			cmpps xmm5, xmm0, 1 // xmm4: 0.5 < Cd ? ffffffff : 00000000
			andps xmm2, xmm4
			andps xmm3, xmm5
			orps xmm2, xmm3 // xmm2: Co = Cd <= 0.5 ? Co' : Co''
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			//loop LOOP1
			dec ecx
			jnz LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// 差の絶対値
	// （Phosothop：差の絶対値）
	// Cr = |Cd - Cs|
	void Blend_Difference(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(差の絶対値)
			movaps xmm2, xmm0
			movaps xmm3, xmm1
			subps xmm2, xmm1 // xmm2: Cd - Cs
			subps xmm3, xmm0 // xmm3: Cs - Cd
			maxps xmm2, xmm3 // xmm2: Co = max(Cd - Cs, Cs - Cd)
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// 排他的和
	// （Phosothop：除外）
	// Cr = ~Cd Cs + Cd ~Cs
	void Blend_Exclusion(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSEで使う定数
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: 転送先
			mov esi, src_col // esi: 転送元
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: スキャンラインの幅
			mov ebx, [eax+HEIGHT_OFS] // ebx: スキャンライン数
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// スキャンラインここから
			mov ecx, edx
LOOP1:
			// 読み込み
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// 色の計算(除外)
			movaps xmm2, xmm0
			movaps xmm3, xmm0
			addps xmm2, xmm1
			mulps xmm3, xmm1
			subps xmm2, xmm3
			subps xmm2, xmm3 // xmm2: Co = (1 - Cd) * Cs + Cd * (1 - Cs) = Cd + Cs - 2 * Cd * Cs
			// アルファ合成
			movaps xmm3, xmm0
			movaps xmm4, xmm1
			shufps xmm3, xmm3, 0xff // xmm3: Ad
			shufps xmm4, xmm4, 0xff
			mulps xmm4, xmm6 // xmm4: As' = As * A
			movaps xmm5, xmm3
			mulps xmm5, xmm4 // xmm5: A3 = Ad * As'
			subps xmm3, xmm5 // xmm3: A1 = Ad * (1 - As')
			subps xmm4, xmm5 // xmm4: A2 = (1 - Ad) * As'
			mulps xmm0, xmm3 // xmm0: C1 = A1 * Cd
			mulps xmm1, xmm4 // xmm1: C2 = A2 * Cs
			mulps xmm2, xmm5 // xmm2: C3 = A3 * Co
			addps xmm0, xmm1
			addps xmm3, xmm4
			addps xmm0, xmm2 // xmm0: Cr = C1 + C2 + C3
			addps xmm3, xmm5 // xmm3: Ar = A1 + A2 + A3
			divps xmm0, xmm3 // xmm0: Cr / Ar
			// EPSILONよりArが小さければ0.0f(全ビット0)にする
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// アルファだけ置換
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// 書き出し
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// スキャンラインここまで
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}
}

// ビットマップの合成
void Blender::Blend(BitmapF &dest, const BitmapF &src, const Blender::BlendInfo &info)
{
	// 同じオブジェクト同士では合成できない.
	if (&dest == &src)
		throw ParameterException();

	// 設定をコピー
	BlendInfo temp_info = info;

	// 最適化
	if (!::Optimize(dest, src, temp_info))
		return;

	// 合成
	switch (temp_info.operation) {
	case OPERATION_COPY: ::Blend_Copy(dest, src, temp_info); break;
	case OPERATION_DESTALPHA: ::Blend_DestAlpha(dest, src, temp_info); break;
	case OPERATION_SRCALPHA: ::Blend_SrcAlpha(dest, src, temp_info); break;
	case OPERATION_MULTIPLE: ::Blend_Multiple(dest, src, temp_info); break;
	case OPERATION_SUBTRACT: ::Blend_Subtract(dest, src, temp_info); break;
	case OPERATION_LOWER: ::Blend_Lower(dest, src, temp_info); break;
	case OPERATION_SCREEN: ::Blend_Screen(dest, src, temp_info); break;
	case OPERATION_ADD: ::Blend_Add(dest, src, temp_info); break;
	case OPERATION_HIGHER: ::Blend_Higher(dest, src, temp_info); break;
	case OPERATION_OVERLAY: ::Blend_Overlay(dest, src, temp_info); break;
	case OPERATION_DIFFERENCE: ::Blend_Difference(dest, src, temp_info); break;
	case OPERATION_EXCLUSION: ::Blend_Exclusion(dest, src, temp_info); break;
	default:
		throw ParameterException();
	}
}
