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
 * SapphireUILayer.cpp
 *
 *  Created on: 2016. aug. 14.
 *      Author: sipka
 */

#include <sapphire/SapphireUILayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/animation/PropertyAnimator.h>
#include <appmain.h>

namespace userapp {

#define ANIMATION_TIME 200

class SapphireUILayer::Hider: public PropertyAnimator<float> {
protected:
	virtual void onFinish() override {
		PropertyAnimator<float>::onFinish();
		layer->visible = false;
	}
	SapphireUILayer* layer;
public:
	Hider(SapphireUILayer* layer)
			: PropertyAnimator<float>(layer->displayPercent, 0.0f, core::MonotonicTime::getCurrent(), core::time_millis { ANIMATION_TIME }), layer(
					layer) {
	}
};
class SapphireUILayer::Dismisser: public PropertyAnimator<float> {
protected:
	virtual void onFinish() override {
		PropertyAnimator<float>::onFinish();
		if (layer->parent != nullptr && layer->parent->dialog) {
			layer->parent->dimPercent = property;
		}
		delete layer;
	}
	SapphireUILayer* layer;
public:
	Dismisser(SapphireUILayer* layer)
			: PropertyAnimator<float>(layer->displayPercent, 0.0f, core::MonotonicTime::getCurrent(), core::time_millis { ANIMATION_TIME }), layer(
					layer) {
	}
	virtual void onStart() override {
		PropertyAnimator<float>::onStart();
		layer->dimPercent = property;
	}
	virtual void onProgress(const core::time_millis& progress) override {
		PropertyAnimator<float>::onProgress(progress);
		if (layer->parent != nullptr && layer->parent->dialog) {
			layer->parent->dimPercent = property;
		}
	}
};
class SapphireUILayer::Shower: public PropertyAnimator<float> {
protected:
	virtual void onFinish() override {
		PropertyAnimator<float>::onFinish();
		if (layer->parent != nullptr && layer->parent->dialog) {
			layer->parent->dimPercent = 1.0f;
		}
		layer->dimPercent = 1.0f - property;
		if (!layer->gainedInput && layer->shouldHaveInput) {
			THROW()<< "We shouldnt be here";
			layer->gainedInput = true;
			layer->getScene()->getWindow()->takeKeyboardInput(layer);
			layer->onGainingInput();
		}
		layer->onShowingLayer();
	}
	SapphireUILayer* layer;
public:
	Shower(SapphireUILayer* layer)
			: PropertyAnimator<float>(layer->displayPercent, 1.0f, core::MonotonicTime::getCurrent(), core::time_millis { ANIMATION_TIME }), layer(
					layer) {
	}
	virtual void onStart() override {
		PropertyAnimator<float>::onStart();
		layer->dimPercent = 1.0f - property;
	}
	virtual void onProgress(const core::time_millis& progress) override {
		PropertyAnimator<float>::onProgress(progress);
		layer->dimPercent = 1.0f - property;
		if (layer->parent != nullptr && layer->parent->dialog) {
			layer->parent->dimPercent = property;
		}
	}
};
class SapphireUILayer::Displayer: public PropertyAnimator<float> {
protected:
	virtual void onFinish() override {
		PropertyAnimator<float>::onFinish();
		if (layer->parent != nullptr && layer->parent->dialog) {
			layer->parent->dimPercent = 1.0f;
		}
		layer->onShowingLayer();
	}
	SapphireUILayer* layer;
public:
	Displayer(SapphireUILayer* layer)
			: PropertyAnimator<float>(layer->displayPercent, 1.0f, core::MonotonicTime::getCurrent(), core::time_millis { ANIMATION_TIME }), layer(
					layer) {
	}
	virtual void onStart() override {
		PropertyAnimator<float>::onStart();
	}
	virtual void onProgress(const core::time_millis& progress) override {
		PropertyAnimator<float>::onProgress(progress);
		if (layer->parent != nullptr && layer->parent->dialog) {
			layer->parent->dimPercent = property;
		}
	}
};

template<typename HiderAnimator>
void SapphireUILayer::hideAnimate() {
	onHidingLayer();
	displayAnim.kill();

	auto* anim = new HiderAnimator(this);
	displayAnim.link(anim);
	anim->start();
	showing = false;
	shouldHaveInput = false;
	if (gainedInput) {
		gainedInput = false;
		onLosingInput();
	}
}
template<typename ShowerAnimator>
void SapphireUILayer::showAnimate() {
	visible = true;

	displayAnim.kill();

	auto* anim = new ShowerAnimator(this);
	displayAnim.link(anim);
	anim->start();
	showing = true;
	shouldHaveInput = true;
}
void SapphireUILayer::showLayer() {
	showAnimate<Shower>();
}
void SapphireUILayer::show(Scene* scene, bool dialog) {
	Layer::removeLinkFromList();
	scene->addLayerToTop(*this);

	showAnimate<Shower>();

	this->dialog = dialog;

	ASSERT(
			static_cast<SapphireScene*>(scene)->getTopSapphireLayer() == parent
					|| static_cast<SapphireScene*>(scene)->getTopSapphireLayer() == this);

	if (parent != nullptr) {
		if (!this->dialog) {
			for (auto* it = parent; it != nullptr; it = it->parent) {
				it->hideAnimate<Hider>();
				if (!it->dialog) {
					break;
				}
			}
		} else {
			//this is dialog, show parents
			for (auto* it = parent; it != nullptr; it = it->parent) {
				it->showAnimate<Displayer>();
				it->shouldHaveInput = false;
				if (it->gainedInput) {
					it->gainedInput = false;
					it->onLosingInput();
				}

				if (!it->dialog) {
					break;
				}
			}
		}
	}
	static_cast<SapphireScene*>(scene)->setTopSapphireLayer(this);

	if (this->getScene() != scene) {
		setScene(scene);
		//do not call sizeChanged if scene doesn't have dimensions yet.
		auto&& size = static_cast<SapphireScene*>(scene)->getUiSize();
		if (size.pixelSize.width() > 0 && size.pixelSize.height() > 0) {
			sizeChanged(static_cast<SapphireScene*>(scene)->getUiSize());
		}
		onSceneSizeInitialized();
	} else {
		static_cast<SapphireScene*>(scene)->setNeedBackground(isNeedBackground());
	}
	if (!gainedInput) {
		gainedInput = true;
		scene->getWindow()->takeKeyboardInput(this);
		onGainingInput();
	}
	if (static_cast<SapphireScene*>(scene)->isLastInteractionKeyboard()) {
		displayKeyboardSelection();
	} else {
		hideKeyboardSelection();
	}
}
void SapphireUILayer::dismiss() {

	KeyEventListener::unsubscribe();

	hideAnimate<Dismisser>();
	ASSERT(static_cast<SapphireScene*>(getScene())->getTopSapphireLayer() == this);

	static_cast<SapphireScene*>(getScene())->setTopSapphireLayer(parent);
	if (parent != nullptr) {
		if (!dialog) {
			parent->show(scene, parent->dialog);
		} else {
			parent->shouldHaveInput = true;
			if (!parent->gainedInput) {
				parent->gainedInput = true;
				parent->getScene()->getWindow()->takeKeyboardInput(parent);
				parent->onGainingInput();
			}
			if (static_cast<SapphireScene*>(getScene())->isLastInteractionKeyboard()) {
				parent->displayKeyboardSelection();
			} else {
				parent->hideKeyboardSelection();
			}
		}
	}
}

bool SapphireUILayer::onKeyEvent() {
	KeyCode keycode = KeyEvent::instance.getKeycode();
	auto inputdevice = KeyEvent::instance.getInputDevice();
	if ((inputdevice == InputDevice::KEYBOARD || inputdevice == InputDevice::GAMEPAD) && keycode != KeyCode::KEY_BACK
			&& keycode != KeyCode::KEY_VOLUME_UP && keycode != KeyCode::KEY_VOLUME_DOWN) {
		static_cast<SapphireScene*>(getScene())->setLastInteractionKeyboard(true);
	}
	if (!shouldHaveInput) {
		return false;
	}
	if (onKeyEventImpl()) {
		return true;
	}
	switch (keycode) {
		case KeyCode::KEY_GAMEPAD_B:
		case KeyCode::KEY_GAMEPAD_BACK:
		case KeyCode::KEY_ESC:
		case KeyCode::KEY_BACKSPACE:
		case KeyCode::KEY_BACK: {
			if (KeyEvent::instance.getAction() == KeyAction::DOWN) {
				if (!KeyEvent::instance.isRepeat()) {
					return onBackRequested();
				}
				return true;
			}
			return false;
		}
		default: {
			return false;
		}
	}
	return true;
}
Layer* SapphireUILayer::touch() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	ss->setLastInteractionTouch(TouchEvent::instance.getAction(),
			TouchEvent::instance.getAction() == TouchAction::HOVER_MOVE ?
					TouchEvent::instance.getAffectedPointer()->getPosition() : TouchEvent::instance.getCenter());
	if (!hasInputFocus()) {
		return nullptr;
	}
	if (TouchEvent::instance.getAction() == TouchAction::HOVER_MOVE) {
		//ignore hover actions
		return nullptr;
	}
	if (touchImpl()) {
		return this;
	}
	return Layer::touch();
}

void SapphireUILayer::setColors(SapphireDifficulty diff) {
	setColors(difficultyToColor(diff), difficultyToSelectedColor(diff));
}

void SapphireUILayer::setScene(Scene* scene) {
	static_cast<SapphireScene*>(scene)->setNeedBackground(isNeedBackground());
	Layer::setScene(scene);
	core::WindowAccesStateListener::unsubscribe();
	KeyEventListener::unsubscribe();
	auto* prevkt = scene->getWindow()->getKeyboardTarget();
	if (prevkt != nullptr) {
		//prevKeyboardTarget.link(prevkt);
	}
	scene->getWindow()->keyListeners += *this;
	scene->getWindow()->accesStateListeners += *this;
	scene->getWindow()->takeKeyboardInput(this);
}

bool SapphireUILayer::onBackRequested() {
	if (parent != nullptr) {
		dismiss();
		return true;
	} else {
		//no parent, show exit dialog
		DialogLayer* dialog = new DialogLayer(this);
		dialog->setTitle("Exit game");
		dialog->addDialogItem(new TextDialogItem("Are you sure you want to quit " SAPPHIRE_GAME_NAME "?"));
		dialog->addDialogItem(new EmptyDialogItem(0.5f));
		dialog->addDialogItem(new CommandDialogItem("No", [=] {
			dialog->dismiss();
		}));
		dialog->addDialogItem(new CommandDialogItem("Yes", [=] {
			dialog->dismiss();
			scene->getWindow()->close();
		}));
		dialog->show(getScene(), true);
	}
	return true;
}

const char* SapphireUILayer::getNextReturnText() const {
	for (auto* it = parent; it != nullptr; it = it->parent) {
		if (it->returnText != nullptr) {
			return it->returnText;
		}
	}
	return "Return to main menu";
}
} // namespace userapp

