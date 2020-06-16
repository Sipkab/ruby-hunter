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
 * SapphireMenu.cpp
 *
 *  Created on: 2016. apr. 14.
 *      Author: sipka
 */

#include <framework/core/Window.h>
#include <framework/geometry/Vector.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/layer/Layer.h>
#include <framework/render/Renderer.h>
#include <framework/scene/Scene.h>
#include <framework/utils/FixedString.h>
#include <framework/xml/XmlNode.h>
#include <framework/animation/PropertyAnimator.h>
#include <gen/types.h>
#include <gen/log.h>
#include <gen/xmldecl.h>
#include <sapphire/dialogs/SettingsLayer.h>
#include <sapphire/DifficultySelectorLayer.h>
#include <sapphire/SapphireMenu.h>
#include <sapphire/SapphireMenuElement.h>
#include <sapphire/CreditsLayer.h>
#include <sapphire/LevelEditorLayer.h>
#include <sapphire/community/CommunityLayer.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>
#include <sapphire/levelrender/LevelDrawer3D.h>
#include <sapphire/SapphireSplashScreen.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>
#include <sapphire/dialogs/LoadingDialog.h>

using namespace rhfw;
using namespace userapp;

LINK_XML_SIMPLE(SapphireMenu)

namespace userapp {

SapphireMenu::SapphireMenu(const xml::XmlNode& node, const xml::XmlAttributes& attributes)
		: elementCount { (int) node.childcount }, elements { new SapphireMenuElement[elementCount] }, elementRects {
				new Rectangle[elementCount] }, iconAnimations { new LifeCycleChain<Animation> [elementCount] }, iconAnimationValues {
				new float[elementCount] } {
	font = attributes.get<Resource<Font>>(RXml::Attr::font);

	for (int i = 0; i < elementCount; ++i) {
		iconAnimationValues[i] = 0;
	}

	//this->setDisplayPercent(1.0f);
	this->returnText = "Return to main menu";
}
SapphireMenu::~SapphireMenu() {
	delete[] elements;
	delete[] elementRects;
	delete[] iconAnimations;
	delete[] iconAnimationValues;

	delete drawer3D;
}

void SapphireMenu::drawImpl(float displayPercent) {
	renderer->setDepthTest(false);
	renderer->initDraw();
	const core::WindowSize& size = static_cast<SapphireScene*>(getScene())->getUiSize();

	float alpha = displayPercent * displayPercent;

	drawSapphireTexture(mvp, titleTexture, alpha, titleRect, Rectangle { 0, 0, 1, 1 });

	if (drawer3D != nullptr) {
		Size2F objsize { elementRects[0].height(), elementRects[0].height() };
		objsize *= 0.9f;
		for (int i = 0; i < elementCount; ++i) {
			auto& rect = elementRects[i];
			if (i == selectedIndex) {
				drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, rect);
			}

			Matrix3D u_m;
			Matrix3D u_minv;
			Vector2F translate { (elementRects[i].left + rect.height() * 0.55f) / objsize.width(), (size.pixelSize.height()
					- (elementRects[i].middle().y())) / objsize.height() };
			u_m.setIdentity().multTranslate(translate.x(), translate.y(), 0.0f);
			u_minv.setTranslate(-translate.x(), -translate.y(), 0.0f);

			drawer3D->drawSapphire(iconAnimationValues[i] / 0.7f, u_m, u_minv, drawer3D->getDefaultColoredCommand());

			drawString(mvp, elements[i].text, font, i == selectedIndex ? Color { 1, 1, 1, alpha } : Color { SAPPHIRE_COLOR.rgb(), alpha },
					Vector2F { rect.left + rect.height() + iconTextPadding, rect.top + rect.height() / 2 }, elementTextSize,
					Gravity::CENTER_VERTICAL | Gravity::LEFT);
		}
		drawer3D->finishDrawing(alpha, 0.0f, objsize, size.pixelSize, Rectangle { 0, 0, 0, 0 }, 20,
				Vector2F { size.pixelSize } / objsize / 2.0f);
	} else {
		for (int i = 0; i < elementCount; ++i) {
			auto& rect = elementRects[i];
			if (i == selectedIndex) {
				drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, rect);
			}
			auto& elem =
					iconAnimationValues[i] >= 1.0f ?
							elements[i].icon->getAtIndex(0) : elements[i].icon->getAtPercent(iconAnimationValues[i]);
			drawSapphireTexture(mvp, elem, alpha, Rectangle { rect.leftTop(), rect.leftTop() + rect.height() }.inset(rect.height() * 0.05f),
					Rectangle { elem.getPosition() });
			drawString(mvp, elements[i].text, font, i == selectedIndex ? Color { 1, 1, 1, alpha } : Color { SAPPHIRE_COLOR.rgb(), alpha },
					Vector2F { rect.left + rect.height() + iconTextPadding, rect.top + rect.height() / 2 }, elementTextSize,
					Gravity::CENTER_VERTICAL | Gravity::LEFT);
		}

	}
}

void SapphireMenu::sizeChanged(const core::WindowSize& size) {
	SapphireUILayer::sizeChanged(size);
	auto pixels = size.pixelSize;

	mvp = Matrix2D { }.setScreenDimension(size.pixelSize);

	float titlepx = pixels.height() * SAPPHIRE_TITLE_PERCENT;
	if (titlepx > size.toPixelsY(3.5f)) {
		titlepx = size.toPixelsY(3.5f);
	}

	Size2F titlesize = titleTexture->getSize();
	titlesize *= titlepx / titlesize.height();
	if (titlesize.width() > pixels.width()) {
		titlesize *= pixels.width() / titlesize.width();
	}

	titleRect = Rectangle { (float) (pixels.width() - titlesize.width()) / 2, 0, (float) (pixels.width() - titlesize.width()) / 2
			+ titlesize.width(), (float) titlesize.height() };

	float y = titlesize.height() * SAPPHIRE_TITLE_PADDING_PERCENT;
	const float remain = pixels.height() - y - size.toPixelsY(0.5f);
	const float sidepadding = size.toPixelsX(0.5f);
	iconTextPadding = sidepadding / 2.0f;

	float rowheights = min(remain / elementCount, size.toPixelsY(2.5f));

	float textsize = rowheights * 6.0f / 8.0f;
	float maximumtextwidth = size.pixelSize.width() - textsize - sidepadding * 2 - iconTextPadding;
	float maxtextwidth = 0.0f;
	for (int i = 0; i < elementCount; ++i) {
		float mw = font->measureText(elements[i].text, textsize);
		if (mw > maxtextwidth) {
			maxtextwidth = mw;
		}
	}
	if (maxtextwidth > maximumtextwidth) {
		float scale = maximumtextwidth / maxtextwidth;
		rowheights *= scale;
		textsize *= scale;
		maxtextwidth = maximumtextwidth;
	}
	const float elempadding = rowheights / 12.0f;
	const float texturesize = rowheights - elempadding * 2;
	elementTextSize = textsize;
	float xpad = size.pixelSize.width() - maxtextwidth - texturesize;
	for (int i = 0; i < elementCount; ++i) {
		elementRects[i] = Rectangle { xpad / 2, y + elempadding, xpad / 2 + maxtextwidth + texturesize + iconTextPadding, y + rowheights
				- elempadding };
		y += rowheights;
	}

	if (drawer3D != nullptr) {
		Size2F objsize { elementRects[0].height(), elementRects[0].height() };
		objsize *= 0.9f;
		Vector2F mid { (elementRects[0].middle().x()) / objsize.width(), (size.pixelSize.height() - elementRects[0].top) / objsize.height()
				+ 0.2f };
		drawer3D->setFixedLighting(mid);
	}
}

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break

void SapphireMenu::startIconAnimation(int index, float target) {
	class IconAnimator: public Animation {
	private:
	protected:
		void onProgress(const core::time_millis& progress) override {
			const float percent = (float) ((progress - starttime) / duration);
			val = startval + (int) (percent * (targetval - startval) + 0.5f);
		}
		virtual void onFinish() override {
			val = targetval;
		}

		int& val;
		int targetval;
		int startval = 0;

		virtual void onStart() override {
			Animation::onStart();
			startval = val;
		}
	public:
		IconAnimator(int& val, int target, core::time_millis starttime, core::time_millis duration)
				: Animation(starttime, duration), val(val), targetval(target) {
		}
	};

	int count = 2;//elements[index].icon->getChildCount();
	int targetelem = target < 0 ? 0 : count;
	unsigned int time = fabsf((float) (iconAnimationValues[index] - targetelem)) / count * 300;
	auto& animlink = iconAnimations[index];
	animlink.kill();
	long long duration = (long long) (250 * fabsf(iconAnimationValues[index] - target));
	auto* anim = new PropertyAnimator<float>(iconAnimationValues[index], target, core::MonotonicTime::getCurrent(), core::time_millis {
			duration });
	animlink.link(*anim);
	anim->start();
}

void SapphireMenu::setSelectedItem(int index) {
	if (index == selectedIndex) {
		return;
	}

	if (this->selectedIndex >= 0) {
		startIconAnimation(this->selectedIndex, 0.0f);
	}

	this->selectedIndex = index;
	if (this->selectedIndex >= 0) {
		if (drawer3D == nullptr) {
			startIconAnimation(this->selectedIndex, 1.0f);
		}
	}
}

bool SapphireMenu::onKeyEventImpl() {
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
		case KeyCode::KEY_DIR_DOWN: {
			BREAK_ON_NOT_DOWN();
			int newindex = selectedIndex + 1;
			if (newindex >= elementCount) {
				newindex = 0;
			}
			setSelectedItem(newindex);
			break;
		}
#if RHFW_DEBUG
		case KeyCode::KEY_F5: {
			THROW();
			break;
		}
#endif /* RHFW_DEBUG */
		case KeyCode::KEY_GAMEPAD_DPAD_UP:
		case KeyCode::KEY_DIR_UP: {
			BREAK_ON_NOT_DOWN();
			int newindex = selectedIndex - 1;
			if (newindex < 0) {
				newindex = elementCount - 1;
			}
			setSelectedItem(newindex);
			break;
		}
		case KeyCode::KEY_GAMEPAD_A:
		case KeyCode::KEY_ENTER: {
			BREAK_ON_NOT_DOWN();
			if (selectedIndex >= 0) {
				selectMenu((unsigned int) selectedIndex);
			}
			break;
		}
		case KeyCode::KEY_HOME: {
			BREAK_ON_NOT_DOWN();
			setSelectedItem(0);
			break;
		}
		case KeyCode::KEY_END: {
			BREAK_ON_NOT_DOWN();
			setSelectedItem(elementCount - 1);
			break;
		}
		default: {
			return SapphireUILayer::onKeyEventImpl();
		}
	}
	return true;
}

void SapphireMenu::selectMenu(unsigned int index) {
	auto ss = static_cast<SapphireScene*>(getScene());

	switch (elements[index].id) {
		case RXml::Id::sapphire_play: {
			if (!ss->isLevelsLoaded()) {
				showLoadingDialog(index);
				return;
			}
			DifficultySelectorLayer* layer = new DifficultySelectorLayer(this);
			layer->show(ss);

			break;
		}
		case RXml::Id::sapphire_community: {
			if (!ss->isLevelsLoaded()) {
				showLoadingDialog(index);
				return;
			}

			CommunityLayer* layer = new CommunityLayer(this);
			layer->show(ss);

			layer->showInitDialogs();
			break;
		}
		case RXml::Id::sapphire_settings: {
			SettingsLayer* layer = new SettingsLayer(this, ss->getSettings());
			layer->show(ss, true);
			break;
		}
		case RXml::Id::sapphire_leveleditor: {
			if (!ss->isLevelsLoaded()) {
				showLoadingDialog(index);
				return;
			}
			LevelEditorLayer* layer = new LevelEditorLayer(this);
			layer->show(ss);
			break;
		}
		case RXml::Id::sapphire_credits: {
			CreditsLayer* layer = new CreditsLayer(this);
			layer->show(ss, false);
			break;
		}
		default: {
			THROW()<< "unknown id " << elements[index].id;
			break;
		}
	}
}

bool SapphireMenu::touchImpl() {
	if (!showing || TouchEvent::instance.getPointerCount() > 1 || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		setSelectedItem(-1);
		return false;
	}

	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			setSelectedItem(-1);
			auto touchpos = TouchEvent::instance.getAffectedPointer()->getPosition();
			for (int i = 0; i < elementCount; ++i) {
				if (elementRects[i].isInside(touchpos)) {
					setSelectedItem(i);
					break;
				}
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (selectedIndex >= 0) {
				auto touchpos = TouchEvent::instance.getAffectedPointer()->getPosition();
				if (!elementRects[selectedIndex].isInside(touchpos)) {
					setSelectedItem(-1);
				}
			}
			break;
		}
		case TouchAction::UP: {
			if (selectedIndex >= 0) {
				selectMenu(selectedIndex);
				setSelectedItem(-1);
			}
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

bool SapphireMenu::addXmlChild(const xml::XmlNode& child) {
	return true;
}

void SapphireMenu::showLoadingDialog(unsigned int index) {
	class MenuLoadingDialog: public LoadingDialog {
		unsigned int index;
	protected:
		virtual void handleLoadingComplete() override {
			static_cast<SapphireMenu*>(getParent())->selectMenu(index);
		}
	public:
		MenuLoadingDialog(SapphireMenu* menu, unsigned int index)
				: LoadingDialog(menu), index(index) {
		}
	};

	DialogLayer* dialog = new MenuLoadingDialog(this, index);
	dialog->showDialog(getScene());
}

void* SapphireMenu::getXmlChild(const xml::XmlNode& child, const xml::XmlAttributes& attributes) {
	this->elements[child.index].applyAttributes(attributes, child.id);
	return this->elements + child.index;
}

void SapphireMenu::onSettingsChanged(const SapphireSettings& settings) {
	if (settings.is3DArtStyle()) {
		if (drawer3D == nullptr) {
			getScene()->getWindow()->foregroundTimeListeners += *this;
			drawer3D = new LevelDrawer3D();
			drawer3D->setOrthographic(true);
		}
	} else {
		if (drawer3D != nullptr) {
			TimeListener::unsubscribe();
			delete drawer3D;
			drawer3D = nullptr;

			for (int i = 0; i < elementCount; ++i) {
				bool selected = i == selectedIndex;
				iconAnimationValues[i] = selected ? 1.0f : 0.0f;
			}
		}
	}
}

void SapphireMenu::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	long long ms = (long long) (core::time_millis { time - previous });
	if (ms > 2000) {
		ms = 2000 + ms % 2000;
	}
	float sec = ms / 1000.0f;

	if (selectedIndex >= 0) {
		iconAnimationValues[selectedIndex] += sec;
		while (iconAnimationValues[selectedIndex] > 0.7f) {
			iconAnimationValues[selectedIndex] -= 0.7f;
		}
	}
	for (int i = 0; i < elementCount; ++i) {
		bool selected = i == selectedIndex;
		if (!selected && iconAnimationValues[i] > 0.0f) {
			iconAnimationValues[i] -= sec;
			if (iconAnimationValues[i] < 0.0f) {
				iconAnimationValues[i] = 0.0f;
			}
		}
	}
}

void SapphireMenu::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	auto* ss = static_cast<SapphireScene*>(scene);
	ss->setTopSapphireLayer(this);

	if (ss->getSettings().is3DArtStyle()) {
		scene->getWindow()->foregroundTimeListeners += *this;
		drawer3D = new LevelDrawer3D();
		drawer3D->setOrthographic(true);
	}
	ss->settingsChangedListeners += *this;

	(new SapphireSplashScreen(this))->show(scene);
}

} // namespace userapp
