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
 * EnumDialogItem.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_ITEMS_ENUMDIALOGITEM_H_
#define TEST_SAPPHIRE_DIALOGS_ITEMS_ENUMDIALOGITEM_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/utility.h>

namespace userapp {

class EnumDialogItem: public CommandDialogItem {
public:
	class SelectionListener {
	public:
		virtual ~SelectionListener() = default;
		virtual void onSelectionChanged(unsigned int sel) = 0;
	};
private:
	const char* const * items;
	unsigned int selected;
	unsigned int count;

	SelectionListener* selectionListener = nullptr;
public:
	EnumDialogItem(float weight, rhfw::FixedString text, const char* const * items, unsigned int selected, unsigned int count)
			: CommandDialogItem(weight, rhfw::util::move(text), [this] {
				this->selected = (this->selected + 1) % this->count;
				if(selectionListener != nullptr) {
					selectionListener->onSelectionChanged(this->selected);
				}
			}), items(items), selected(selected), count(count) {
	}
	~EnumDialogItem() {
		delete selectionListener;
	}

	template<typename Handler>
	void setSelectionListener(Handler&& handler) {
		class Impl: public SelectionListener {
			typename util::remove_reference<Handler>::type handler;
		public:
			Impl(Handler&& ahandler)
					: handler(util::forward<Handler>(ahandler)) {
			}
			virtual void onSelectionChanged(unsigned int sel) override {
				handler(sel);
			}
		};

		delete selectionListener;
		this->selectionListener = new Impl(util::forward<Handler>(handler));
	}

	virtual void draw(const rhfw::Matrix2D& mvp) override {
		CommandDialogItem::draw(mvp);

		float textSize = dialog->getSharedTextSize();
		const bool highlighted = dialog->isHighlighted(this);
		Color color = highlighted ? dialog->getUiSelectedColor() : dialog->getUiColor();
		const float padding = (getRect().height() - textSize) / 2.0f;

		drawString(mvp, items[selected], dialog->getFont(), color,
				Vector2F { getRect().right - padding, getRect().top + getRect().height() / 2.0f }, dialog->getSharedTextSize(),
				Gravity::RIGHT | Gravity::CENTER_VERTICAL);
	}

	virtual float measureTextWidth(float textsize, float maxwidth) override {
		unsigned int len = strlen(items[0]);
		const char* longest = items[0];
		for (int i = 1; i < count; ++i) {
			int testlen = strlen(items[i]);
			if (testlen > len) {
				len = testlen;
				longest = items[i];
			}
		}
		return CommandDialogItem::measureTextWidth(textsize, maxwidth) + textsize + dialog->getFont()->measureText(longest, textsize);
	}

	unsigned int getSelected() const {
		return selected;
	}

	virtual bool onKeyEvent() override;
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_ITEMS_ENUMDIALOGITEM_H_ */
