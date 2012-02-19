// wakaba/color.h
#ifndef WAKABA_COLOR_H
#define WAKABA_COLOR_H

#include "basetype.h"

namespace Wakaba {

	/// ARGB �F���
	template<typename ElementT>
	struct Color {
		/// �s�����x
		ElementT a;
		/// �ԐF�̋���
		ElementT r;
		/// �ΐF�̋���
		ElementT g;
		/// �F�̋���
		ElementT b;
	};

	/// �f�[�^�i�[���� Bitmap �̃o�b�t�@�Ɠ����� ARGB �F���
	template<typename ElementT>
	struct RawColor {
		/// �F�̋���
		ElementT b;
		/// �ΐF�̋���
		ElementT g;
		/// �ԐF�̋���
		ElementT r;
		/// �s�����x
		ElementT a;
	};

	/// 8 �r�b�g ARGB �F���
	typedef Color<UInt8> Color8;
	/// 16 �r�b�g ARGB �F���
	typedef Color<UInt16> Color16;
	/// ���������_�� ARGB �F���
	typedef Color<float> ColorF;

	/// �i�[�������ۂ̃f�[�^�Ɠ����� 8 �r�b�g ARGB �F���
	typedef RawColor<UInt8> RawColor8;
	/// �i�[�������ۂ̃f�[�^�Ɠ����� 16 �r�b�g ARGB �F���
	typedef RawColor<UInt16> RawColor16;
	/// �i�[�������ۂ̃f�[�^�Ɠ��������������_�� ARGB �F���
	typedef RawColor<float> RawColorF;
}

#endif
