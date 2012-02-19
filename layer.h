// wakaba/layer.h
#ifndef WAKABA_LAYER_H
#define WAKABA_LAYER_H

#include "dllexport.h"
#include "bitmap.h"
#include "blender.h"
#include "exception.h"
#include <list>

namespace Wakaba {
	/// ���C���[�̊��N���X
	class WAKABA_API Layer {
		Int32 x_;
		Int32 y_;
		bool visible_;
		float alpha_;
		Blender::Operation operation_;

		// �֎~����
		Layer(const Layer &);
		Layer &operator =(const Layer &);

	protected:
		Layer();
		virtual ~Layer();

	public:
		/// �I�u�W�F�N�g�̔j��
		/**
		 * ���C���[�I�u�W�F�N�g��j�����܂�.
		 * @remarks ���̊֐����Ăяo���ƃI�u�W�F�N�g�������ɂȂ�܂�. �Ăяo����̃I�u�W�F�N�g���g�p���Ă͂����܂���.
		 * @remarks ���̊֐��� nullptr �ɑ΂��ČĂяo�����Ƃ��ł��܂�. ���̏ꍇ�͉����s���܂���.
		 */
		void Dispose();

		/// ���C���[�̗v�f��`��
		/**
		 * �w�肷��r�b�g�}�b�v�փ��C���[�̗v�f��`�悵�܂�.
		 * @param dest ���C���[�̗v�f��`�悷�� BitmapF.
		 * @param x �`��͈͂̍��� X ���W.
		 * @param y �`��͈͂̍��� Y ���W.
		 * @param width �`��͈͂̕�.
		 * @param height �`��͈͂̍���.
		 * @param alpha �`�悷��ۂ̕s�����x. ����, ���̒l�ɂ� 1.0f ��ݒ肷��.
		 */
		virtual void Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const = 0;

		/// ���݂� X ���W���擾
		/**
		 * ���C���[�̌��݂� X �����I�t�Z�b�g���擾���܂�.
		 * @retval Int32 ���݂� X ���W.
		 */
		Int32 GetX() const { return x_; }

		/// ���݂� Y ���W���擾
		/**
		 * ���C���[�̌��݂� Y �����I�t�Z�b�g���擾���܂�.
		 * @retval Int32 ���݂� Y ���W.
		 */
		Int32 GetY() const { return y_; }

		/// X ���W��ݒ�
		/**
		 * ���C���[�̐V���� X �����I�t�Z�b�g��ݒ肵�܂�.
		 * @param x �V���� X ���W.
		 */
		void SetX(Int32 x) { x_ = x; }

		/// Y ���W��ݒ�
		/**
		 * ���C���[�̐V���� Y �����I�t�Z�b�g��ݒ肵�܂�.
		 * @param y �V���� Y ���W.
		 */
		void SetY(Int32 y) { y_ = y; }

		/// X ���W, Y ���W�𓯎��ɐݒ�
		/**
		 * ���C���[�̐V���� X ����, Y �����I�t�Z�b�g��ݒ肵�܂�.
		 * @param x �V���� X ���W.
		 * @param y �V���� Y ���W.
		 */
		void SetPosition(Int32 x, Int32 y) { x_ = x, y_ = y; }

		/// ����Ԃ̎擾
		/**
		 * ���C���[�̌��݂̉���Ԃ��擾���܂�.
		 * @retval bool ���݂̉����.
		 */
		bool GetVisible() const { return visible_; }

		/// ����Ԃ̐ݒ�
		/**
		 * ���C���[�̐V��������Ԃ�ݒ肵�܂�.
		 * @param visible �V���������.
		 */
		void SetVisible(bool visible) { visible_ = visible; }

		/// �s�����x�̎擾
		/**
		 * ���C���[�̌��݂̕s�����x���擾���܂�.
		 * @retval float ���݂̕s�����x.
		 */
		float GetAlpha() const { return alpha_; }

		/// �s�����x�̐ݒ�
		/**
		 * ���C���[�̐V�����s�����x��ݒ肵�܂�.
		 * @param alpha �V�����s�����x.
		 * @remarks alpha �̒l�Ƃ��� 0.0f �ȉ���ݒ肵�悤�Ƃ����ꍇ�� 0.0f ��,
		 * 1.0f �ȏ��ݒ肵�悤�Ƃ����ꍇ�� 1.0f �ɐ؂�l�߂��܂�.
		 */
		void SetAlpha(float alpha);

		/// �u�����h���[�h�̎擾
		/**
		 * ���C���[�̌��݂̃u�����h���[�h�i���C���[�����̉��Z�̎�ށj���擾���܂�.
		 * @retval Blender::Operation ���݂̃u�����h���[�h.
		 */
		Blender::Operation GetBlendMode() const { return operation_; }

		/// �u�����h���[�h�̐ݒ�
		/**
		 * ���C���[�̐V�����u�����h���[�h�i���C���[�����̉��Z�̎�ށj��ݒ肵�܂�.
		 * @retval operation �V�����u�����h���[�h��\�� Blender::Operation.
		 */
		void SetBlendMode(Blender::Operation operation) { operation_ = operation; }

		/// �������݉\�ȃr�b�g�}�b�v��ێ����Ă��邩�ǂ���
		/**
		 * ���C���[���������݉\�ȃr�b�g�}�b�v��ێ����Ă��邩�ǂ����𒲂ׂ܂�.
		 * ���̊֐��� true ���Ԃ����ꍇ, GetBitmap() �� BitmapF �I�u�W�F�N�g���擾���邱�Ƃ��ł��܂�.
		 * @retval true �������݉\�ȃr�b�g�}�b�v��ێ����Ă���.
		 * @retval false �������݉\�ȃr�b�g�}�b�v��ێ����Ă��Ȃ�.
		 */
		virtual bool HasBitmap() const = 0;

		/// �������݉\�� BitmapF �I�u�W�F�N�g�̎擾
		/**
		 * ���C���[���̃r�b�g�}�b�v���擾���܂�.
		 * @retval BitmapF& ���C���[�̕ێ����� BitmapF �I�u�W�F�N�g.
		 * @exception NotImplementedException �r�b�g�}�b�v��ێ����Ă��܂���.
		 * @remarks ���C���[���r�b�g�}�b�v��ێ����Ă��邩�ǂ����s���̏ꍇ,
		 * ���̊֐����Ăяo���O�� HasBitmap() �ɂ���ăr�b�g�}�b�v��ێ����Ă��邩�ǂ����𒲂ׂ�K�v������܂�.
		 */
		virtual BitmapF &GetBitmap() = 0;

		/// BitmapF �I�u�W�F�N�g�̎擾 (const)
		/**
		 * ���C���[���̃r�b�g�}�b�v���擾���܂�.
		 * @retval BitmapF& ���C���[�̕ێ����� BitmapF �I�u�W�F�N�g.
		 * @exception NotImplementedException ���̃I�u�W�F�N�g�̓r�b�g�}�b�v��ێ����Ă��܂���.
		 * @remarks ���C���[���r�b�g�}�b�v��ێ����Ă��邩�ǂ����s���̏ꍇ,
		 * ���̊֐����Ăяo���O�� HasBitmap() �ɂ���ăr�b�g�}�b�v��ێ����Ă��邩�ǂ����𒲂ׂ�K�v������܂�.
		 */
		virtual const BitmapF &GetBitmap() const = 0;

		/// �q���C���[�̐�
		/**
		 * ���̃��C���[�����q���C���[�̐���Ԃ��܂�.
		 * @retval UInt32 �q���C���[�̐�.
		 */
		virtual UInt32 ChildNum() const = 0;

		/// �q���C���[��ǉ��ł��邩�ǂ���
		/**
		 * ���̃��C���[���q���C���[�������Ƃ��ł��邩�ǂ����𒲂ׂ܂�.
		 * ���̊֐��� true ���Ԃ����ꍇ, AddChild() �Ŏq���C���[��ǉ����邱�Ƃ��ł��܂�.
		 * @retval true �q���C���[�������Ƃ��ł���.
		 * @retval false �q���C���[�������Ƃ��ł��Ȃ�.
		 */
		virtual bool CanAddChild() const = 0;

		/// �q���C���[��ǉ�
		/**
		 * �q���C���[��ǉ����܂�.
		 * @param layer �ǉ�����q���C���[��\�� Layer �I�u�W�F�N�g.
		 * @param index �q���C���[��ǉ�����ʒu
		 * @exception NotImplementedException ���̃I�u�W�F�N�g�͎q���C���[�������Ƃ��ł��܂���.
		 * @exception OutOfRangeException index ���͈͊O�ł�.
		 * @exception ParameterException layer �Ŏw�肵�����C���[�͒ǉ����邱�Ƃ��ł��܂���.
		 * @remarks index �ɂ� 0 (�ŏ㕔) ���� ChildNum() (�ŉ���) �͈̔͂̒l���w��ł��܂�.
		 * @remarks ���̊֐��ɂ���Ēǉ����ꂽ�q���C���[��, �e���C���[�ɂ���Ď����I�ɔj������܂�.
		 * �q���C���[�𖾎��I�ɔj�����Ă͂����܂���.
		 */
		virtual void AddChild(Layer *layer, UInt32 index) = 0;

		/// �q���C���[�����o��
		/**
		 * �w�肵���q���C���[��e���C���[����؂藣����, ���̃��C���[�ւ̃|�C���^��Ԃ��܂�.
		 * @param index �q���C���[�̔ԍ�.
		 * @retval Layer* ���o�������C���[��\�� Layer �I�u�W�F�N�g.
		 * @exception NotImplementedException ���̃I�u�W�F�N�g�͎q���C���[�������Ƃ��ł��܂���.
		 * @exception OutOfRangeException index ���͈͊O�ł�.
		 * @remarks index �ɂ� 0 (�ŏ㕔) ���� ChildNum() - 1 (�ŉ���) �͈̔͂̒l���w��ł��܂�.
		 * @remarks ���̊֐��ɂ���Ď��o���ꂽ���C���[��, �e���C���[�ɂ���Ď����I�ɔj������Ȃ��Ȃ�܂�.
		 * ���o�������C���[�� Dispose() �ɂ���Ė����I�ɔj������K�v������܂�.
		 */
		virtual Layer *RemoveChild(UInt32 index) = 0;

		/// �q���C���[���擾
		/**
		 * �w�肵���q���C���[�ւ̎Q�Ƃ��擾���܂�.
		 * @param index �q���C���[�̔ԍ�.
		 * @retval Layer& �w�肵���q���C���[��\�� Layer �I�u�W�F�N�g.
		 * @exception NotImplementedException ���̃I�u�W�F�N�g�͎q���C���[�������Ƃ��ł��܂���.
		 * @exception OutOfRangeException index ���͈͊O�ł�.
		 * @remarks index �ɂ� 0 (�ŏ㕔) ���� ChildNum() - 1 (�ŉ���) �͈̔͂̒l���w��ł��܂�.
		 */
		virtual Layer &Child(UInt32 index) = 0;

		/// �q���C���[���擾 (const)
		/**
		 * �w�肵���q���C���[�ւ̎Q�Ƃ��擾���܂�.
		 * @param index �q���C���[�̔ԍ�.
		 * @retval Layer& �w�肵���q���C���[��\�� Layer �I�u�W�F�N�g.
		 * @exception NotImplementedException ���̃I�u�W�F�N�g�͎q���C���[�������Ƃ��ł��܂���.
		 * @exception OutOfRangeException index ���͈͊O�ł�.
		 * @remarks index �ɂ� 0 (�ŏ㕔) ���� ChildNum() - 1 (�ŉ���) �͈̔͂̒l���w��ł��܂�.
		 */
		virtual const Layer &Child(UInt32 index) const = 0;
	};

	/// ���C���[�O���[�v
	class WAKABA_API LayerGroup : public Layer {
		std::list<Layer *> *children_;

		LayerGroup();
		~LayerGroup();

	public:
		/// ���C���[�O���[�v�̍쐬
		/**
		 * �V�������C���[�O���[�v���쐬���܂�.
		 * @retval LayerGroup* �V���� LayerGroup �I�u�W�F�N�g.
		 */
		static LayerGroup *New();

		void Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const;

		bool HasBitmap() const { return false; }

		/**
		 * ���̊֐����Ăяo�����Ƃ͂ł��܂���.
		 * @exception NotImplementedException �֐��͎�������Ă��܂���.
		 */
		BitmapF &GetBitmap() { throw NotImplementedException(); }

		/**
		 * ���̊֐����Ăяo�����Ƃ͂ł��܂���.
		 * @exception NotImplementedException �֐��͎�������Ă��܂���.
		 */
		const BitmapF &GetBitmap() const { throw NotImplementedException(); }

		UInt32 ChildNum() const { return children_->size(); }
		bool CanAddChild() const { return true; }
		void AddChild(Layer *layer, UInt32 index);
		Layer *RemoveChild(UInt32 index);
		Layer &Child(UInt32 index);
		const Layer &Child(UInt32 index) const;
	};

	/// �r�b�g�}�b�v���Ǘ����郌�C���[
	class WAKABA_API BitmapLayer : public Layer {
		BitmapF *bitmap_;

		BitmapLayer(UInt32 width, UInt32 height);
		~BitmapLayer();

	public:
		/// �r�b�g�}�b�v���C���[�̍쐬
		/**
		 * �V�����r�b�g�}�b�v���C���[���쐬���܂�.
		 * @param width ���C���[�̕�
		 * @param height ���C���[�̍���
		 * @retval BitmapLayer* �V���� BitmapLayer �I�u�W�F�N�g.
		 */
		static BitmapLayer *New(UInt32 width, UInt32 height);

		void Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const;

		bool HasBitmap() const { return true; }
		BitmapF &GetBitmap() { return *bitmap_; }
		const BitmapF &GetBitmap() const { return *bitmap_; }

		UInt32 ChildNum() const { return 0; }
		bool CanAddChild() const { return false; }

		/**
		 * ���̊֐����Ăяo�����Ƃ͂ł��܂���.
		 * @exception NotImplementedException �֐��͎�������Ă��܂���.
		 */
		void AddChild(Layer *, UInt32) { throw NotImplementedException(); }

		/**
		 * ���̊֐����Ăяo�����Ƃ͂ł��܂���.
		 * @exception NotImplementedException �֐��͎�������Ă��܂���.
		 */
		Layer *RemoveChild(UInt32) { throw NotImplementedException(); }

		/**
		 * ���̊֐����Ăяo�����Ƃ͂ł��܂���.
		 * @exception NotImplementedException �֐��͎�������Ă��܂���.
		 */
		Layer &Child(UInt32) { throw NotImplementedException(); }

		/**
		 * ���̊֐����Ăяo�����Ƃ͂ł��܂���.
		 * @exception NotImplementedException �֐��͎�������Ă��܂���.
		 */
		const Layer &Child(UInt32) const { throw NotImplementedException(); }
	};
}

#endif
