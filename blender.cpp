// wakaba/blender.cpp
#include "blender.h"
#include "exception.h"

using namespace Wakaba;

// �萔
#define WIDTH_OFS 16
#define HEIGHT_OFS 20
#define PIXEL_SIZE 16

#define ONE_F 1.0f
#define HALF_F 0.5f
#define EPSILON_F 0.000001f

/*
 * @note SSE2 �̎g�p���j
 * BitmapF �̃o�b�t�@�� 16 �o�C�g�A���C�����g�ɍ��킹�Ă���̂� movaps ���߂��g����.
 * ����ȊO�̃f�[�^�̓]���� movups ���߂��g��.
 */

namespace {
	// �ݒ���œK��
	bool Optimize(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		Int32 dest_w = dest.Width();
		Int32 dest_h = dest.Height();
		Int32 src_w = src.Width();
		Int32 src_h = src.Height();
	
		// �A���t�@�l�̊m�F
		if (info.alpha <= 0.0f && info.operation != Blender::OPERATION_COPY)
			return false;
		if (info.alpha > 1.0f)
			info.alpha = 1.0f;

		// ���̕�������
		if (info.width < 0) {
			info.dest_x += info.width;
			info.src_x += info.width;
			info.width = -info.width;
		}

		// �����̕�������
		if (info.height < 0) {
			info.dest_y += info.height;
			info.src_y += info.height;
			info.height = -info.height;
		}

		Int32 diff = 0, tmp;

		// �E�[�̒���
		tmp = (dest_w - info.dest_x < src_w - info.src_x) ? dest_w - info.dest_x : src_w - info.src_x;
		tmp -= info.width;
		if (tmp < 0)
			diff += tmp;

		// ���[�̒���
		tmp = (info.dest_x < info.src_x) ? info.dest_x : info.src_x;
		if (tmp < 0) {
			diff += tmp;
			info.dest_x -= tmp;
			info.src_x -= tmp;
		}

		// ���̊m�F
		if ((info.width += diff) <= 0)
			return false;

		diff = 0;

		// ���[�̒���
		tmp = (dest_h - info.dest_y < src_h - info.src_y) ? dest_h - info.dest_y : src_h - info.src_y;
		tmp -= info.height;
		if (tmp < 0)
			diff += tmp;

		// ��[�̒���
		tmp = (info.dest_y < info.src_y) ? info.dest_y : info.src_y;
		if (tmp < 0) {
			diff += tmp;
			info.dest_y -= tmp;
			info.src_y -= tmp;
		}

		// �����̊m�F
		if ((info.height += diff) <= 0)
			return false;

		return true;
	}

	// dest��src���R�s�[�iMN_BLENDINFO�̃A���t�@�l�͖����j
	// Cr = Cs
	void Blend_Copy(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���ŏ����o������
			movaps xmm0, [esi]
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// dest����ɂ��ăA���t�@�u�����h
	// Cr = Ad Cd + (~Ad) As Cs
	void Blend_DestAlpha(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);

		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm5, [EPSILON] // xmm5: e
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm4, xmm5
			cmpps xmm4, xmm2, 1 // xmm4: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm4
			// �A���t�@�����u��
			shufps xmm2, xmm0, 0xa0
			shufps xmm0, xmm2, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// src����ɂ��ăA���t�@�u�����h
	// �iPhotoshop�F�ʏ�j
	// Cr = (~As) Ad Cd + As Cs
	void Blend_SrcAlpha(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);

		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm5, [EPSILON] // xmm5: e
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movaps xmm4, xmm5
			cmpps xmm4, xmm2, 1 // xmm4: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm4
			// �A���t�@�����u��
			shufps xmm2, xmm0, 0xa0
			shufps xmm0, xmm2, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// ��Z
	// �iPhotoshop�F��Z�j
	// Cr = Cd Cs
	void Blend_Multiple(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(��Z)
			movaps xmm2, xmm0
			mulps xmm2, xmm1 // xmm2: Co = Cd * Cs
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// ���]���Z
	// �iPhotoshop�F�Ă����݁i���j�A�j�j
	// Cr = max(Cd - ~Cs, 0.0)
	void Blend_Subtract(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(���]���Z)
			movaps xmm2, xmm0
			addps xmm2, xmm1
			subps xmm2, xmm7 // xmm2: Cd - (1 - Cs)
			xorps xmm3, xmm3 // xmm3: 0
			maxps xmm2, xmm3 // xmm2: Co = max(Cd - (1 - Cs), 0)
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// dest��src���r���ĈÂ���
	// �iPhotoshop�F��r�i�Áj�j
	// Cr = min(Cd, Cs)
	void Blend_Lower(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(��r���ĈÂ���)
			movaps xmm2, xmm0
			minps xmm2, xmm1 // xmm2: Co = min(Cd, Cs)
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// ���]��Z
	// �iPhotoshop�F�X�N���[���j
	// Cr = ~((~Cd) (~Cs))
	void Blend_Screen(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(���]��Z)
			movaps xmm2, xmm0
			addps xmm2, xmm1
			movaps xmm3, xmm0
			mulps xmm3, xmm1
			subps xmm2, xmm3 // xmm2: Co = 1 - (1 - Cd) * (1 - Cs) = Cd + Cs - Cd * Cs
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// ���Z
	// �iPhotoshop�F�����Ă��i���j�A�j�j
	// Cr = min(Cd + Cs, 1.0)
	void Blend_Add(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(���Z)
			movaps xmm2, xmm0
			movaps xmm3, xmm7
			addps xmm2, xmm1 // xmm2: Cd + Cs
			minps xmm2, xmm7 // xmm2: Co = min(Cd + Cs, 1)
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// dest��src���r���Ė��邢��
	// �iPhotoshop�F��r�i���j�j
	// Cr = max(Cd, Cs)
	void Blend_Higher(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(��r���Ė��邢��)
			movaps xmm2, xmm0
			maxps xmm2, xmm1 // xmm2: Co = max(Cd, Cs)
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// dest<0.5�ŏ�Z�Adest>0.5�Ŕ��]��Z
	// �iPhosothop�F�I�[�o�[���C�j
	// Cr = (Cd <= 0.5) ? 2.0 Cd Cs : ~(2.0 (~Cd) (~Cs))
	void Blend_Overlay(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF HALF = {HALF_F, HALF_F, HALF_F, HALF_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(�I�[�o�[���C)
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
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			//loop LOOP1
			dec ecx
			jnz LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// ���̐�Βl
	// �iPhosothop�F���̐�Βl�j
	// Cr = |Cd - Cs|
	void Blend_Difference(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(���̐�Βl)
			movaps xmm2, xmm0
			movaps xmm3, xmm1
			subps xmm2, xmm1 // xmm2: Cd - Cs
			subps xmm3, xmm0 // xmm3: Cs - Cd
			maxps xmm2, xmm3 // xmm2: Co = max(Cd - Cs, Cs - Cd)
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}

	// �r���I�a
	// �iPhosothop�F���O�j
	// Cr = ~Cd Cs + Cd ~Cs
	void Blend_Exclusion(BitmapF &dest, const BitmapF &src, Blender::BlendInfo &info)
	{
		RawColorF *dest_col = reinterpret_cast<RawColorF *>(dest.Buffer()) + (info.dest_y * dest.Width()) + info.dest_x;
		const RawColorF *src_col = reinterpret_cast<const RawColorF *>(src.Buffer()) + (info.src_y * src.Width()) + info.src_x;
		const Int32 dest_skip = PIXEL_SIZE * (dest.Width() - info.width);
		const Int32 src_skip = PIXEL_SIZE * (src.Width() - info.width);
	
		// SSE�Ŏg���萔
		const RawColorF ALPHA = {info.alpha, info.alpha, info.alpha, info.alpha};
		const RawColorF ONE = {ONE_F, ONE_F, ONE_F, ONE_F};
		const RawColorF EPSILON = {EPSILON_F, EPSILON_F, EPSILON_F, EPSILON_F};

		__asm {
			mov edi, dest_col // edi: �]����
			mov esi, src_col // esi: �]����
			mov eax, info
			mov edx, [eax+WIDTH_OFS] // edx: �X�L�������C���̕�
			mov ebx, [eax+HEIGHT_OFS] // ebx: �X�L�������C����
			movups xmm6, [ALPHA] // xmm6: A
			movups xmm7, [ONE] // xmm7: 1
LOOP2:
			// �X�L�������C����������
			mov ecx, edx
LOOP1:
			// �ǂݍ���
			movaps xmm0, [edi] // xmm0: Cd
			movaps xmm1, [esi] // xmm1: Cs
			// �F�̌v�Z(���O)
			movaps xmm2, xmm0
			movaps xmm3, xmm0
			addps xmm2, xmm1
			mulps xmm3, xmm1
			subps xmm2, xmm3
			subps xmm2, xmm3 // xmm2: Co = (1 - Cd) * Cs + Cd * (1 - Cs) = Cd + Cs - 2 * Cd * Cs
			// �A���t�@����
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
			// EPSILON���Ar�����������0.0f(�S�r�b�g0)�ɂ���
			movups xmm5, [EPSILON]
			cmpps xmm5, xmm3, 1 // xmm5: e < Ar ? ffffffff : 000000000
			andps xmm0, xmm5
			// �A���t�@�����u��
			shufps xmm3, xmm0, 0xa0
			shufps xmm0, xmm3, 0x24
			// �����o��
			movaps [edi], xmm0

			add edi, PIXEL_SIZE
			add esi, PIXEL_SIZE
			loop LOOP1
			// �X�L�������C�������܂�
			add edi, dest_skip
			add esi, src_skip
			dec ebx
			jnz LOOP2
		}
	}
}

// �r�b�g�}�b�v�̍���
void Blender::Blend(BitmapF &dest, const BitmapF &src, const Blender::BlendInfo &info)
{
	// �����I�u�W�F�N�g���m�ł͍����ł��Ȃ�.
	if (&dest == &src)
		throw ParameterException();

	// �ݒ���R�s�[
	BlendInfo temp_info = info;

	// �œK��
	if (!::Optimize(dest, src, temp_info))
		return;

	// ����
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
