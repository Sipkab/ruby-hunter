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
 * SelectorEnumDialogItem.h
 *
 *  Created on: 2017. aug. 3.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_ITEMS_SELECTORENUMDIALOGITEM_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_ITEMS_SELECTORENUMDIALOGITEM_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <framework/utils/utility.h>

namespace userapp {
using namespace rhfw;

class SelectorEnumDialogItem: public DialogItem {
public:
	class SelectionListener {
	public:
		virtual ~SelectionListener() = default;
		virtual void onSelectionChanged(unsigned int sel) = 0;
	};
protected:
	virtual void onHighlighted(Vector2F* pointer) override;
private:
	const char* const * items;
	unsigned int selected;
	unsigned int count;

	SelectionListener* selectionListener = nullptr;

	int touchHighlightIndex = -1;

	int getIndexForPointer(const Vector2F& pointer);
public:
	SelectorEnumDialogItem(const char* const * items, unsigned int selected, unsigned int count);
	~SelectorEnumDialogItem();

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

	virtual void draw(const rhfw::Matrix2D& mvp) override;
	virtual float measureTextWidth(float textsize, float maxwidth) override;

	virtual bool onKeyEvent() override;

	unsigned int getSelected() const {
		return selected;
	}

	virtual float measureTextHeight(float textsize, float maxwidth) override {
		return minLines;
	}
	virtual void onSelected(const Vector2F* pointer) override;
};

} // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_ITEMS_SELECTORENUMDIALOGITEM_H_ */
