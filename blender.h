// wakaba/blender.h
#ifndef WAKABA_BLENDER_H
#define WAKABA_BLENDER_H

#include "dllexport.h"
#include "basetype.h"
#include "bitmap.h"

namespace Wakaba {
	/// �r�b�g�}�b�v�̍�������
	class WAKABA_API Blender {
	public:
		/// �r�b�g�}�b�v�������Z�̎��
		enum Operation {
			/// �P���R�s�[
			/**
			 * result = src
			 * 
			 * ���̉��Z�ł� dest �̃A���t�@, ����� BlendInfo::alpha �͖�������܂�.
			 */
			OPERATION_COPY,
			/// dest ����ɂ��ăA���t�@����
			/**
			 * result = dest
			 * 
			 * ���̉��Z�ł� BlendInfo::alpha �� dest �ɐώZ����܂�.
			 */
			OPERATION_DESTALPHA,
			/// �A���t�@����
			/**
			 * result = src
			 * 
			 * Photoshop �́u�ʏ�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_SRCALPHA,
			/// ��Z
			/**
			 * result = dest * src
			 * 
			 * Photoshop �́u��Z�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_MULTIPLE,
			/// ���]���Z
			/**
			 * result = max(dest - (1 - src), 0)
			 * 
			 * Photoshop �́u�Ă�����(���j�A)�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_SUBTRACT,
			/// ��r���ĈÂ���
			/**
			 * result = min(dest, src)
			 * 
			 * Photoshop �́u��r(��)�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_LOWER,
			/// ���]��Z
			/**
			 * result = 1 - (1 - dest) * (1 - src)
			 * 
			 * Photoshop �́u�X�N���[���v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_SCREEN,
			/// ���Z
			/**
			 * result = min(dest + src, 1)
			 * 
			 * Photoshop �́u�����Ă�(���j�A)�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_ADD,
			/// ��r���Ė��邢��
			/**
			 * result = max(dest, src)
			 * 
			 * Photoshop �́u��r(��)�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_HIGHER,
			/// dest < 50% �ŏ�Z, dest > 50% �Ŕ��]��Z
			/**
			 * result = dest <= 0.5 ? 2 * dest * src : 1 - 2 * (1 - dest) * (1 - src)
			 * 
			 * Photoshop �́u�I�[�o�[���C�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_OVERLAY,
			/// ���̐�Βl
			/**
			 * result = abs(dest - src)
			 * 
			 * Photoshop �́u���̐�Βl�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_DIFFERENCE,
			/// �r���I�a
			/**
			 * result = (1 - dest) * src + dest * (1 - src)
			 * 
			 * Photoshop �́u���O�v�Ɠ��l�̉��Z�ł�.
			 * ���̉��Z�ł� BlendInfo::alpha �� src �ɐώZ����܂�.
			 */
			OPERATION_EXCLUSION
		};

		/// �摜�����̐ݒ���
		struct BlendInfo {
			/// dest �̍��� X ���W
			Int32 dest_x;
			/// dest �̍��� Y ���W
			Int32 dest_y;
			/// src �̍��� X ���W
			Int32 src_x;
			/// src �̍��� Y ���W
			Int32 src_y;
			/// �������s���̈�̕�
			Int32 width;
			/// �������s���̈�̍���
			Int32 height;
			/// �s�����x (0.0f ~ 1.0f)
			/**
			 * ���̒l��, ���Z�̎�ނɂ���� dest �� src �̕s�����x�Ɏ��O�ɐώZ����܂�.
			 */
			float alpha;
			/// ���Z�̎��
			Operation operation;
		};

		/// �r�b�g�}�b�v������
		/**
		 * 2 ���̃r�b�g�}�b�v���w�肵�����Z�ō������� 1 ���ɂ��܂�.
		 * @param dest �K�p��� BitmapF.
		 * @param src �������� BitmapF.
		 * @param info �ݒ����\�� Blender::BlendInfo.
		 * @exception ParameterException ��Ή��̉��Z���w�肳�ꂽ��, dest �� src �ɓ����I�u�W�F�N�g���w�肳��܂���.
		 */
		static void Blend(BitmapF &dest, const BitmapF &src, const BlendInfo &info);
	};
}

#endif
