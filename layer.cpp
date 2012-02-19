// layer.cpp
#include "layer.h"
#include <algorithm>

using namespace Wakaba;



/*--------------------------------------------------------------
	class Wakaba::Layer
--------------------------------------------------------------*/

// c-tor
Layer::Layer()
	: x_(0)
	, y_(0)
	, visible_(true)
	, alpha_(1.0f)
	, operation_(Blender::OPERATION_SRCALPHA)
{
}

// d-tor
Layer::~Layer()
{
}

// �I�u�W�F�N�g�̔j��
void Layer::Dispose()
{
	delete this;
}

// �A���t�@�l�̐ݒ�
void Layer::SetAlpha(float alpha)
{
	// �l�� 0.0f ~ 1.0f �ɗ}����
	alpha_
		= alpha > 1.0f ? 1.0f
		: alpha < 0.0f ? 0.0f
		: alpha;
}



/*--------------------------------------------------------------
	class Wakaba::LayerGroup
--------------------------------------------------------------*/

// c-tor
LayerGroup::LayerGroup()
	: children_(new std::list<Layer *>())
{
}

// d-tor
LayerGroup::~LayerGroup()
{
	// ���ׂĂ̎q���C���[��j������
	while (!children_->empty()) {
		Layer *child = children_->front();
		children_->pop_front();
		child->Dispose();
	}
	delete children_;
}

// ����
LayerGroup *LayerGroup::New()
{
	return new LayerGroup();
}

// �`��
void LayerGroup::Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const
{
	// �s���Ȃ�`�悵�Ȃ�
	if (!GetVisible())
		return;

	// �p�����[�^�̒���
	x -= GetX();
	y -= GetY();
	alpha *= GetAlpha();

	// �Ō���̃��C���[���珇�Ԃɍ�������
	std::list<Layer *>::const_reverse_iterator it;
	for(it = children_->rbegin(); it != children_->rend(); ++it)
		(*it)->Draw(dest, x, y, width, height, alpha);
}

// �q���C���[��ǉ�
void LayerGroup::AddChild(Layer *layer, UInt32 index)
{
	// �C���f�b�N�X�̊m�F
	// �Ō���ɒǉ�����ꍇ�� index == children_->size() �ƂȂ�̂�,
	// ���̊֐��Ƃ͔��肪�قȂ�.
	if (index > children_->size())
		throw OutOfRangeException();

	// ���ɒǉ�����Ă����玸�s
	if (std::find(children_->begin(), children_->end(), layer) != children_->end())
		throw ParameterException();

	std::list<Layer *>::iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	children_->insert(it, layer);
}

// �q���C���[�����o��
Layer *LayerGroup::RemoveChild(UInt32 index)
{
	// �C���f�b�N�X�̊m�F
	if (index >= children_->size())
		throw OutOfRangeException();

	std::list<Layer *>::iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	Layer *child = *it;
	children_->erase(it);
	return child;
}

// �q���C���[���擾
Layer &LayerGroup::Child(UInt32 index)
{
	// �C���f�b�N�X�̊m�F
	if (index >= children_->size())
		throw OutOfRangeException();

	std::list<Layer *>::const_iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	return **it;
}

// �q���C���[���擾 (const)
const Layer &LayerGroup::Child(UInt32 index) const
{
	// �C���f�b�N�X�̊m�F
	if (index >= children_->size())
		throw OutOfRangeException();

	std::list<Layer *>::const_iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	return **it;
}



/*--------------------------------------------------------------
	class Wakaba::BitmapLayer
--------------------------------------------------------------*/

// c-tor
BitmapLayer::BitmapLayer(UInt32 width, UInt32 height)
{
	try {
		bitmap_ = BitmapF::New(width, height);
	} catch (...) {
		throw;
	}
}

// d-tor
BitmapLayer::~BitmapLayer()
{
	bitmap_->Dispose();
}

// ����
BitmapLayer *BitmapLayer::New(UInt32 width, UInt32 height)
{
	return new BitmapLayer(width, height);
}

// �`��
void BitmapLayer::Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const
{
	// �s���Ȃ�`�悵�Ȃ�
	if (!GetVisible())
		return;

	// �r�b�g�}�b�v�̕`��
	Blender::BlendInfo info = {
		x, // dest_x
		y, // dest_y
		x-GetX(), // src_x
		y-GetY(), // src_y
		width,
		height,
		alpha*GetAlpha(),
		GetBlendMode()
	};
	Blender::Blend(dest, *bitmap_, info);
}
