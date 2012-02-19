// wakaba/bitmap.h
#ifndef WAKABA_BITMAP_H
#define WAKABA_BITMAP_H

#include "dllexport.h"
#include "basetype.h"
#include "color.h"

/*
 * @note
 * �������m�ۂ̊֌W�Ńr�b�g�}�b�v�̓e���v���[�g�������.
 * �ǂ��ɂ�����􂪂ق���.
 */

namespace Wakaba {
	/// 8 �r�b�g ARGB �r�b�g�}�b�v
	class WAKABA_API Bitmap8 {
		UInt32 width_;
		UInt32 height_;
		UInt8 *mem_;

		// c-tor/d-tor
		Bitmap8(UInt32 width, UInt32 height);
		~Bitmap8();

		// �֎~����
		Bitmap8(const Bitmap8 &);
		Bitmap8 &operator =(const Bitmap8 &);
	public:
		/// ���ƍ������w�肵�ĐV�K�r�b�g�}�b�v�𐶐�
		/**
		 * ��, �������w�肵��, ����������Ă��Ȃ��V�����r�b�g�}�b�v�𐶐����܂�.
		 * 
		 * @param width �r�b�g�}�b�v�̕�.
		 * @param height �r�b�g�}�b�v�̍���.
		 * @retval Bitmap8* �V���� Bitmap8 �I�u�W�F�N�g.
		 * @exception ParameterException �����������s���Ȓl�ł�.
		 * @remarks width �� height�� 0 ���w�肷�邱�Ƃ͂ł��܂���. ���Ȃ炸 1 �ȏ�̒l�ł���K�v������܂�.
		 */
		static Bitmap8 *New(UInt32 width, UInt32 height);

		/// �r�b�g�}�b�v�̔j��
		/**
		 * �f�[�^�X�g���[����������ăr�b�g�}�b�v��j�����܂�.
		 * @remarks ���̊֐����Ăяo���ƃI�u�W�F�N�g�������ɂȂ�܂�. �Ăяo����̃I�u�W�F�N�g���g�p���Ă͂����܂���.
		 * @remarks ���̊֐��� nullptr �ɑ΂��ČĂяo�����Ƃ��ł��܂�. ���̏ꍇ�͉����s���܂���.
		 */
		void Dispose();

		/// �w�肵���F�őS�̂��N���A
		/**
		 * �w�肵���F�Ńr�b�g�}�b�v�S�̂����������܂�.
		 * @param color �K�p����F.
		 */
		void Clear(const Color8 &color);

		/// �r�b�g�}�b�v�̃R�s�[
		/**
		 * �w�肵���r�b�g�}�b�v��������R�s�[���܂�.
		 * 
		 * �e�r�b�g�}�b�v�͕��ƍ�������v���Ă��Ȃ���΂Ȃ�܂���.
		 * 
		 * @param src �R�s�[���ƂȂ� Bitmap8 �I�u�W�F�N�g.
		 * @exception FormatException �r�b�g�}�b�v�̕�����������v���܂���.
		 * @remarks �e�r�b�g�}�b�v�̕��ƍ����͈�v���Ă��Ȃ���΂Ȃ�܂���.
		 */
		void CopyFrom(const Bitmap8 &src);

		/// �r�b�g�}�b�v�̕����擾
		UInt32 Width() const { return width_; }
		/// �r�b�g�}�b�v�̍������擾
		UInt32 Height() const { return height_; }
		/// �f�[�^�̐擪�ւ̃|�C���^���擾
		UInt8 *Buffer() { return mem_; }
		/// �f�[�^�̐擪�ւ̃|�C���^���擾 (const)
		const UInt8 *Buffer() const { return mem_; }
	};

	/// 16 �r�b�g ARGB �r�b�g�}�b�v
	class WAKABA_API Bitmap16 {
		UInt32 width_;
		UInt32 height_;
		UInt16 *mem_;

		// c-tor/d-tor
		Bitmap16(UInt32 width, UInt32 height);
		~Bitmap16();

		// �֎~����
		Bitmap16(const Bitmap16 &);
		Bitmap16 &operator =(const Bitmap16 &);
	public:
		/// ���ƍ������w�肵�ĐV�K�r�b�g�}�b�v�𐶐�
		/**
		 * ��, �������w�肵��, ����������Ă��Ȃ��V�����r�b�g�}�b�v�𐶐����܂�.
		 * 
		 * @param width �r�b�g�}�b�v�̕�.
		 * @param height �r�b�g�}�b�v�̍���.
		 * @retval Bitmap16* �V���� Bitmap16 �I�u�W�F�N�g.
		 * @exception ParameterException �����������s���Ȓl�ł�.
		 * @remarks width �� height�� 0 ���w�肷�邱�Ƃ͂ł��܂���. ���Ȃ炸 1 �ȏ�̒l�ł���K�v������܂�.
		 */
		static Bitmap16 *New(UInt32 width, UInt32 height);

		/// �r�b�g�}�b�v�̔j��
		/**
		 * �f�[�^�X�g���[����������ăr�b�g�}�b�v��j�����܂�.
		 * @remarks ���̊֐����Ăяo���ƃI�u�W�F�N�g�������ɂȂ�܂�. �Ăяo����̃I�u�W�F�N�g���g�p���Ă͂����܂���.
		 * @remarks ���̊֐��� nullptr �ɑ΂��ČĂяo�����Ƃ��ł��܂�. ���̏ꍇ�͉����s���܂���.
		 */
		void Dispose();

		/// �w�肵���F�őS�̂��N���A
		/**
		 * �w�肵���F�Ńr�b�g�}�b�v�S�̂����������܂�.
		 * @param color �K�p����F.
		 */
		void Clear(const Color16 &color);

		/// �r�b�g�}�b�v�̃R�s�[
		/**
		 * �w�肵���r�b�g�}�b�v��������R�s�[���܂�.
		 * 
		 * �e�r�b�g�}�b�v�͕��ƍ�������v���Ă��Ȃ���΂Ȃ�܂���.
		 * 
		 * @param src �R�s�[���ƂȂ� Bitmap16 �I�u�W�F�N�g.
		 * @exception FormatException �r�b�g�}�b�v�̕�����������v���܂���.
		 * @remarks �e�r�b�g�}�b�v�̕��ƍ����͈�v���Ă��Ȃ���΂Ȃ�܂���.
		 */
		void CopyFrom(const Bitmap16 &src);

		/// �r�b�g�}�b�v�̕����擾
		UInt32 Width() const { return width_; }
		/// �r�b�g�}�b�v�̍������擾
		UInt32 Height() const { return height_; }
		/// �f�[�^�̐擪�ւ̃|�C���^���擾
		UInt16 *Buffer() { return mem_; }
		/// �f�[�^�̐擪�ւ̃|�C���^���擾 (const)
		const UInt16 *Buffer() const { return mem_; }
	};

	/// ���������_�� ARGB �r�b�g�}�b�v
	class WAKABA_API BitmapF {
		UInt32 width_;
		UInt32 height_;
		float *mem_;

		// c-tor/d-tor
		BitmapF(UInt32 width, UInt32 height);
		~BitmapF();

		// �֎~����
		BitmapF(const BitmapF &);
		BitmapF &operator =(const BitmapF &);
	public:
		/// ���ƍ������w�肵�ĐV�K�r�b�g�}�b�v�𐶐�
		/**
		 * ��, �������w�肵��, ����������Ă��Ȃ��V�����r�b�g�}�b�v�𐶐����܂�.
		 * 
		 * @param width �r�b�g�}�b�v�̕�.
		 * @param height �r�b�g�}�b�v�̍���.
		 * @retval BitmapF* �V���� BitmapF �I�u�W�F�N�g.
		 * @exception ParameterException �����������s���Ȓl�ł�.
		 * @remarks width �� height�� 0 ���w�肷�邱�Ƃ͂ł��܂���. ���Ȃ炸 1 �ȏ�̒l�ł���K�v������܂�.
		 */
		static BitmapF *New(UInt32 width, UInt32 height);

		/// �r�b�g�}�b�v�̔j��
		/**
		 * �f�[�^�X�g���[����������ăr�b�g�}�b�v��j�����܂�.
		 * @remarks ���̊֐����Ăяo���ƃI�u�W�F�N�g�������ɂȂ�܂�. �Ăяo����̃I�u�W�F�N�g���g�p���Ă͂����܂���.
		 * @remarks ���̊֐��� nullptr �ɑ΂��ČĂяo�����Ƃ��ł��܂�. ���̏ꍇ�͉����s���܂���.
		 */
		void Dispose();

		/// �w�肵���F�őS�̂��N���A
		/**
		 * �w�肵���F�Ńr�b�g�}�b�v�S�̂����������܂�.
		 * @param color �K�p����F.
		 */
		void Clear(const ColorF &color);

		/// �r�b�g�}�b�v�̃R�s�[
		/**
		 * �w�肵���r�b�g�}�b�v��������R�s�[���܂�.
		 * 
		 * �e�r�b�g�}�b�v�͕��ƍ�������v���Ă��Ȃ���΂Ȃ�܂���.
		 * 
		 * @param src �R�s�[���ƂȂ� BitmapF �I�u�W�F�N�g.
		 * @exception FormatException �r�b�g�}�b�v�̕�����������v���܂���.
		 * @remarks �e�r�b�g�}�b�v�̕��ƍ����͈�v���Ă��Ȃ���΂Ȃ�܂���.
		 */
		void CopyFrom(const BitmapF &src);

		/// �r�b�g�}�b�v�̕����擾
		UInt32 Width() const { return width_; }
		/// �r�b�g�}�b�v�̍������擾
		UInt32 Height() const { return height_; }
		/// �f�[�^�̐擪�ւ̃|�C���^���擾
		float *Buffer() { return mem_; }
		/// �f�[�^�̐擪�ւ̃|�C���^���擾 (const)
		const float *Buffer() const { return mem_; }
	};
}

#endif
