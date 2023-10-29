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
 * SettingsLayer.cpp
 *
 *  Created on: 2016. apr. 17.
 *      Author: sipka
 */

#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/render/Texture.h>
#include <framework/render/RenderingContext.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/utility.h>
#include <gen/resources.h>
#include <appmain.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/SettingsLayer.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/dialogs/KeyMapDialog.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/dialogs/LinkProgressDialog.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>

#include <cstring>

using namespace rhfw;

namespace userapp {

static const char* GAMESCALE_LABELS[(unsigned int) GameScale::_count_of_entries] { "Smallest", "Small", "Normal", "Big", "Biggest" };
static const char* ARTSTYLE_LABELS[(unsigned int) SapphireArtStyle::_count_of_entries] { "Retro 2D", "3D", "3D Ortho" };
static const char* VOLUME_LEVEL_LABELS[11] { "0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%", };

#define MAKE_SOUND_ITEM(name, value) (new EnumDialogItem(1.0f, name, VOLUME_LEVEL_LABELS, value, 11))
#define TITLE_ADVANCED_SOUND "Advanced volumes"

class AdvancedSoundDialog: public DialogLayer {
public:
	AdvancedSoundDialog(SettingsLayer* parent)
			: DialogLayer(parent) {
		setTitle(TITLE_ADVANCED_SOUND);

		auto* yam = MAKE_SOUND_ITEM("Yam-yams", parent->settings.soundYamYam / 10);
		auto* laser = MAKE_SOUND_ITEM("Lasers", parent->settings.soundLaser / 10);
		auto* explosion = MAKE_SOUND_ITEM("Explosions", parent->settings.soundExplosion / 10);
		auto* convert = MAKE_SOUND_ITEM("Conversions", parent->settings.soundConvert / 10);
		auto* pickup = MAKE_SOUND_ITEM("Pick-ups", parent->settings.soundPickUp / 10);
		auto* doorusing = MAKE_SOUND_ITEM("Doors", parent->settings.soundDoorUsing / 10);
		auto* falling = MAKE_SOUND_ITEM("Drop off", parent->settings.soundFalling / 10);
		auto* enemies = MAKE_SOUND_ITEM("Enemies", parent->settings.soundEnemies / 10);
		auto* wheel = MAKE_SOUND_ITEM("Wheel", parent->settings.soundWheel / 10);
		auto* gems = MAKE_SOUND_ITEM("Gems", parent->settings.soundGems / 10);

		addDialogItem(yam);
		addDialogItem(laser);
		addDialogItem(explosion);
		addDialogItem(convert);
		addDialogItem(pickup);
		addDialogItem(doorusing);
		addDialogItem(falling);
		addDialogItem(enemies);
		addDialogItem(wheel);
		addDialogItem(gems);

		addDialogItem(new EmptyDialogItem(0.5f));
		addDialogItem(new CommandDialogItem { "Accept", [=] {
			parent->settings.soundYamYam = yam->getSelected() * 10;
			parent->settings.soundLaser = laser->getSelected() * 10;
			parent->settings.soundExplosion = explosion->getSelected() * 10;
			parent->settings.soundConvert = convert->getSelected() * 10;
			parent->settings.soundPickUp = pickup->getSelected() * 10;
			parent->settings.soundDoorUsing = doorusing->getSelected() * 10;
			parent->settings.soundFalling = falling->getSelected() * 10;
			parent->settings.soundEnemies = enemies->getSelected() * 10;
			parent->settings.soundWheel = wheel->getSelected() * 10;
			parent->settings.soundGems = gems->getSelected() * 10;
			dismiss();
		} });
		addDialogItem(new CommandDialogItem { "Cancel", [=] {
			dismiss();
		} });
	}
};

SettingsLayer::SettingsLayer(SapphireUILayer* parent, const SapphireSettings& settings)
		: DialogLayer(parent), settings(settings) {
	auto* ss = static_cast<SapphireScene*>(parent->getScene());
	setTitle("Settings");

	auto* uiscaleitem = new EnumDialogItem { 1, "UI scale", GAMESCALE_LABELS, (unsigned int) settings.uiScale,
			(unsigned int) GameScale::_count_of_entries };
	auto* gamescaleitem = new EnumDialogItem { 1, "Game scale", GAMESCALE_LABELS, (unsigned int) settings.gameScale,
			(unsigned int) GameScale::_count_of_entries };
	EnumDialogItem* fullscreenitem = nullptr;
	EnumDialogItem* rendereritem = nullptr;
	EnumDialogItem* antialiasitem = nullptr;
	EnumDialogItem* artstyleitem = new EnumDialogItem(1.0f, "Art style", ARTSTYLE_LABELS,
			(unsigned int) this->settings.artStyle - (unsigned int) SapphireArtStyle::MIN,
			(unsigned int) SapphireArtStyle::_count_of_entries);
	EnumDialogItem* titlemusicitem = new EnumDialogItem(1.0f, "Title music", ss->getAvailableMusicItems(),
			ss->getMusicIndexForName(this->settings.openingMusicName), ss->getAvailableMusicCount() + 1);
	auto* soundlevelitem = MAKE_SOUND_ITEM("Sound volume", this->settings.sound / 10);
	auto* musiclevelitem = MAKE_SOUND_ITEM("Music volume", this->settings.music / 10);
	titlemusicitem->setSelectionListener([=](unsigned int sel) {
		ss->setBackgroundMusic(sel == ss->getAvailableMusicCount() ? nullptr : ss->getAvailableMusicItems()[sel]);
	});

	addDialogItem(musiclevelitem);
	addDialogItem(soundlevelitem);
	addDialogItem(new CommandDialogItem( TITLE_ADVANCED_SOUND, [=] {
		AdvancedSoundDialog* asd = new AdvancedSoundDialog(this);
		asd->showDialog(getScene());
	}));
	addDialogItem(titlemusicitem);
	addDialogItem(artstyleitem);
#if !defined(SAPPHIRE_NO_ONSCREEN_CONTROLS)
	TickDialogItem* lefthandedtick = new TickDialogItem("Left handed touch controls", settings.leftHandedOnScreenControls);
	addDialogItem(lefthandedtick);
#endif /* defined(SAPPHIRE_NO_ONSCREEN_CONTROLS) */
	{
		auto* window = parent->getScene()->getWindow();
		unsigned int displayselectedindex = 0;
		if (window->supportsWindowStyle(WindowStyle::BORDERED)) {
			if (settings.fullScreenState == SapphireFullScreenState::WINDOWED) {
				displayselectedindex = displayModeCount;
			}
			displayModeIndexMap[displayModeCount] = SapphireFullScreenState::WINDOWED;
			displayModeLabels[displayModeCount++] = "Windowed";
		}
		if (window->supportsWindowStyle(WindowStyle::FULLSCREEN)) {
			if (settings.fullScreenState == SapphireFullScreenState::BORDERLESS_FULLSCREEN) {
				displayselectedindex = displayModeCount;
			}
			displayModeIndexMap[displayModeCount] = SapphireFullScreenState::BORDERLESS_FULLSCREEN;
			displayModeLabels[displayModeCount++] = "Borderless";
		}
		if (renderer->getRenderingContext()->supportsExclusiveFullScreen()) {
			if (settings.fullScreenState == SapphireFullScreenState::EXCLUSIVE_FULLSCREEN) {
				displayselectedindex = displayModeCount;
			}
			displayModeIndexMap[displayModeCount] = SapphireFullScreenState::EXCLUSIVE_FULLSCREEN;
			displayModeLabels[displayModeCount++] = "Fullscreen";
		}
		if (displayModeCount > 1) {
			fullscreenitem = new EnumDialogItem { 1, "Display mode", displayModeLabels, displayselectedindex, displayModeCount };
			addDialogItem(fullscreenitem);
		}
	}
	{
		if ((unsigned int) RenderConfig::_count_of_entries > 1) {
			rendereritem = new EnumDialogItem(1, "Renderer", RenderMap::instance.ids, (unsigned int) renderer->getRendererType(),
					(unsigned int) RenderConfig::_count_of_entries);
			addDialogItem(rendereritem);
		}
	}
	TickDialogItem* vsync = nullptr;

	if (HAS_FLAG(renderer->getRenderingContext()->getSupportedVSyncOptions(), VSyncOptions::VSYNC_ON)) {
		vsync = new TickDialogItem("VSync", this->settings.vsync);
		addDialogItem(vsync);
	}

	unsigned int maxmsaafactor = renderer->getRenderingContext()->getMaximumMultiSampleFactor();
	unsigned int currentmsaafactor = renderer->getRenderingContext()->getRenderingContextOptions().multiSamplingFactor;
	if (maxmsaafactor > 1) {
		unsigned int labelcount = 0;
		for (unsigned int c = maxmsaafactor; c > 1; c /= 2) {
			++labelcount;
		}
		static const int MAX_LABEL_LENGTH = 10;

		char* labels = new char[MAX_LABEL_LENGTH * labelcount];
		antiAliasLabels = labels;
		antiAliasLabelMap = new const char*[labelcount + 1];
		antiAliasLabelMap[0] = "Off";
		for (unsigned int i = 0, pow = 2; i < labelcount; ++i, pow *= 2) {
			antiAliasLabelMap[i + 1] = labels + i * MAX_LABEL_LENGTH;
			snprintf(labels + i * MAX_LABEL_LENGTH, MAX_LABEL_LENGTH, "MSAAx%u", pow);
		}
		unsigned int selected = 0;
		if (currentmsaafactor != 0) {
			unsigned int pow = currentmsaafactor;
			while (pow > 1) {
				pow /= 2;
				selected++;
			}
			if (selected > labelcount) {
				selected = labelcount - 1;
			}
		}
		antialiasitem = new EnumDialogItem(1, "Anti-aliasing", antiAliasLabelMap, selected, labelcount + 1);
		addDialogItem(antialiasitem);
	}

	gameScaleItem = gamescaleitem;

	addDialogItem(uiscaleitem);
	addDialogItem(gamescaleitem);

	if (ss->hasRecognizedGamePadAttached()) {
		gamePadControlsItem = new CommandDialogItem("Gamepad control settings", [=] {
			auto* controls = new KeyMapDialog(this, ss->getGamePadKeyMap(), InputDevice::GAMEPAD);
			controls->setTitle("Gamepad control settings");
			controls->showDialog(getScene());
		});
		addDialogItem(gamePadControlsItem);
	} else {
		if (gamepadContext != nullptr) {
			gamepadContext->addGamePadStateListener(this);
		}
	}
	if (ss->isKeyboardDetected()) {
		//only if has keyboard
		addDialogItem(new CommandDialogItem("Keyboard control settings", [=] {
			auto* controls = new KeyMapDialog(this, ss->getKeyMap(), InputDevice::KEYBOARD);
			controls->setTitle("Keyboard control settings");
			controls->showDialog(getScene());
		}));
	}

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	if(ss->isAppBorrowed()) {
		addDialogItem(new CommandDialogItem("Purchase " SAPPHIRE_GAME_NAME, [=] {
							ss->navigateToStorePageToPurchase();
						}));
	}
	TickDialogItem* steamachievementprogress = new TickDialogItem("Enable Steam achievement progress indicator",
			settings.steamAchievementProgressIndicatorEnabled);
	addDialogItem(steamachievementprogress);
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

	//not really used
//	sendFeedbackItem = new CommandDialogItem("Send feedback", [=] {
//		auto* dialog = new DialogLayer(this);
//		dialog->setTitle("Feedback");
//		auto* textitem = new EditTextDialogItem("Feedback:", nullptr);
//		textitem->setContentMaximumLength(SAPPHIRE_FEEDBACK_MAX_LEN);
//		textitem->setReturnHandler([=] {
//					sendFeedback(dialog, textitem, textitem->getContent(), textitem->getContentLength());
//					return true;
//				});
//		dialog->addDialogItem(new TextDialogItem("Thank you for taking the time to send us your feedback. It is very important to us!"));
//		dialog->addDialogItem(new EmptyDialogItem(0.4f));
//		dialog->addDialogItem(textitem);
//		dialog->addDialogItem(new EmptyDialogItem(0.5f));
//		dialog->addDialogItem(new CommandDialogItem( "Send", [=] {
//							sendFeedback(dialog, textitem, textitem->getContent(), textitem->getContentLength());
//						}));
//
//		dialog->showDialog(getScene());
//	});
//	addDialogItem(sendFeedbackItem);
	//linking not supported anymore. doesn't really work, might do it later
//	addDialogItem(
//			new CommandDialogItem( LINK_PROGRESS_DIALOG_TITLE,
//					[=] {
//					(new LinkProgressDialog(this))->show(ss, true);
//				}));
#if RHFW_DEBUG
	addDialogItem(new CommandDialogItem("Verify demos", [=] {
		ss->checkLevelDemos();
	}));
#endif /* RHFW_DEBUG */
	addDialogItem(new EmptyDialogItem { 0.5f });
	addDialogItem(new CommandDialogItem { "Apply", [=] {
		this->settings.music = musiclevelitem->getSelected() * 10;
		this->settings.sound = soundlevelitem->getSelected() * 10;
		this->settings.uiScale = (GameScale) uiscaleitem->getSelected();
		this->settings.gameScale = (GameScale) gamescaleitem->getSelected();
		if (vsync != nullptr) {
			this->settings.vsync = vsync->isTicked();
		}
		SapphireArtStyle nart = (SapphireArtStyle) (artstyleitem->getSelected() + (unsigned int)SapphireArtStyle::MIN);
		DialogLayer* info = nullptr;
		if(nart != this->settings.artStyle) {
			this->settings.artStyle = nart;
		}
		if(fullscreenitem != nullptr) {
			this->settings.fullScreenState = displayModeIndexMap[fullscreenitem->getSelected()];
		}
		if (antialiasitem != nullptr) {
			this->settings.multiSampleFactor = antialiasitem->getSelected() == 0 ? 0 : (1 << antialiasitem->getSelected());
		} else {
			this->settings.multiSampleFactor = currentmsaafactor;
		}
		this->settings.openingMusicName = titlemusicitem->getSelected() == ss->getAvailableMusicCount() ?
		nullptr : ss->getAvailableMusicItems()[titlemusicitem->getSelected()];
		{
#if !defined(SAPPHIRE_NO_ONSCREEN_CONTROLS)
			this->settings.leftHandedOnScreenControls = lefthandedtick->isTicked();
#endif /* defined(SAPPHIRE_NO_ONSCREEN_CONTROLS) */
		}
		{
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
			this->settings.steamAchievementProgressIndicatorEnabled = steamachievementprogress->isTicked();
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
		}
		ss->updateSettings(this->settings);

		if((unsigned int) RenderConfig::_count_of_entries > 1
				&& rendereritem->getSelected() != (unsigned int) renderer->getRendererType()) {
			changeRenderer((RenderConfig) rendereritem->getSelected());
			if (this->settings.fullScreenState == SapphireFullScreenState::EXCLUSIVE_FULLSCREEN
					&& !renderer->getRenderingContext()->supportsExclusiveFullScreen()) {
				this->settings.fullScreenState = SapphireFullScreenState::BORDERLESS_FULLSCREEN;
				ss->updateSettings(this->settings);
			}
		}

		ss->writeSettings();
		dismiss();
		if(info != nullptr) {
			info->showDialog(getScene());
		}
	} });
	addDialogItem(new CommandDialogItem { "Cancel", [=] {
		dismiss();
	} });
}
SettingsLayer::~SettingsLayer() {
	delete[] antiAliasLabels;
	delete[] antiAliasLabelMap;
	if (gamepadContext != nullptr && gamePadControlsItem == nullptr) {
		gamepadContext->removeGamePadStateListener(this);
	}
}

void SettingsLayer::onGamePadAttached(GamePad* gamepad) {
	gamepadContext->removeGamePadStateListener(this);

	gamePadControlsItem = new CommandDialogItem("Gamepad control settings", [=] {
		auto* controls = new KeyMapDialog(this, static_cast<SapphireScene*>(getScene())->getGamePadKeyMap(), InputDevice::GAMEPAD);
		controls->showDialog(getScene());
	});
	addDialogItemAfter(gameScaleItem, gamePadControlsItem);
	relayout();
}
void SettingsLayer::onGamePadDetached(GamePad* gamepad) {
}

void SettingsLayer::sendFeedback(DialogLayer* parent, DialogItem* fbitem, const char* content, unsigned int length) {
	if (length < 4) {
		DialogLayer* info = new DialogLayer(parent);
		info->setTitle("Information");
		info->addDialogItem(new TextDialogItem("Please provide a feedback!"));
		info->addDialogItem(new EmptyDialogItem(0.5f));
		info->addDialogItem(new CommandDialogItem("Back", [=] {
			info->dismiss();
			parent->setHighlighted(fbitem);
		}));

		info->showDialog(getScene());
	} else {
		DialogLayer* thanks = new ParentDismisserDialogLayer(parent);
		thanks->setTitle("Feedback sent");
		thanks->addDialogItem(new TextDialogItem("Thank you for your feedback!"));
		thanks->addDialogItem(new EmptyDialogItem(0.5f));
		thanks->addDialogItem(new CommandDialogItem("Back", [=] {
			thanks->dismiss();
		}));

		thanks->showDialog(getScene());
	}
}

} // namespace userapp

