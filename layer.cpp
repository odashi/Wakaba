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

// オブジェクトの破棄
void Layer::Dispose()
{
	delete this;
}

// アルファ値の設定
void Layer::SetAlpha(float alpha)
{
	// 値を 0.0f ~ 1.0f に抑える
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
	// すべての子レイヤーを破棄する
	while (!children_->empty()) {
		Layer *child = children_->front();
		children_->pop_front();
		child->Dispose();
	}
	delete children_;
}

// 生成
LayerGroup *LayerGroup::New()
{
	return new LayerGroup();
}

// 描画
void LayerGroup::Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const
{
	// 不可視なら描画しない
	if (!GetVisible())
		return;

	// パラメータの調整
	x -= GetX();
	y -= GetY();
	alpha *= GetAlpha();

	// 最後尾のレイヤーから順番に合成する
	std::list<Layer *>::const_reverse_iterator it;
	for(it = children_->rbegin(); it != children_->rend(); ++it)
		(*it)->Draw(dest, x, y, width, height, alpha);
}

// 子レイヤーを追加
void LayerGroup::AddChild(Layer *layer, UInt32 index)
{
	// インデックスの確認
	// 最後尾に追加する場合は index == children_->size() となるので,
	// 他の関数とは判定が異なる.
	if (index > children_->size())
		throw OutOfRangeException();

	// 既に追加されていたら失敗
	if (std::find(children_->begin(), children_->end(), layer) != children_->end())
		throw ParameterException();

	std::list<Layer *>::iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	children_->insert(it, layer);
}

// 子レイヤーを取り出す
Layer *LayerGroup::RemoveChild(UInt32 index)
{
	// インデックスの確認
	if (index >= children_->size())
		throw OutOfRangeException();

	std::list<Layer *>::iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	Layer *child = *it;
	children_->erase(it);
	return child;
}

// 子レイヤーを取得
Layer &LayerGroup::Child(UInt32 index)
{
	// インデックスの確認
	if (index >= children_->size())
		throw OutOfRangeException();

	std::list<Layer *>::const_iterator it = children_->begin();
	for (UInt32 i = 0; i < index; ++i)
		++it;
	return **it;
}

// 子レイヤーを取得 (const)
const Layer &LayerGroup::Child(UInt32 index) const
{
	// インデックスの確認
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

// 生成
BitmapLayer *BitmapLayer::New(UInt32 width, UInt32 height)
{
	return new BitmapLayer(width, height);
}

// 描画
void BitmapLayer::Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const
{
	// 不可視なら描画しない
	if (!GetVisible())
		return;

	// ビットマップの描画
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
