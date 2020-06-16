/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * DialogLayer.h
 *
 *  Created on: 2016. apr. 17.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGLAYER_H_
#define TEST_SAPPHIRE_DIALOGLAYER_H_

#include <framework/animation/Animation.h>
#include <framework/core/Window.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/render/RenderTarget.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/LinkedNode.h>
#include <framework/utils/utility.h>
#include <framework/io/touch/gesture/scroll/ScrollGestureDetector.h>
#include <gen/resources.h>
#include <appmain.h>
#include <sapphire/SapphireUILayer.h>

namespace rhfw {
class Scene;
class LayerGroup;
} // namespace rhfw

namespace userapp {
using namespace rhfw;

class DialogLayer;

class DialogItem: rhfw::LinkedNode<DialogItem> {
	friend class DialogLayer;
	bool highlightable = false;

	float lines = 0;
	float maxLines = 0.0f;

	rhfw::Rectangle rect;

protected:
	bool noHighlightRectDraw = false;
	float minLines;
	DialogLayer* dialog = nullptr;

	float getLines() const {
		return lines;
	}
	virtual void onHighlightRevoked() {
	}
	virtual void onHighlighted(Vector2F* pointer) {
	}
public:

	DialogItem(float minlines)
			: minLines(minlines) {
	}

	virtual void draw(const rhfw::Matrix2D& mvp) = 0;
	virtual float measureTextWidth(float textsize, float maxwidth) = 0;
	virtual float measureTextHeight(float textsize, float maxwidth) = 0;

	virtual void onTouch() {
	}
	virtual bool onKeyEvent() {
		return false;
	}
	virtual void onSelected(const Vector2F* pointer) {
	}

	virtual bool shouldInvalidate() {
		return false;
	}

	DialogItem* get() override {
		return this;
	}

	bool isHighlightable() const {
		return highlightable;
	}
	void setHighlightable(bool highlightable) {
		this->highlightable = highlightable;
	}

	const rhfw::Rectangle& getRect() const {
		return rect;
	}

};

class CommandDialogItem: public DialogItem {
protected:
	rhfw::FixedString text;

	class SelectHandler {
	public:
		virtual ~SelectHandler() = default;
		virtual void operator()() = 0;
	};
	SelectHandler* handler = nullptr;
	Color* textColor = nullptr;

protected:
	virtual void drawCommandImpl(const Matrix2D& mvp, const Color& textcolor);
public:
	template<typename Handler>
	CommandDialogItem(float weight, rhfw::FixedString text, Handler&& handler)
			: DialogItem(weight), text(rhfw::util::move(text)) {
		setHighlightable(true);
		setHandler(rhfw::util::forward<Handler>(handler));
	}
	CommandDialogItem(float weight, rhfw::FixedString text)
			: DialogItem(weight), text(rhfw::util::move(text)) {
		setHighlightable(true);
	}
	template<typename Handler>
	CommandDialogItem(rhfw::FixedString text, Handler&& handler)
			: DialogItem(1.0f), text(rhfw::util::move(text)) {
		setHighlightable(true);
		setHandler(rhfw::util::forward<Handler>(handler));
	}
	CommandDialogItem(rhfw::FixedString text)
			: DialogItem(1.0f), text(rhfw::util::move(text)) {
		setHighlightable(true);
	}
	CommandDialogItem(const CommandDialogItem&) = delete;
	CommandDialogItem& operator=(const CommandDialogItem&) = delete;
	~CommandDialogItem() {
		delete handler;
		delete textColor;
	}

	virtual void draw(const rhfw::Matrix2D& mvp) override;
	virtual float measureTextHeight(float textsize, float maxwidth) override;

	virtual float measureTextWidth(float textsize, float maxwidth) override;

	virtual void onSelected(const Vector2F* pointer) override {
		(*handler)();
	}

	void setText(FixedString text) {
		this->text = util::move(text);
	}

	template<typename Handler>
	void setHandler(Handler&& handler) {
		delete this->handler;
		class TemplatedHandler: public SelectHandler {
			Handler h;
		public:
			TemplatedHandler(Handler&& h)
					: h(rhfw::util::forward<Handler>(h)) {
			}
			virtual void operator()() override {
				h();
			}
		};
		this->handler = new TemplatedHandler { rhfw::util::forward<Handler>(handler) };
	}

	void setTextColor(const Color& color) {
		delete this->textColor;
		this->textColor = new Color(color);
	}
	void clearTextColor() {
		delete this->textColor;
		textColor = nullptr;
	}
};
class ValueCommandDialogItem: public CommandDialogItem {
	FixedString value;
protected:
	virtual void drawCommandImpl(const Matrix2D& mvp, const Color& textcolor) override;
public:
	using CommandDialogItem::CommandDialogItem;

	const FixedString& getValue() const {
		return value;
	}
	void setValue(FixedString value) {
		this->value = util::move(value);
	}

	virtual float measureTextWidth(float textsize, float maxwidth) override;
};

class DialogLayer: public SapphireUILayer {
	rhfw::Resource<rhfw::render::Texture> buffer;
	rhfw::Resource<rhfw::render::RenderTarget> target;

	rhfw::AutoResource<rhfw::render::Texture> backIconWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_arrow_back_white);

	rhfw::LifeCycleChain<rhfw::core::KeyEventListener, false> prevKeyboardTarget;

	rhfw::Rectangle targetPos;
	rhfw::FixedString title;
	rhfw::Rectangle backButtonPos;
	float titleTextSize = 0.0f;
	bool backPressed = false;

	rhfw::LinkedList<DialogItem> items;
	rhfw::LinkedList<DialogItem>::ring_iterator highlightedItem = items.ring();
	rhfw::LinkedList<DialogItem>::ring_iterator touchStartHighlight { items };
	float touchStartKeyboardOffset = 0.0f;

	rhfw::Size2F edgeSize;

	float sharedTextSize = 0.0f;

	Size2F basePadding;

	bool dirty = false;

	ScrollGestureDetector scrollDetector;

	Vector2F lastDrawScrollPosition;

	void redraw(const rhfw::Size2UI& pxsize);

	bool distributeLines(const Size2F& maxpxsize);

	float getSoftKeyboardOffset(const rhfw::LinkedList<DialogItem>::ring_iterator& highlighted);

	void animateToItem(DialogItem* item);

protected:
	rhfw::AutoResource<rhfw::Font> font;
	rhfw::core::WindowSize windowSize;

	void drawEdge(const rhfw::Matrix2D& mvp, const rhfw::Color& color);

	Size2F layoutItems();

	virtual void onGainingInput() override {
		SapphireUILayer::onGainingInput();
		invalidate();
	}
public:

	DialogLayer(SapphireUILayer * parent);
	~DialogLayer();

	float getSharedTextSize() const {
		return sharedTextSize;
	}

	void invalidate() {
		dirty = true;
	}

	void relayout();

	void finishRemoveDialogItem();
	void queueRemoveDialogItem(DialogItem* item);

	virtual void drawImpl(float displaypercent) override;

	void addDialogItem(DialogItem* item) {
		items.addToEnd(*item);
		item->dialog = this;
	}
	void addDialogItemFront(DialogItem* item) {
		items.addToStart(*item);
		item->dialog = this;
	}
	void addDialogItemAfter(DialogItem* relativeto, DialogItem* item);
	void addDialogItemBefore(DialogItem* relativeto, DialogItem* item);
	void removeDialogItem(DialogItem* item);

	void clearDialogItems();

	const rhfw::FixedString& getTitle() const {
		return title;
	}

	void setTitle(rhfw::FixedString title) {
		this->title = rhfw::util::move(title);
		invalidate();
	}

	bool isScrolling() const {
		return scrollDetector.getWorkingSize().width() - scrollDetector.getSize().width() > 0.5f
				|| scrollDetector.getWorkingSize().height() - scrollDetector.getSize().height() > 0.5f;
	}

	virtual bool touchImpl() override;

	virtual void dismiss() override;

	virtual void sizeChanged(const rhfw::core::WindowSize& size) override;

	virtual bool onKeyEventImpl() override;
	virtual void displayKeyboardSelection() override;
	void nextKeyboardSelection();
	void previousKeyboardSelection();
	virtual void hideKeyboardSelection() override;

	bool isHighlighted(DialogItem* item) {
		return *highlightedItem == item;
	}
	void setHighlighted(DialogItem* item) {
		this->highlightedItem = {items, item};
		invalidate();
	}
	void invalidateHighlight() {
		highlightedItem.invalidate();
	}
	bool hasHighlighted() const {
		return highlightedItem.isValid();
	}

	rhfw::Resource<rhfw::Font> getFont() const {
		return font;
	}

	const rhfw::Size2F& getEdgeSize() const {
		return edgeSize;
	}

	Size2F getSize() {
		return this->buffer->getSize();
	}

};

class TextDialogItem: public DialogItem {
private:
	static int countLines(const FixedString& text) {
		const char* start = text.begin();
		const char* end = text.end();
		const char* it;
		int lines = 0;
		for (it = start;; ++it) {
			if (*it == '\n') {
				++lines;
				start = it + 1;
			} else if (it == end) {
				if (start != it) {
					++lines;
				}
				break;
			}
		}
		return lines;
	}

	rhfw::FixedString text;
	float maxTextSizeCm;

	Color* textColor = nullptr;
public:
	TextDialogItem(rhfw::FixedString text, float maxtextsizecm = 1.0f)
			: DialogItem(1), maxTextSizeCm(maxtextsizecm) {
		setText(util::move(text));
	}
	~TextDialogItem() {
		delete textColor;
	}
	virtual float measureTextHeight(float textsize, float maxwidth) override;
	virtual float measureTextWidth(float textsize, float maxwidth) override;

	virtual void draw(const rhfw::Matrix2D& mvp) override;

	const rhfw::FixedString& getText() const {
		return text;
	}

	void setText(rhfw::FixedString text) {
		this->text = util::move(text);
		if (dialog != nullptr) {
			dialog->invalidate();
		}
	}

	void setTextColor(const Color& color) {
		delete this->textColor;
		this->textColor = new Color(color);
	}
	void clearTextColor() {
		delete this->textColor;
		textColor = nullptr;
	}
};

class TickDialogItem: public CommandDialogItem {
private:
	bool ticked;

	rhfw::AutoResource<rhfw::render::Texture> checkboxWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_check_box_white);
	rhfw::AutoResource<rhfw::render::Texture> checkboxOutlineWhite = getTexture(
			rhfw::ResIds::gameres::game_sapphire::art::ic_check_box_outline_white);

public:
	template<typename Handler>
	TickDialogItem(rhfw::FixedString text, bool ticked, Handler&& handler)
			: CommandDialogItem(rhfw::util::move(text), [this, handler] {
				this->ticked = !this->ticked;
				handler();
			}), ticked(ticked) {
	}
	TickDialogItem(rhfw::FixedString text, bool ticked)
			: CommandDialogItem(rhfw::util::move(text), [this] {
				this->ticked = !this->ticked;
			}), ticked(ticked) {
	}
	virtual void draw(const rhfw::Matrix2D& mvp) override {
		CommandDialogItem::draw(mvp);
		float textSize = dialog->getSharedTextSize();
		const bool highlighted = dialog->isHighlighted(this);
		Color color = highlighted ? dialog->getUiSelectedColor() : dialog->getUiColor();
		const float padding = (getRect().height() - textSize) / 2.0f;

		drawSapphireTexture(mvp, ticked ? checkboxWhite : checkboxOutlineWhite, color,
				Rectangle { getRect().right - textSize / 2.0f - textSize * 2 / 3, getRect().top + padding + textSize / 6, getRect().right
						- textSize / 2.0f, getRect().bottom - padding - textSize / 6 }, Rectangle { 0, 0, 1, 1 });
	}
	bool isTicked() const {
		return ticked;
	}
	virtual float measureTextWidth(float textsize, float maxwidth) override {
		return CommandDialogItem::measureTextWidth(textsize, maxwidth) + textsize * 3;
	}
};

} // namespace userapp

#include <sapphire/dialogs/items/EmptyDialogItem.h>
#include <sapphire/dialogs/items/EnumDialogItem.h>

#endif /* TEST_SAPPHIRE_DIALOGLAYER_H_ */
