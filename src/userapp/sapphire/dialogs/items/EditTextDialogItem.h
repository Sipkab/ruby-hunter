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
 * EditTextDialogItem.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_ITEMS_EDITTEXTDIALOGITEM_H_
#define TEST_SAPPHIRE_DIALOGS_ITEMS_EDITTEXTDIALOGITEM_H_

#include <sapphire/dialogs/DialogLayer.h>

#include <framework/utils/utility.h>
#include <framework/utils/FixedString.h>

namespace userapp {
using namespace rhfw;

class EditTextDialogItem: public DialogItem {
public:
	class ReturnHandler {
	public:
		virtual ~ReturnHandler() = default;

		virtual bool handleReturn() = 0;
	};
private:
	FixedString text;
	bool cursorDisplay = false;
	bool softKeyboardEditing = false;

	unsigned int contentCapacity;
	unsigned int contentLength;
	char* content;

	unsigned int contentMaximumLength = 0xFFFFFFFF;

	bool numericOnly = false;

	float textSpace = 16.0f;

	ReturnHandler* returnHandler = nullptr;

	void addChar(char c);
	void deleteChar();
protected:
	virtual void onHighlightRevoked() override;
public:
	EditTextDialogItem(FixedString text, const FixedString& content, float textSpace = 16.0f);
	~EditTextDialogItem();

	template<typename Handler>
	void setReturnHandler(Handler&& handler) {
		class Impl: public ReturnHandler {
			typename util::remove_reference<Handler>::type handler;
		public:
			Impl(Handler&& ahandler)
					: handler(util::forward<Handler>(ahandler)) {
			}
			virtual bool handleReturn() override {
				return handler();
			}
		};

		delete returnHandler;
		this->returnHandler = new Impl(util::forward<Handler>(handler));
	}

	virtual bool onKeyEvent() override;
	virtual bool shouldInvalidate() override;

	virtual void draw(const rhfw::Matrix2D& mvp) override;
	virtual float measureTextWidth(float textsize, float maxwidth) override;
	virtual float measureTextHeight(float textsize, float maxwidth) override;

	virtual void onSelected(const Vector2F* pointer) override;

	FixedString getContentString() const {
		return FixedString { content, contentLength };
	}
	const char* getContent() const {
		return content;
	}
	unsigned int getContentLength() const {
		return contentLength;
	}

	void setNumericOnly(bool numeric) {
		this->numericOnly = numeric;
	}

	void setContent(const char* content, unsigned int contentlength);
	void setContent(const FixedString& content) {
		setContent(content, content.length());
	}

	void setContentMaximumLength(unsigned int length) {
		this->contentMaximumLength = length;
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_ITEMS_EDITTEXTDIALOGITEM_H_ */
