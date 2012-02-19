// wakaba/converter.h
#ifndef WAKABA_CONVERTER_H
#define WAKABA_CONVERTER_H

#include "dllexport.h"
#include "bitmap.h"

namespace Wakaba {
	/// �r�b�g�}�b�v�̌^�ϊ�
	class WAKABA_API Converter {
	public:
		/// Bitmap16 ���� Bitmap8 �֕ϊ�
		/**
		 * �ȉ��̌v�Z����p����, Bitmap16 ���� Bitmap8 �ւ̕ϊ��������s���܂�.
		 * 
		 * col8 = col16 >> 8
		 * @param dest �ϊ���̃f�[�^���i�[���� Bitmap8.
		 * @param src �ϊ��Ώۂ̃f�[�^���i�[���� Bitmap16.
		 * @exception ParameterException dest �� src �̉摜�T�C�Y���قȂ�܂�.
		 * @remarks dest �� src �͕��ƍ��������ꂼ���v���Ă���K�v������܂�.
		 */
		static void Convert(Bitmap8 &dest, const Bitmap16 &src);

		/// BitmapF ���� Bitmap8 �֕ϊ�
		/**
		 * �ȉ��̌v�Z����p����, BitmapF ���� Bitmap8 �ւ̕ϊ��������s���܂�.
		 * 
		 * col8 = floor(colF * 255 + 0.5)
		 * @param dest �ϊ���̃f�[�^���i�[���� Bitmap8.
		 * @param src �ϊ��Ώۂ̃f�[�^���i�[���� BitmapF.
		 * @exception ParameterException dest �� src �̉摜�T�C�Y���قȂ�܂�.
		 * @remarks dest �� src �͕��ƍ��������ꂼ���v���Ă���K�v������܂�.
		 */
		static void Convert(Bitmap8 &dest, const BitmapF &src);

		/// Bitmap8 ���� Bitmap16 �֕ϊ�
		/**
		 * �ȉ��̌v�Z����p����, Bitmap8 ���� Bitmap16 �ւ̕ϊ��������s���܂�.
		 * 
		 * col16 = col8 * 0x0101
		 * @param dest �ϊ���̃f�[�^���i�[���� Bitmap16.
		 * @param src �ϊ��Ώۂ̃f�[�^���i�[���� Bitmap8.
		 * @exception ParameterException dest �� src �̉摜�T�C�Y���قȂ�܂�.
		 * @remarks dest �� src �͕��ƍ��������ꂼ���v���Ă���K�v������܂�.
		 */
		static void Convert(Bitmap16 &dest, const Bitmap8 &src);

		/// BitmapF ���� Bitmap16 �֕ϊ�
		/**
		 * �ȉ��̌v�Z����p����, BitmapF ���� Bitmap16 �ւ̕ϊ��������s���܂�.
		 * 
		 * col16 = floor(colF * 65535 + 0.5)
		 * @param dest �ϊ���̃f�[�^���i�[���� Bitmap16.
		 * @param src �ϊ��Ώۂ̃f�[�^���i�[���� BitmapF.
		 * @exception ParameterException dest �� src �̉摜�T�C�Y���قȂ�܂�.
		 * @remarks dest �� src �͕��ƍ��������ꂼ���v���Ă���K�v������܂�.
		 */
		static void Convert(Bitmap16 &dest, const BitmapF &src);

		/// Bitmap8 ���� BitmapF �֕ϊ�
		/**
		 * �ȉ��̌v�Z����p����, Bitmap8 ���� BitmapF �ւ̕ϊ��������s���܂�.
		 * 
		 * colF = col8 / 255
		 * @param dest �ϊ���̃f�[�^���i�[���� BitmapF.
		 * @param src �ϊ��Ώۂ̃f�[�^���i�[���� Bitmap8.
		 * @exception ParameterException dest �� src �̉摜�T�C�Y���قȂ�܂�.
		 * @remarks dest �� src �͕��ƍ��������ꂼ���v���Ă���K�v������܂�.
		 */
		static void Convert(BitmapF &dest, const Bitmap8 &src);

		/// Bitmap16 ���� BitmapF �֕ϊ�
		/**
		 * �ȉ��̌v�Z����p����, Bitmap16 ���� BitmapF �ւ̕ϊ��������s���܂�.
		 * 
		 * colF = col16 / 65535
		 * @param dest �ϊ���̃f�[�^���i�[���� BitmapF.
		 * @param src �ϊ��Ώۂ̃f�[�^���i�[���� Bitmap16.
		 * @exception ParameterException dest �� src �̉摜�T�C�Y���قȂ�܂�.
		 * @remarks dest �� src �͕��ƍ��������ꂼ���v���Ă���K�v������܂�.
		 */
		static void Convert(BitmapF &dest, const Bitmap16 &src);
	};
}

#endif
