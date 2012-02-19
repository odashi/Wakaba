// wakaba/layer.h
#ifndef WAKABA_LAYER_H
#define WAKABA_LAYER_H

#include "dllexport.h"
#include "bitmap.h"
#include "blender.h"
#include "exception.h"
#include <list>

namespace Wakaba {
	/// レイヤーの基底クラス
	class WAKABA_API Layer {
		Int32 x_;
		Int32 y_;
		bool visible_;
		float alpha_;
		Blender::Operation operation_;

		// 禁止操作
		Layer(const Layer &);
		Layer &operator =(const Layer &);

	protected:
		Layer();
		virtual ~Layer();

	public:
		/// オブジェクトの破棄
		/**
		 * レイヤーオブジェクトを破棄します.
		 * @remarks この関数を呼び出すとオブジェクトが無効になります. 呼び出し後のオブジェクトを使用してはいけません.
		 * @remarks この関数は nullptr に対して呼び出すことができます. その場合は何も行いません.
		 */
		void Dispose();

		/// レイヤーの要素を描画
		/**
		 * 指定するビットマップへレイヤーの要素を描画します.
		 * @param dest レイヤーの要素を描画する BitmapF.
		 * @param x 描画範囲の左上 X 座標.
		 * @param y 描画範囲の左上 Y 座標.
		 * @param width 描画範囲の幅.
		 * @param height 描画範囲の高さ.
		 * @param alpha 描画する際の不透明度. 普通, この値には 1.0f を設定する.
		 */
		virtual void Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const = 0;

		/// 現在の X 座標を取得
		/**
		 * レイヤーの現在の X 方向オフセットを取得します.
		 * @retval Int32 現在の X 座標.
		 */
		Int32 GetX() const { return x_; }

		/// 現在の Y 座標を取得
		/**
		 * レイヤーの現在の Y 方向オフセットを取得します.
		 * @retval Int32 現在の Y 座標.
		 */
		Int32 GetY() const { return y_; }

		/// X 座標を設定
		/**
		 * レイヤーの新しい X 方向オフセットを設定します.
		 * @param x 新しい X 座標.
		 */
		void SetX(Int32 x) { x_ = x; }

		/// Y 座標を設定
		/**
		 * レイヤーの新しい Y 方向オフセットを設定します.
		 * @param y 新しい Y 座標.
		 */
		void SetY(Int32 y) { y_ = y; }

		/// X 座標, Y 座標を同時に設定
		/**
		 * レイヤーの新しい X 方向, Y 方向オフセットを設定します.
		 * @param x 新しい X 座標.
		 * @param y 新しい Y 座標.
		 */
		void SetPosition(Int32 x, Int32 y) { x_ = x, y_ = y; }

		/// 可視状態の取得
		/**
		 * レイヤーの現在の可視状態を取得します.
		 * @retval bool 現在の可視状態.
		 */
		bool GetVisible() const { return visible_; }

		/// 可視状態の設定
		/**
		 * レイヤーの新しい可視状態を設定します.
		 * @param visible 新しい可視状態.
		 */
		void SetVisible(bool visible) { visible_ = visible; }

		/// 不透明度の取得
		/**
		 * レイヤーの現在の不透明度を取得します.
		 * @retval float 現在の不透明度.
		 */
		float GetAlpha() const { return alpha_; }

		/// 不透明度の設定
		/**
		 * レイヤーの新しい不透明度を設定します.
		 * @param alpha 新しい不透明度.
		 * @remarks alpha の値として 0.0f 以下を設定しようとした場合は 0.0f に,
		 * 1.0f 以上を設定しようとした場合は 1.0f に切り詰められます.
		 */
		void SetAlpha(float alpha);

		/// ブレンドモードの取得
		/**
		 * レイヤーの現在のブレンドモード（レイヤー合成の演算の種類）を取得します.
		 * @retval Blender::Operation 現在のブレンドモード.
		 */
		Blender::Operation GetBlendMode() const { return operation_; }

		/// ブレンドモードの設定
		/**
		 * レイヤーの新しいブレンドモード（レイヤー合成の演算の種類）を設定します.
		 * @retval operation 新しいブレンドモードを表す Blender::Operation.
		 */
		void SetBlendMode(Blender::Operation operation) { operation_ = operation; }

		/// 書き込み可能なビットマップを保持しているかどうか
		/**
		 * レイヤーが書き込み可能なビットマップを保持しているかどうかを調べます.
		 * この関数で true が返った場合, GetBitmap() で BitmapF オブジェクトを取得することができます.
		 * @retval true 書き込み可能なビットマップを保持している.
		 * @retval false 書き込み可能なビットマップを保持していない.
		 */
		virtual bool HasBitmap() const = 0;

		/// 書き込み可能な BitmapF オブジェクトの取得
		/**
		 * レイヤー内のビットマップを取得します.
		 * @retval BitmapF& レイヤーの保持する BitmapF オブジェクト.
		 * @exception NotImplementedException ビットマップを保持していません.
		 * @remarks レイヤーがビットマップを保持しているかどうか不明の場合,
		 * この関数を呼び出す前に HasBitmap() によってビットマップを保持しているかどうかを調べる必要があります.
		 */
		virtual BitmapF &GetBitmap() = 0;

		/// BitmapF オブジェクトの取得 (const)
		/**
		 * レイヤー内のビットマップを取得します.
		 * @retval BitmapF& レイヤーの保持する BitmapF オブジェクト.
		 * @exception NotImplementedException このオブジェクトはビットマップを保持していません.
		 * @remarks レイヤーがビットマップを保持しているかどうか不明の場合,
		 * この関数を呼び出す前に HasBitmap() によってビットマップを保持しているかどうかを調べる必要があります.
		 */
		virtual const BitmapF &GetBitmap() const = 0;

		/// 子レイヤーの数
		/**
		 * このレイヤーが持つ子レイヤーの数を返します.
		 * @retval UInt32 子レイヤーの数.
		 */
		virtual UInt32 ChildNum() const = 0;

		/// 子レイヤーを追加できるかどうか
		/**
		 * このレイヤーが子レイヤーを持つことができるかどうかを調べます.
		 * この関数で true が返った場合, AddChild() で子レイヤーを追加することができます.
		 * @retval true 子レイヤーを持つことができる.
		 * @retval false 子レイヤーを持つことができない.
		 */
		virtual bool CanAddChild() const = 0;

		/// 子レイヤーを追加
		/**
		 * 子レイヤーを追加します.
		 * @param layer 追加する子レイヤーを表す Layer オブジェクト.
		 * @param index 子レイヤーを追加する位置
		 * @exception NotImplementedException このオブジェクトは子レイヤーを持つことができません.
		 * @exception OutOfRangeException index が範囲外です.
		 * @exception ParameterException layer で指定したレイヤーは追加することができません.
		 * @remarks index には 0 (最上部) から ChildNum() (最下部) の範囲の値を指定できます.
		 * @remarks この関数によって追加された子レイヤーは, 親レイヤーによって自動的に破棄されます.
		 * 子レイヤーを明示的に破棄してはいけません.
		 */
		virtual void AddChild(Layer *layer, UInt32 index) = 0;

		/// 子レイヤーを取り出す
		/**
		 * 指定した子レイヤーを親レイヤーから切り離して, そのレイヤーへのポインタを返します.
		 * @param index 子レイヤーの番号.
		 * @retval Layer* 取り出したレイヤーを表す Layer オブジェクト.
		 * @exception NotImplementedException このオブジェクトは子レイヤーを持つことができません.
		 * @exception OutOfRangeException index が範囲外です.
		 * @remarks index には 0 (最上部) から ChildNum() - 1 (最下部) の範囲の値を指定できます.
		 * @remarks この関数によって取り出されたレイヤーは, 親レイヤーによって自動的に破棄されなくなります.
		 * 取り出したレイヤーは Dispose() によって明示的に破棄する必要があります.
		 */
		virtual Layer *RemoveChild(UInt32 index) = 0;

		/// 子レイヤーを取得
		/**
		 * 指定した子レイヤーへの参照を取得します.
		 * @param index 子レイヤーの番号.
		 * @retval Layer& 指定した子レイヤーを表す Layer オブジェクト.
		 * @exception NotImplementedException このオブジェクトは子レイヤーを持つことができません.
		 * @exception OutOfRangeException index が範囲外です.
		 * @remarks index には 0 (最上部) から ChildNum() - 1 (最下部) の範囲の値を指定できます.
		 */
		virtual Layer &Child(UInt32 index) = 0;

		/// 子レイヤーを取得 (const)
		/**
		 * 指定した子レイヤーへの参照を取得します.
		 * @param index 子レイヤーの番号.
		 * @retval Layer& 指定した子レイヤーを表す Layer オブジェクト.
		 * @exception NotImplementedException このオブジェクトは子レイヤーを持つことができません.
		 * @exception OutOfRangeException index が範囲外です.
		 * @remarks index には 0 (最上部) から ChildNum() - 1 (最下部) の範囲の値を指定できます.
		 */
		virtual const Layer &Child(UInt32 index) const = 0;
	};

	/// レイヤーグループ
	class WAKABA_API LayerGroup : public Layer {
		std::list<Layer *> *children_;

		LayerGroup();
		~LayerGroup();

	public:
		/// レイヤーグループの作成
		/**
		 * 新しいレイヤーグループを作成します.
		 * @retval LayerGroup* 新しい LayerGroup オブジェクト.
		 */
		static LayerGroup *New();

		void Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const;

		bool HasBitmap() const { return false; }

		/**
		 * この関数を呼び出すことはできません.
		 * @exception NotImplementedException 関数は実装されていません.
		 */
		BitmapF &GetBitmap() { throw NotImplementedException(); }

		/**
		 * この関数を呼び出すことはできません.
		 * @exception NotImplementedException 関数は実装されていません.
		 */
		const BitmapF &GetBitmap() const { throw NotImplementedException(); }

		UInt32 ChildNum() const { return children_->size(); }
		bool CanAddChild() const { return true; }
		void AddChild(Layer *layer, UInt32 index);
		Layer *RemoveChild(UInt32 index);
		Layer &Child(UInt32 index);
		const Layer &Child(UInt32 index) const;
	};

	/// ビットマップを管理するレイヤー
	class WAKABA_API BitmapLayer : public Layer {
		BitmapF *bitmap_;

		BitmapLayer(UInt32 width, UInt32 height);
		~BitmapLayer();

	public:
		/// ビットマップレイヤーの作成
		/**
		 * 新しいビットマップレイヤーを作成します.
		 * @param width レイヤーの幅
		 * @param height レイヤーの高さ
		 * @retval BitmapLayer* 新しい BitmapLayer オブジェクト.
		 */
		static BitmapLayer *New(UInt32 width, UInt32 height);

		void Draw(BitmapF &dest, Int32 x, Int32 y, Int32 width, Int32 height, float alpha) const;

		bool HasBitmap() const { return true; }
		BitmapF &GetBitmap() { return *bitmap_; }
		const BitmapF &GetBitmap() const { return *bitmap_; }

		UInt32 ChildNum() const { return 0; }
		bool CanAddChild() const { return false; }

		/**
		 * この関数を呼び出すことはできません.
		 * @exception NotImplementedException 関数は実装されていません.
		 */
		void AddChild(Layer *, UInt32) { throw NotImplementedException(); }

		/**
		 * この関数を呼び出すことはできません.
		 * @exception NotImplementedException 関数は実装されていません.
		 */
		Layer *RemoveChild(UInt32) { throw NotImplementedException(); }

		/**
		 * この関数を呼び出すことはできません.
		 * @exception NotImplementedException 関数は実装されていません.
		 */
		Layer &Child(UInt32) { throw NotImplementedException(); }

		/**
		 * この関数を呼び出すことはできません.
		 * @exception NotImplementedException 関数は実装されていません.
		 */
		const Layer &Child(UInt32) const { throw NotImplementedException(); }
	};
}

#endif
