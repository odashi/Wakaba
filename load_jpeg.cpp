// wakaba/load_jpeg.cpp
#include "imagefile.h"
#include "exception.h"
#include <jpeg/jpeglib.h>
#include <cstdio>
#include <csetjmp>

using namespace Wakaba;

namespace {
	// �Ǝ���`�̂̃G���[�}�l�[�W��
	struct MyErrorMgr {
		jpeg_error_mgr pub;
		std::jmp_buf setjmp_buffer;
	};

	// �Ǝ���`�̃G���[�n���h��
	void MyErrorExit(j_common_ptr cinfo)
	{
		// �������Ȃ��Ń��^�[���A�h���X�܂Ŗ߂�
		std::longjmp(((MyErrorMgr *)cinfo->err)->setjmp_buffer, 1);
	}
}

// libjpeg�ɂ��JPEG�`���̓ǂݍ���
Bitmap16 *ImageFile::LoadJPEG(const std::string &filepath)
{
	::jpeg_decompress_struct cinfo;
	::MyErrorMgr jerr;
	::JSAMPLE **scanline;
	
	Bitmap16 *bitmap;
	
	// �t�@�C�����J��
	std::FILE *fp = std::fopen(filepath.c_str(), "rb");
	if (!fp)
		throw IOException();
	
	try {
		// setjmp
#pragma warning(push)
#pragma warning(disable:4611)
		if (::setjmp(jerr.setjmp_buffer))
			throw FormatException();
#pragma warning(pop)

		// jpeg���C�u����������
		cinfo.err = ::jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = ::MyErrorExit;
		::jpeg_create_decompress(&cinfo);

		// �t�@�C���ݒ�
		::jpeg_stdio_src(&cinfo, fp);
	
		// �w�b�_�ǂݍ���
		::jpeg_read_header(&cinfo, TRUE);
	
		// �r�b�g�}�b�v����
		bitmap = Bitmap16::New(cinfo.image_width, cinfo.image_height);
			
		// �����̊J�n
		Int32 scanline_size = cinfo.num_components * cinfo.image_width;
		RawColor16 *buf = reinterpret_cast<RawColor16 *>(bitmap->Buffer());
		::jpeg_start_decompress(&cinfo);
		scanline = (*cinfo.mem->alloc_sarray)((::j_common_ptr)&cinfo, JPOOL_IMAGE, scanline_size, 1);

		switch (cinfo.out_color_components) {
		case 1: // Gray-scale JPEG
			while (cinfo.output_scanline < cinfo.output_height) {
				// �L��
				::jpeg_read_scanlines(&cinfo, scanline, 1);
				
				// �X�L�������C���̓ǂݍ���
				for (::JDIMENSION x = 0; x < cinfo.output_width; ++x, ++buf) {
					register UInt16 k = scanline[0][x];
					buf->b = buf->g = buf->r = (k << 8) + k;
					buf->a = 0xffff;
				}
			}
			break;
		case 3: // RGB JPEG
			while (cinfo.output_scanline < cinfo.output_height) {
				// �L��
				::jpeg_read_scanlines(&cinfo, scanline, 1);
				
				// �X�L�������C���̓ǂݍ���
				::JSAMPLE *s = scanline[0];
				for (::JDIMENSION x = 0; x < cinfo.output_width; ++x, ++buf) {
					buf->r = (*s << 8) + *s, ++s;
					buf->g = (*s << 8) + *s, ++s;
					buf->b = (*s << 8) + *s, ++s;
					buf->a = 0xffff;
				}
			}
			break;
		default: // ���m��JPEG
			throw FormatException();	
		}
	} catch (...) {
		::jpeg_destroy_decompress(&cinfo);
		if (bitmap)
			bitmap->Dispose();
		std::fclose(fp);
		throw;
	}
		
	// �ǂݍ��ݏI��
	::jpeg_finish_decompress(&cinfo);
	::jpeg_destroy_decompress(&cinfo);
	std::fclose(fp);
	return bitmap;
}
