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
 * DialogLayer.cpp
 *
 *  Created on: 2016. apr. 17.
 *      Author: sipka
 */

#include <framework/animation/Animation.h>
#include <framework/animation/PropertyAnimator.h>
#include <framework/core/timing.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/render/Renderer.h>
#include <gen/types.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <appmain.h>
#include <sapphire/SapphireScene.h>

using namespace rhfw;

namespace userapp {

DialogLayer::DialogLayer(SapphireUILayer* parent)
		: SapphireUILayer(parent), font { ::userapp::getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf) } {
	buffer = renderer->createTexture();
	target = renderer->createRenderTarget();
	render::RenderTargetDescriptor desc;
	desc.setColorTarget(buffer);
	target->setDescriptor(desc);
}
DialogLayer::~DialogLayer() {
	target.freeIfLoaded();
	buffer.freeIfLoaded();
}

float DialogLayer::getSoftKeyboardOffset(const rhfw::LinkedList<DialogItem>::ring_iterator& highlighted) {
	if (getScene()->getWindow()->isSoftKeyboardShowing() && highlighted) {
		//translate a bit top
		auto* hl = highlighted->get();
		auto& rect = hl->getRect();
		if (targetPos.top + rect.bottom > windowSize.pixelSize.height() / 3.0f) {
			return -rect.bottom - targetPos.top + (windowSize.pixelSize.height() / 3.0f);
		}
	}
	return 0.0f;
}

void DialogLayer::drawImpl(float displayPercent) {
	float percent = displayPercent;
	float diff = (1 - percent) * (1 - percent) * 0.2f;
	float alpha = percent * percent;
	auto mvp =
			Matrix2D { }.setTranslate(0, touchStartHighlight ? touchStartKeyboardOffset : getSoftKeyboardOffset(highlightedItem)).multScreenDimension(
					windowSize.pixelSize).multScale(1 - diff, 1 - diff);

	bool shouldinvalidate = dirty || scrollDetector.getPosition() != lastDrawScrollPosition;
	if (!shouldinvalidate) {
		for (auto&& i : items.objects()) {
			//should invalidate?
			if (i.shouldInvalidate()) {
				shouldinvalidate = true;
				break;
			}
		}
	}
	if (shouldinvalidate) {
		if (buffer->getInputSource() != nullptr) {
			this->redraw(buffer->getSize());
		}
		renderer->resetViewPort();
		dirty = false;
	}

	renderer->setDepthTest(false);
	renderer->initDraw();

	Color colormult { 1.0f, 1.0f, 1.0f, alpha };

	static const float DIM_VALUE = 0.8f;
	colormult.xyz() -= this->getDimPercent() * DIM_VALUE;

	drawSapphireTexture(mvp, buffer, colormult, targetPos, Rectangle { 0, 0, 1, 1 });
}

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
bool DialogLayer::onKeyEventImpl() {
	if (highlightedItem) {
		auto* highlighted = highlightedItem->get();
		if (highlighted->onKeyEvent()) {
			if (highlightedItem && highlightedItem->get() == highlighted) {
				animateToItem(highlighted);
			}
			return true;
		}
	}
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
		case KeyCode::KEY_DIR_DOWN: {
			BREAK_ON_NOT_DOWN();
			if (isScrolling() && (KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
				scrollDetector.applyWheel(-1.0f);
			} else {
				nextKeyboardSelection();
				if(!highlightedItem.isValid()) {
					//no next keyboard selection
					//scroll instead
					scrollDetector.applyWheel(-1.0f);
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_UP:
		case KeyCode::KEY_DIR_UP: {
			BREAK_ON_NOT_DOWN();
			if (isScrolling() && (KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
				scrollDetector.applyWheel(1.0f);
			} else {
				previousKeyboardSelection();
				if(!highlightedItem.isValid()) {
					//no next keyboard selection
					//scroll instead
					scrollDetector.applyWheel(1.0f);
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_A:
		case KeyCode::KEY_ENTER: {
			BREAK_ON_NOT_DOWN();
			if (highlightedItem) {
				highlightedItem->get()->onSelected(nullptr);
				invalidate();
			}
			break;
		}
		case KeyCode::KEY_END: {
			BREAK_ON_NOT_DOWN();
			if (isScrolling() && (KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
				scrollDetector.animateTo(scrollDetector.getScrollRange().rightBottom());
			} else {
				if(highlightedItem) {
					highlightedItem->get()->onHighlightRevoked();
					highlightedItem.invalidate();
				}
				previousKeyboardSelection();
			}
			break;
		}
		case KeyCode::KEY_HOME: {
			BREAK_ON_NOT_DOWN();
			if (isScrolling() && (KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
				scrollDetector.animateTo(scrollDetector.getScrollRange().leftTop());
			} else {
				if(highlightedItem) {
					highlightedItem->get()->onHighlightRevoked();
					highlightedItem.invalidate();
				}
				nextKeyboardSelection();
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER:
		case KeyCode::KEY_PAGE_DOWN: {
			BREAK_ON_NOT_DOWN();
			if (isScrolling()) {
				scrollDetector.animateTo(scrollDetector.getPosition() - scrollDetector.getSize());
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_LEFT_SHOULDER:
		case KeyCode::KEY_PAGE_UP: {
			BREAK_ON_NOT_DOWN();
			if (isScrolling()) {
				scrollDetector.animateTo(scrollDetector.getPosition() + scrollDetector.getSize());
			}
			break;
		}
		default: {
			return SapphireUILayer::onKeyEventImpl();
		}
	}
	return true;
}

bool DialogLayer::distributeLines(const Size2F& maxpxsize) {
	int remaincount = 0;
	for (auto&& i : items.objects()) {
		i.maxLines = i.measureTextHeight(sharedTextSize, maxpxsize.width() - basePadding.width());
		i.lines = i.maxLines;
//		float remain = i.maxLines - i.lines;
//		if (remain > 0.05f && remain >= 1.0f) {
//			++remaincount;
//			if (remain < minremain) {
//				minremain = remain;
//			}
//		}
	}

	//use some threshold here, because we are dealing with floating points
//	while (remaincount > 0) {
//		float nminremain = linecount;
//		int oldremaincount = remaincount;
//		remaincount = 0;
//		for (auto&& i : items.objects()) {
//			float remain = i.maxLines - i.lines;
//			if (remain >= 1.0f) {
//				i.lines += 1.0f;
//				if (remain > 0.05f && remain >= 1.0f) {
//					++remaincount;
//					if (remain < nminremain) {
//						nminremain = remain;
//					}
//				}
//			}
//		}
//		minremain = nminremain;
//	}
//	return remaincount > 0;
	return false;
}

void DialogLayer::sizeChanged(const rhfw::core::WindowSize& size) {
	this->windowSize = size;

	relayout();
	if (highlightedItem) {
		animateToItem(highlightedItem->get());
	}
}
Size2F DialogLayer::layoutItems() {
	//TODO really show redo the entire dialog system
	const rhfw::core::WindowSize& size = windowSize;

	Vector2F bufsize = size.getPhysicalSize();
	bufsize *= 0.85f;
	Size2F pxsize = size.toPixels(bufsize);
	Size2F maxpxsize = size.toPixels(bufsize);
	float edgecm = min(min(size.getPhysicalSize().width(), size.getPhysicalSize().height()) / 300, 0.07f);
	edgeSize = windowSize.toPixels(Vector2F { edgecm, edgecm });

	float titleheight = windowSize.toPixelsY(1.3f);
	if (titleheight > pxsize.y() / 10.0f) {
		titleheight = pxsize.y() / 10.0f;
	}

	backButtonPos = Rectangle { edgeSize, edgeSize + titleheight };

	auto totaltitleheight = titleheight + edgeSize.height();

//	auto totalcontentheight = pxsize.height() - totaltitleheight - edgeSize.height();

	titleTextSize = titleheight;

	float measuredtitlew = getFont()->measureText(title, titleTextSize);

	float maxwidth = measuredtitlew + edgeSize.width() * 2 + backButtonPos.width() + windowSize.toPixelsX(0.5);
	if (maxwidth > pxsize.width()) {
		titleTextSize = titleTextSize / (measuredtitlew + maxwidth - pxsize.width()) * measuredtitlew;
		maxwidth = pxsize.width();
	}

	sharedTextSize = titleTextSize * 0.8f;

	basePadding = Size2F { edgeSize.width() * 8.0f, edgeSize.height() * 2.0f };

//	float mincmtextsize = size.toPixelsY(0.2f);
//	float minrowtextsize = min(totalcontentheight / (minLinesCount * 1.1f), sharedTextSize);
//	float maximumtextsize = max(minrowtextsize, sharedTextSize);
//	float mintextsize = max(maximumtextsize, mincmtextsize);
//
//	sharedTextSize = mintextsize;
//	if (sharedTextSize > minrowtextsize) {
//		sharedTextSize = minrowtextsize;
//	}

//	float totallinecount = totalcontentheight / (sharedTextSize + font->getLeading(sharedTextSize));

	bool needmore = distributeLines(maxpxsize);
//	if (needmore) {
//		if (titleTextSize > size.pixelSize.height() / 14.0f) {
//			titleTextSize = size.pixelSize.height() / 14.0f;
//			sharedTextSize = titleTextSize * 7 / 8;
//			totallinecount = totalcontentheight / (sharedTextSize + font->getLeading(sharedTextSize));
//			distributeLines(totallinecount, maxpxsize);
//		}
//	}

	float maxmeasuredwidth = 0.0f;
	for (auto&& i : items.objects()) {
		float measuredwidth = i.measureTextWidth(sharedTextSize, maxpxsize.width() - basePadding.width());
		if (measuredwidth > maxmeasuredwidth) {
			maxmeasuredwidth = measuredwidth;
		}
	}
	pxsize.width() = max(maxmeasuredwidth + basePadding.width(), maxwidth);
	if (pxsize.width() > maxpxsize.width()) {
		pxsize.width() = maxpxsize.width();
	}

	Size2F acc { basePadding.width() / 2.0f, totaltitleheight };
	float contentheight = 0.0f;
	for (auto&& i : items.objects()) {
		Size2F maxdim = Size2F { pxsize.width() - basePadding.width(), sharedTextSize * i.lines
				+ font->getLeading(sharedTextSize) * (i.lines - 1) };
		Size2F ms = { maxdim.width(), maxdim.height() + basePadding.height() };
		i.rect = Rectangle { acc, acc + ms };
		contentheight += ms.height();
		acc.height() += ms.height();
	}
//	if (!items.isEmpty()) {
//		contentheight -= basePadding.height() / 2.0f;
//	}

	pxsize.height() = min(contentheight + titleheight + edgeSize.height() * 2, maxpxsize.height());

	Size2F padding = (size.pixelSize - pxsize) / 2.0f;
	targetPos = {padding.width(), padding.height(), size.pixelSize.width() - padding.width(), size.pixelSize.height() - padding.height()};

	scrollDetector.setSize(Vector2F { pxsize.width(), pxsize.height() - titleheight } - edgeSize * 2,
			Vector2F { pxsize.width() - edgeSize.width() * 2, contentheight });
	scrollDetector.setWheelMultiplier(Vector2F { 0.0f, (sharedTextSize + basePadding.height()) * 2.5f });

	return pxsize;
}

void DialogLayer::redraw(const Size2UI& pxsize) {
	renderer->pushRenderTarget(target);
	{
		lastDrawScrollPosition = scrollDetector.getPosition();
		auto vp = renderer->getViewPort();
		renderer->setDepthTest(false);
		renderer->resetViewPort();
		renderer->initDraw();

		auto mvp = Matrix2D { }.setScreenDimension(pxsize).multRenderToTexture(renderer);

		drawEdge(mvp, getUiColor());

		Matrix2D itemmvp = Matrix2D { }.setTranslate(0, scrollDetector.getPosition().y()) *= mvp;
		for (auto&& i : items.objects()) {
			if (i.getRect().bottom + scrollDetector.getPosition().y() <= backButtonPos.bottom
					|| i.getRect().top + scrollDetector.getPosition().y() >= pxsize.height() - edgeSize.height()) {
				continue;
			}
			if (!i.noHighlightRectDraw && isHighlighted(&i)) {
				drawRectangleColor(itemmvp, getUiColor(), Rectangle { 0.0f, i.getRect().top, (float) pxsize.width(), i.getRect().bottom });
			}
			i.draw(itemmvp);
		}

		drawRectangleColor(mvp, Color { 0, 0, 0, 1 },
				Rectangle { edgeSize.width(), edgeSize.height(), (float) pxsize.width() - edgeSize.width(), backButtonPos.bottom });

		if (backPressed) {
			drawRectangleColor(mvp, getUiColor(), backButtonPos);
		}
		drawSapphireTexture(mvp, backIconWhite, backPressed ? getUiSelectedColor() : getUiColor(), backButtonPos, Rectangle { 0, 0, 1, 1 });
		drawString(mvp, title, font, getUiColor(), Vector2F { backButtonPos.right, backButtonPos.middle().y() }, titleTextSize,
				Gravity::LEFT | Gravity::CENTER_VERTICAL);

		if (isScrolling()) {
			float heightpercent = scrollDetector.getSize().height() / scrollDetector.getWorkingSize().height();
			float startpercent = scrollDetector.getPosition().y() / scrollDetector.getScrollRange().height() * (1 - heightpercent);
			float endpercent = startpercent + heightpercent;
			drawRectangleColor(mvp, getUiColor(),
					Rectangle { pxsize.width() - edgeSize.width() * 3, backButtonPos.bottom
							+ scrollDetector.getSize().height() * startpercent, pxsize.width() - edgeSize.width(), backButtonPos.bottom
							+ scrollDetector.getSize().height() * endpercent });
		}

		renderer->setViewPort(vp);
	}
	renderer->popRenderTarget();
}

void DialogLayer::drawEdge(const rhfw::Matrix2D& mvp, const rhfw::Color& color) {
	renderer->clearColor(color);
	auto size = buffer->getSize();

	drawRectangleColor(mvp, Color { 0, 0, 0, 1 }, Rectangle { edgeSize, size - edgeSize });
}

bool DialogLayer::touchImpl() {
	if (TouchEvent::instance.isJustDown()) {
		touchStartHighlight = highlightedItem;
		if (getScene()->getWindow()->isSoftKeyboardShowing()) {
			touchStartKeyboardOffset = getSoftKeyboardOffset(touchStartHighlight);
		}
	}

	if ((TouchEvent::instance.getAction() != TouchAction::WHEEL || TouchEvent::instance.getAction() != TouchAction::SCROLL)
			&& (!showing || !gainedInput || TouchEvent::instance.getPointerCount() > 1
					|| TouchEvent::instance.getAction() == TouchAction::CANCEL
					|| !targetPos.isInside(TouchEvent::instance.getAffectedPointer()->getPosition() + Vector2F { 0,
							-touchStartKeyboardOffset }))) {
		if (highlightedItem) {
			highlightedItem->get()->onHighlightRevoked();
			highlightedItem.invalidate();
		}
		touchStartKeyboardOffset = 0.0f;
		touchStartHighlight.invalidate();
		backPressed = false;
		invalidate();
		//because of outside touch
		return showing ? true : false;
	}

	bool applyscroll = TouchEvent::instance.getAction() == TouchAction::WHEEL || TouchEvent::instance.getAction() == TouchAction::SCROLL
			|| !highlightedItem;
	scrollDetector.onTouch(applyscroll);
	if (applyscroll) {
		invalidate();
	}

	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			Vector2F touchpos = TouchEvent::instance.getAffectedPointer()->getPosition() - targetPos.leftTop();
			touchpos.y() -= touchStartKeyboardOffset;
			Vector2F scrolltouchpos = touchpos - scrollDetector.getPosition();

			if (backButtonPos.isInside(touchpos)) {
				backPressed = true;
				if (highlightedItem) {
					highlightedItem->get()->onHighlightRevoked();
					highlightedItem.invalidate();
				}
			} else {
				bool unhighlightcurrent = true;
				for (auto&& i : items.pointers()) {
					if (!i->get()->highlightable)
						continue;
					if (i->get()->rect.isInside(scrolltouchpos)) {
						//found
						unhighlightcurrent = false;
						if (highlightedItem) {
							if (i->get() == highlightedItem->get()) {
								//selected the same
								break;
							}
							//revoke the previous one
							highlightedItem->get()->onHighlightRevoked();
							highlightedItem.invalidate();
						}
						highlightedItem = {items, i};
						i->get()->onHighlighted(&touchpos);
						break;
					}
				}
				if (unhighlightcurrent) {
					if (highlightedItem) {
						highlightedItem->get()->onHighlightRevoked();
						highlightedItem.invalidate();
					}
				}
			}
			invalidate();
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			Vector2F touchpos = TouchEvent::instance.getAffectedPointer()->getPosition() - targetPos.leftTop();
			touchpos.y() -= touchStartKeyboardOffset;
			Vector2F scrolltouchpos = touchpos - scrollDetector.getPosition();
			if (highlightedItem && !highlightedItem->get()->rect.isInside(scrolltouchpos)) {
				if (highlightedItem) {
					highlightedItem->get()->onHighlightRevoked();
					highlightedItem.invalidate();
				}
				invalidate();
			} else if (backPressed && !backButtonPos.isInside(touchpos)) {
				backPressed = false;
				invalidate();
			}
			break;
		}
		case TouchAction::UP: {
			if (TouchEvent::instance.getPointerCount() == 0 && touchStartHighlight && highlightedItem != touchStartHighlight) {
				touchStartHighlight.invalidate();
			}
			if (highlightedItem) {
				auto* hl = highlightedItem->get();
				highlightedItem->get()->onHighlightRevoked();
				highlightedItem.invalidate();
				Vector2F touchpos = TouchEvent::instance.getAffectedPointer()->getPosition() - targetPos.leftTop();
				touchpos.y() -= touchStartKeyboardOffset;
				//touchpos -= basePadding / 2.0f;
				hl->onSelected(&touchpos);
				animateToItem(hl);
			} else if (backPressed) {
				backPressed = false;
				dismiss();
			}
			touchStartKeyboardOffset = 0.0f;
			touchStartHighlight.invalidate();
			invalidate();
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

float CommandDialogItem::measureTextHeight(float textsize, float maxwidth) {
	return minLines;
}

float CommandDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return dialog->getFont()->measureText(text.begin(), text.end(), textsize);
}

void CommandDialogItem::draw(const rhfw::Matrix2D& mvp) {
	const bool highlighted = dialog->isHighlighted(this);
	Color color = highlighted ? dialog->getUiSelectedColor() : (this->textColor == nullptr ? dialog->getUiColor() : *this->textColor);

	drawCommandImpl(mvp, color);
}
void CommandDialogItem::drawCommandImpl(const Matrix2D& mvp, const Color& textcolor) {
	Vector2F pos { (getRect().leftTop() + getRect().leftBottom()) / 2 };
	drawString(mvp, text, dialog->getFont(), textcolor, pos, dialog->getSharedTextSize(), Gravity::LEFT | Gravity::CENTER_VERTICAL);
}

void ValueCommandDialogItem::drawCommandImpl(const Matrix2D& mvp, const Color& textcolor) {
	CommandDialogItem::drawCommandImpl(mvp, textcolor);
	if (value != nullptr) {
		Vector2F pos { (getRect().rightTop() + getRect().rightBottom()) / 2 };
		drawString(mvp, value, dialog->getFont(), textcolor, pos, dialog->getSharedTextSize(), Gravity::RIGHT | Gravity::CENTER_VERTICAL);
	}
}

float ValueCommandDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return CommandDialogItem::measureTextWidth(textsize, maxwidth)
			+ (value == nullptr ? 0 : textsize + dialog->getFont()->measureText(value.begin(), value.end(), textsize));
}

static const int MAX_CHAR_PER_LINE = 44;
float TextDialogItem::measureTextHeight(float textsize, float maxwidth) {
	float result = minLines;
	const char* start = text.begin();
	const char* end = text.end();
	const char* it;
	float xpos = 0.0f;
	float leading = dialog->getFont()->getLeading(textsize);

	float spacewidth = dialog->getFont()->measureText(" ", textsize);

	int linecharcount = 0;
	//flowing text
	for (it = start;; ++it) {
		++linecharcount;
		if (it == end) {
			float width = dialog->getFont()->measureText(start, it, textsize);
			if (xpos + width > maxwidth || linecharcount > MAX_CHAR_PER_LINE) {
				//new line
				result += 1.0f;
				xpos = 0.0f;
				linecharcount = it - start;
			}
			break;
		} else if (*it == ' ' || *it == '\n') {
			float width = dialog->getFont()->measureText(start, it, textsize);
			if (xpos + width > maxwidth || linecharcount > MAX_CHAR_PER_LINE) {
				//new line
				result += 1.0f;
				xpos = 0.0f;
				linecharcount = it - start;
			}
			xpos += width;

			start = it + 1;
			if (*it == '\n') {
				xpos = 0.0f;
				result += 1.0f;
				linecharcount = 0;
			} else {
				xpos += spacewidth;
			}
		}
	}
	return result;
}

float TextDialogItem::measureTextWidth(float textsize, float maxwidth) {
	int drawlines = (int) this->getLines();

	if (drawlines == 0) {
		return 0.0f;
	}
	float maxwidthres = 0.0f;
	const char* start = text.begin();
	const char* end = text.end();
	const char* it;
	float xpos = 0.0f;
	float leading = dialog->getFont()->getLeading(textsize);
	int linecharcount = 0;

	float spacewidth = dialog->getFont()->measureText(" ", textsize);
	//flowing text
	for (it = start;; ++it) {
		++linecharcount;
		if (it == end) {
			float width = dialog->getFont()->measureText(start, it, textsize);
			if (xpos + width > maxwidth || linecharcount > MAX_CHAR_PER_LINE) {
				//new line
				xpos = 0.0f;
				if (--drawlines <= 0) {
					break;
				}
				linecharcount = it - start;
			} else {
				if (xpos + width > maxwidthres) {
					maxwidthres = xpos + width;
				}
			}
			break;
		} else if (*it == ' ' || *it == '\n') {
			float width = dialog->getFont()->measureText(start, it, textsize);
			if (xpos + width > maxwidth || linecharcount > MAX_CHAR_PER_LINE) {
				//new line
				if (xpos > maxwidthres) {
					maxwidthres = xpos;
				}
				if (--drawlines <= 0) {
					break;
				}
				xpos = 0.0f;
				linecharcount = it - start;
			}
			xpos += width;
			if (xpos > maxwidthres) {
				maxwidthres = xpos;
			}

			start = it + 1;
			if (*it == '\n') {
				if (xpos > maxwidthres) {
					maxwidthres = xpos;
				}
				if (--drawlines <= 0) {
					break;
				}
				xpos = 0.0f;
				linecharcount = 0;
			} else {
				xpos += spacewidth;
			}
		}
	}
	return maxwidthres;
}

void TextDialogItem::draw(const rhfw::Matrix2D& mvp) {
	const char* start = text.begin();
	const char* end = text.end();
	const char* it;
	Vector2F pos { getRect().leftTop() };
	float textSize = dialog->getSharedTextSize();
	float leading = dialog->getFont()->getLeading(textSize);

	auto rect = getRect();

	int linecharcount = 0;
	float spacewidth = dialog->getFont()->measureText(" ", textSize);

	const Color& textcolor = this->textColor == nullptr ? dialog->getUiColor() : *this->textColor;

	//flowing text
	for (it = start;; ++it) {
		++linecharcount;
		if (it == end) {
			if (start != it) {
				float width = dialog->getFont()->measureText(start, it, textSize);
				if (pos.x() + width > rect.right + 0.1f || linecharcount > MAX_CHAR_PER_LINE) {
					//new line
					if (pos.y() + (textSize * 2) > rect.bottom + 0.1f) {
						break;
					}
					pos.y() += textSize + leading;
					pos.x() = rect.left;
					linecharcount = it - start;
				}
				drawString(mvp, start, it, dialog->getFont(), textcolor, pos, textSize, Gravity::TOP | Gravity::LEFT);
			}
			//do not print ..
			return;
		} else if (*it == ' ' || *it == '\n') {

			float width = dialog->getFont()->measureText(start, it, textSize);
			if (pos.x() + width > rect.right + 0.1f || linecharcount > MAX_CHAR_PER_LINE) {
				//new line
				if (pos.y() + (textSize * 2 + leading) > rect.bottom + 0.1f) {
					pos.x() -= spacewidth;
					break;
				}
				pos.y() += textSize + leading;
				pos.x() = rect.left;
				linecharcount = it - start;
			}
			pos.x() += drawString(mvp, start, it, dialog->getFont(), textcolor, pos, textSize, Gravity::TOP | Gravity::LEFT);

			start = it + 1;
			if (*it == '\n') {
				if (pos.y() + (textSize * 2 + leading) > rect.bottom + 0.1f) {
					break;
				}
				pos.y() += textSize + leading;
				pos.x() = rect.left;
				linecharcount = 0;
			} else {
				pos.x() += spacewidth;
			}
		}
	}
	drawString(mvp, "..", dialog->getFont(), textcolor, pos, textSize, Gravity::TOP | Gravity::LEFT);
}

void DialogLayer::dismiss() {
	if (highlightedItem) {
		highlightedItem->get()->onHighlightRevoked();
		highlightedItem.invalidate();
	}
	SapphireUILayer::dismiss();
}

void DialogLayer::relayout() {
	/*if (highlightedItem) {
	 highlightedItem->get()->onHighlightRevoked();
	 highlightedItem.invalidate();
	 }*/
	touchStartHighlight.invalidate();
	backPressed = false;

	Size2F pxsize = layoutItems();

	buffer->setInputSource(new render::EmptyInputSource { pxsize, ColorFormat::RGBA_8888 });
	buffer.loadOrReload();
	target.loadOrReload();

	invalidate();
}

void DialogLayer::nextKeyboardSelection() {
	if (items.isEmpty()) {
		return;
	}
	auto firststart = highlightedItem;
	if (!highlightedItem.isValid()) {
		//try the first if nothing is highlighted now
		++highlightedItem;
		if (highlightedItem->get()->highlightable) {
			animateToItem(highlightedItem->get());
			invalidate();
			return;
		}
	}
	auto start = highlightedItem;
	++highlightedItem;
	while (!highlightedItem->get()->highlightable) {
		++highlightedItem;
		if (highlightedItem == start) {
			if (!highlightedItem->get()->highlightable) {
				highlightedItem.invalidate();
			}
			break;
		}
	}

	if (highlightedItem.isValid()) {
		if (firststart) {
			firststart->get()->onHighlightRevoked();
		}
		highlightedItem->get()->onHighlighted(nullptr);
		animateToItem(highlightedItem->get());
	}

	invalidate();
}
void DialogLayer::previousKeyboardSelection() {
	if (items.isEmpty()) {
		return;
	}
	auto firststart = highlightedItem;
	if (!highlightedItem.isValid()) {
		//try the last if nothing is highlighted now
		--highlightedItem;
		if (highlightedItem->get()->highlightable) {
			animateToItem(highlightedItem->get());
			invalidate();
			return;
		}
	}
	auto start = highlightedItem;
	--highlightedItem;
	while (!highlightedItem->get()->highlightable) {
		--highlightedItem;
		if (highlightedItem == start) {
			if (!highlightedItem->get()->highlightable) {
				highlightedItem.invalidate();
			}
			break;
		}
	}

	if (highlightedItem.isValid()) {
		if (firststart) {
			firststart->get()->onHighlightRevoked();
		}
		highlightedItem->get()->onHighlighted(nullptr);
		animateToItem(highlightedItem->get());
	}

	invalidate();
}

void DialogLayer::displayKeyboardSelection() {
	if (!highlightedItem.isValid()) {
		nextKeyboardSelection();
	}
}

void DialogLayer::addDialogItemBefore(DialogItem* relativeto, DialogItem* item) {
	ASSERT(items.contains(relativeto));
	items.insertBefore(*relativeto, *item);

	item->dialog = this;
}
void DialogLayer::addDialogItemAfter(DialogItem* relativeto, DialogItem* item) {
	ASSERT(items.contains(relativeto));
	items.insertAfter(*relativeto, *item);

	item->dialog = this;
}

void DialogLayer::hideKeyboardSelection() {
	if (highlightedItem) {
		highlightedItem->get()->onHighlightRevoked();
		highlightedItem.invalidate();
		invalidate();
	}
}

void DialogLayer::removeDialogItem(DialogItem* item) {
	queueRemoveDialogItem(item);
	finishRemoveDialogItem();
}

void DialogLayer::clearDialogItems() {
	if (highlightedItem) {
		highlightedItem->get()->onHighlightRevoked();
		highlightedItem.invalidate();
	}
	this->items.clear();
	this->relayout();
}

void DialogLayer::finishRemoveDialogItem() {
	this->relayout();
}

void DialogLayer::queueRemoveDialogItem(DialogItem* item) {
	if (highlightedItem && highlightedItem->get() == item) {
		item->onHighlightRevoked();

		auto end = items.ring();
		do {
			--highlightedItem;
		} while (highlightedItem != end && !highlightedItem->get()->highlightable);
	}
	item->removeLinkFromList();
}

void DialogLayer::animateToItem(DialogItem* item) {
	scrollDetector.animateTo(scrollDetector.getTargetScrollPosition(item->rect.translate(Vector2F { 0, -backButtonPos.bottom })));
}

} // namespace userapp

