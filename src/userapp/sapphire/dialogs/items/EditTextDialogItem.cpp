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
 * EditTextDialogItem.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <framework/core/timing.h>
#include <framework/utils/utility.h>
#include <framework/geometry/Vector.h>
#include <framework/io/key/KeyEvent.h>
#include <sapphire/SapphireScene.h>
#include <appmain.h>

#define CARET_DISPLAY_PERIOD_MS 800

namespace userapp {
using namespace rhfw;

EditTextDialogItem::EditTextDialogItem(FixedString text, const FixedString& content, float textspace)
		: DialogItem(1.0f), text(util::move(text)), contentCapacity(max((content.length() + 1) * 3 / 2, (unsigned int) 64)), contentLength(
				content.length()), content(new char[contentCapacity]), textSpace(textspace) {
	memcpy(this->content, (const char*) content, contentLength * sizeof(char));
	setHighlightable(true);
	this->content[contentLength] = 0;
}

EditTextDialogItem::~EditTextDialogItem() {
	delete returnHandler;
	delete[] content;
}

void EditTextDialogItem::draw(const rhfw::Matrix2D& mvp) {
	const bool highlighted = dialog->isHighlighted(this);
	Color color = highlighted ? dialog->getUiSelectedColor() : dialog->getUiColor();

	Vector2F pos { (getRect().leftTop() + getRect().leftBottom()) / 2 };
	float textwidth = drawString(mvp, text, dialog->getFont(), color, pos, dialog->getSharedTextSize(),
			Gravity::LEFT | Gravity::CENTER_VERTICAL);

	Vector2F contentpos { (getRect().rightTop() + getRect().rightBottom()) / 2 };
	contentpos.x() -= dialog->getSharedTextSize() / 8.0f;

	struct ReverseIterator {
	public:
		char* ptr;
		char* end;

		char operator*() const {
			if (ptr < end) {
				return 0;
			}
			return *ptr;
		}
		ReverseIterator& operator++() {
			--ptr;
			return *this;
		}
		char* getPointer() const {
			if (ptr < end) {
				return end;
			}
			return ptr + 1;
		}
	};

	unsigned int contentcount = content + contentLength - dialog->getFont()->measureText(ReverseIterator { content + contentLength - 1,
			content }, dialog->getSharedTextSize(), getRect().width() - textwidth).getPointer();

	drawString(mvp, content + contentLength - contentcount, content + contentLength, dialog->getFont(), color, contentpos,
			dialog->getSharedTextSize(), Gravity::RIGHT | Gravity::CENTER_VERTICAL);

	if (cursorDisplay && highlighted) {
		drawRectangleColor(mvp, color, Rectangle { contentpos.x(), getRect().top, contentpos.x() + dialog->getSharedTextSize() / 8.0f,
				getRect().bottom });
	}
}

float EditTextDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return dialog->getFont()->measureText(text.begin(), text.end(), textsize) + textsize * textSpace;
}
#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
#define RETURN_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) return true
bool EditTextDialogItem::onKeyEvent() {
	if (!dialog->isHighlighted(this)) {
		return false;
	}
	if (KeyEvent::instance.getAction() == KeyAction::UNICODE_SEQUENCE) {
		unsigned int len = KeyEvent::instance.getUnicodeSequenceLength();
		const UnicodeCodePoint* seq = KeyEvent::instance.getUnicodeSequence();
		for (unsigned int i = 0; i < len; ++i) {
			UnicodeCodePoint cp = seq[i];
			if (cp >= '0' && cp <= '9') {
				addChar(cp);
			} else if (!numericOnly && (cp >= 32 && cp <= 126)) {
				addChar(cp);
			}
		}
		dialog->invalidate();
		return true;
	}
	if (KeyEvent::instance.getAction() == KeyAction::UNICODE_REPEAT) {
		const UnicodeCodePoint cp = KeyEvent::instance.getUnicodeRepeat();
		if ((cp >= '0' && cp <= '9') || (!numericOnly && (cp >= 32 && cp <= 126))) {
			unsigned int count = KeyEvent::instance.getUnicodeRepeatCount();
			for (unsigned int i = 0; i < count; ++i) {
				addChar(cp);
			}
			dialog->invalidate();
		}
		return true;
	}
	auto keycode = KeyEvent::instance.getKeycode();
	/*if (keycode >= KeyCode::KEY_NUM_0 && keycode <= KeyCode::KEY_NUM_9) {
	 keycode = (KeyCode) ((unsigned int) KeyCode::KEY_0 + (unsigned int) keycode - (unsigned int) KeyCode::KEY_NUM_0);
	 }*/
	/*if (keycode >= KeyCode::KEY_0 && keycode <= KeyCode::KEY_9) {
	 RETURN_ON_NOT_DOWN();
	 addChar('0' + ((unsigned int) keycode - (unsigned int) KeyCode::KEY_0));
	 dialog->invalidate();
	 } else {
	 if (!numericOnly && keycode >= KeyCode::KEY_A && keycode <= KeyCode::KEY_Z) {
	 RETURN_ON_NOT_DOWN();
	 if ((KeyEvent::instance.getModifiers() & KeyModifiers::SHIFT_ON_BOOL_MASK) != 0) {
	 addChar('A' + ((unsigned int) keycode - (unsigned int) KeyCode::KEY_A));
	 } else {
	 addChar('a' + ((unsigned int) keycode - (unsigned int) KeyCode::KEY_A));
	 }
	 dialog->invalidate();
	 } else {*/
	switch (keycode) {
		/*case KeyCode::KEY_SPACE: {
		 if (!numericOnly) {
		 BREAK_ON_NOT_DOWN();
		 addChar(' ');
		 dialog->invalidate();
		 } else {
		 return false;
		 }
		 break;
		 }*/
		case KeyCode::KEY_BACKSPACE: {
			BREAK_ON_NOT_DOWN();
			deleteChar();
			break;
		}
		case KeyCode::KEY_ENTER: {
			if(softKeyboardEditing) {
				BREAK_ON_NOT_DOWN();
				softKeyboardEditing = false;
				dialog->invalidateHighlight();
				dialog->getScene()->getWindow()->dismissSoftKeyboard();
				dialog->invalidate();
			} else {
				if(returnHandler != nullptr) {
					BREAK_ON_NOT_DOWN();
					return returnHandler->handleReturn();
				}
				return false;
			}
			break;
		}
		case KeyCode::KEY_BACK: {
			if(softKeyboardEditing) {
				BREAK_ON_NOT_DOWN();
				softKeyboardEditing = false;
				dialog->invalidateHighlight();
				dialog->getScene()->getWindow()->dismissSoftKeyboard();
				dialog->invalidate();
			} else {
				return false;
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_RIGHT: {
			if(numericOnly) {
				BREAK_ON_NOT_DOWN();
				auto&& num = stringToNumber(content, contentLength, 0);
				if(num < 0xffffffff) {
					auto&& asstr = numberToString(num + 1);
					if(asstr.length() < contentMaximumLength) {
						setContent(asstr);
						dialog->invalidate();
					}
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT: {
			if(numericOnly) {
				BREAK_ON_NOT_DOWN();
				auto&& num = stringToNumber(content, contentLength, 0);
				if(num > 0) {
					auto&& asstr = numberToString(num - 1);
					if(asstr.length() < contentMaximumLength) {
						setContent(asstr);
						dialog->invalidate();
					}
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_X: {
			BREAK_ON_NOT_DOWN();
			deleteChar();
			break;
		}
		default: {
			if(KeyEvent::instance.getInputDevice() == InputDevice::KEYBOARD) {
				if (dialog->getScene()->getWindow()->isSoftKeyboardShowing()) {
					softKeyboardEditing = false;
					dialog->getScene()->getWindow()->dismissSoftKeyboard();
					dialog->invalidate();
					return true;
				}
			}
			return false;
		}
	}
	/*}
	 }*/
	return true;
}

bool EditTextDialogItem::shouldInvalidate() {
	if (!dialog->isHighlighted(this)) {
		return false;
	}
	//invalidate on cursor tick
	bool oldcd = cursorDisplay;
	cursorDisplay = (((long long) (core::time_millis) core::MonotonicTime::getCurrent()) / CARET_DISPLAY_PERIOD_MS) % 2 == 0;
	return oldcd != cursorDisplay;
}

float EditTextDialogItem::measureTextHeight(float textsize, float maxwidth) {
	return minLines;
}

void EditTextDialogItem::addChar(char c) {
	if (contentLength + 1 > contentMaximumLength) {
		return;
	}
	if (contentLength + 1 >= contentCapacity) {
		contentCapacity = (contentLength + 1) * 3 / 2;
		auto* old = content;
		content = new char[contentCapacity];
		memcpy(content, old, contentLength * sizeof(char));
		delete[] old;
	}
	content[contentLength++] = c;
	content[contentLength] = 0;
}

void EditTextDialogItem::deleteChar() {
	if (contentLength > 0) {
		content[--contentLength] = 0;
		dialog->invalidate();
	}
}

void EditTextDialogItem::onHighlightRevoked() {
	if (softKeyboardEditing) {
		dialog->getScene()->getWindow()->dismissSoftKeyboard();
		softKeyboardEditing = false;
	}
}

void EditTextDialogItem::onSelected(const Vector2F* pointer) {
	bool keyboard = pointer == nullptr;
	if (!keyboard && !dialog->getScene()->getWindow()->isHardwareKeyboardPresent()) {
		if (softKeyboardEditing) {
			dialog->getScene()->getWindow()->dismissSoftKeyboard();
			softKeyboardEditing = false;
		} else {
			dialog->getScene()->getWindow()->requestSoftKeyboard(numericOnly ? KeyboardType::NUMERIC : KeyboardType::ALPHANUMERIC);
			dialog->setHighlighted(this);
			softKeyboardEditing = true;
		}
	} else {
		//has keyboard, or hardware keyboard is present
		dialog->setHighlighted(this);
		softKeyboardEditing = false;
	}
}

void EditTextDialogItem::setContent(const char* content, unsigned int contentlength) {
	if (contentCapacity < (contentlength + 1)) {
		contentCapacity = (contentlength + 1) * 2 / 3;
		delete[] this->content;
		this->content = new char[contentCapacity];
	}
	memcpy(this->content, content, contentlength + 1);
	this->contentLength = contentlength;
}

} // namespace userapp

