// wakaba/imagefile.h
#ifndef WAKABA_IMAGEFILE_H
#define WAKABA_IMAGEFILE_H

#include "dllexport.h"
#include "bitmap.h"
#include <string>

namespace Wakaba {
	/// �摜�t�@�C���̓��o�͏���
	class WAKABA_API ImageFile {
		// �t�H�[�}�b�g���Ƃ̓ǂݍ��݊֐�
		static Bitmap16 *LoadBMP(const std::string &filepath);
		static Bitmap16 *LoadPNG(const std::string &filepath);
		static Bitmap16 *LoadJPEG(const std::string &filepath);

		// �t�H�[�}�b�g���Ƃ̏����o���֐�
		static void SaveBMP(const Bitmap16 &bitmap, const std::string &filepath);
		static void SavePNG(const Bitmap16 &bitmap, const std::string &filepath);
		static void SaveJPEG(const Bitmap16 &bitmap, const std::string &filepath);
	public:
		/// �t�@�C������摜��ǂݍ���
		/**
		 * �摜�t�@�C������r�b�g�}�b�v�𐶐����܂�.
		 * 
		 * �t�@�C���t�H�[�}�b�g�͎����I�ɔ��ʂ���܂�.
		 * 
		 * @param filepath �ǂݍ��ޑΏۂ̃t�@�C���p�X.
		 * @retval Bitmap16* �摜�����i�[���� Bitmap16.
		 * @exception IOException �t�@�C������Ɏ��s���܂���.
		 * @exception FormatException ��Ή��̃t�@�C���t�H�[�}�b�g��, �f�[�^���s���ł�.
		 * @exception InternalException ���������ŃG���[���������܂���.
		 * @remarks ���݂��̊֐����ǂݍ��߂�͈̂ȉ��̃t�@�C���t�H�[�}�b�g�ł�.
		 * - BMP(Windows/OS2, 16bpp�ȊO�̔񈳏k�`��)
		 * - PNG
		 * - JPEG(JFIF/Exif)
		 */
		static Bitmap16 *Load(const std::string &filepath);

		/// �t�@�C���։摜�������o��
		/**
		 * �r�b�g�}�b�v���w�肵���t�H�[�}�b�g�̉摜�t�@�C���֏����o���܂�.
		 * 
		 * @param bitmap �摜���.
		 * @param filepath �����o���Ώۂ̃t�@�C���p�X.
		 * @param format �t�@�C���t�H�[�}�b�g (remarks �Q��).
		 * @exception IOException �t�@�C������Ɏ��s���܂���.
		 * @exception FormatException ��Ή��̃t�@�C���t�H�[�}�b�g���w�肳��܂���.
		 * @exception InternalException ���������ŃG���[���������܂���.
		 * @remarks ���݂��̊֐����Ή����Ă���t�@�C���t�H�[�}�b�g, ����уt�H�[�}�b�g�w��̂��߂� format �̒l�͈ȉ��̒ʂ�ł�.
		 * - BMP(Windows, 24bpp) �c�c "BMP"
		 * - PNG(RGBA 32bpp) �c�c "PNG"
		 * - JPEG(JFIF) �c�c "JPEG" "JPG"
		 * @remarks format �̒l�͑啶��, ��������₢�܂���.
		 */
		static void Save(const Bitmap16 &bitmap, const std::string &filepath, const std::string &format);
	};
}

#endif
