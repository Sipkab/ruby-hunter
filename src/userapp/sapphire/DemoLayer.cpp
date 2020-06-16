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
 * DemoLayer.cpp
 *
 *  Created on: 2016. apr. 28.
 *      Author: sipka
 */

#include <framework/geometry/Matrix.h>
#include <framework/geometry/Vector.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/layer/Layer.h>
#include <framework/render/Renderer.h>
#include <framework/utils/utility.h>
#include <gen/types.h>
#include <appmain.h>
#include <sapphire/DemoLayer.h>
#include <sapphire/SapphireScene.h>

#include <gen/log.h>

namespace userapp {

using namespace rhfw;

DemoLayer::DemoLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level&& level)
		: SapphireUILayer(parent), descriptor(descriptor), levelPrototype(level), level(util::move(level)) {
	setNeedBackground(false);
	setColors(this->level.getInfo().difficulty);
}
DemoLayer::~DemoLayer() {
	if (levelBackwardLoaderTask != nullptr) {
		levelBackwardLoaderTask->cancel();
		delete levelBackwardLoaderTask;
	}
	delete backwardCacheProto;
	delete backwardNextCache;
}

bool DemoLayer::touchImpl() {
	if (!showing || !gainedInput || TouchEvent::instance.getPointerCount() > 1 || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		menuTouch = nullptr;
		demoController.cancelTouch();
		return false;
	}
	auto* affected = TouchEvent::instance.getAffectedPointer();
	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			auto touchpos = affected->getPosition();

			if (menuTouch == nullptr && menuRect.isInside(touchpos)) {
				menuTouch = affected;
				return true;
			} else {
				return demoController.onTouchEvent();
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (menuTouch != nullptr) {
				if (!menuRect.isInside(menuTouch->getPosition())) {
					menuTouch = nullptr;
				}
			} else {
				return demoController.onTouchEvent();
			}
			break;
		}
		case TouchAction::UP: {
			if (affected == menuTouch) {
				menuTouch = nullptr;
				showPausedDialog();
			} else {
				return demoController.onTouchEvent();
			}
			break;
		}
		default: {
			break;
		}
	}
	return menuTouch != nullptr;
}

bool DemoLayer::onKeyEventImpl() {
	if (demoController.onKeyEvent()) {
		return true;
	}
	if (KeyEvent::instance.getAction() == KeyAction::DOWN) {
		KeyCode kc = KeyEvent::instance.getKeycode();
		switch (kc) {
			case KeyCode::KEY_GAMEPAD_START: {
				showPausedDialog();
				return true;
			}
			default: {
				break;
			}
		}
	}
	return false;
}

void DemoLayer::drawHud(float displaypercent) {
	hudDrawer.draw(level, displaypercent);
}

void DemoLayer::drawImpl(float displayPercent) {
	Matrix2D mvp;
	mvp.setScreenDimension(drawer.getSize().pixelSize);

	drawer.draw(this->turnPercent, displayPercent);
	drawHud(displayPercent);

	drawDemoRelated(displayPercent);

	demoController.draw(this->turnPercent, displayPercent);

#ifdef SAPPHIRE_SCREENSHOT_MODE
	return;
#endif /* defined(SAPPHIRE_SCREENSHOT_MODE) */

	if (menuTouch != nullptr) {
		drawRectangleColor(mvp, Color { getUiColor().xyz(), displayPercent }, menuRect);
	}
	drawSapphireTexture(mvp, menuTexture,
			menuTouch != nullptr ? Color { getUiSelectedColor().xyz(), displayPercent } : Color { getUiColor().xyz(), displayPercent },
			menuRect, Rectangle { 0, 0, 1, 1 });

}

void DemoLayer::sizeChanged(const core::WindowSize& size) {
	demoController.onSizeChanged(size);

	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
	hudDrawer.setSize(size, ss->getUiUserScale());

	drawer.setPaddings(demoController.getPaddings());

	float menuicondimcm = min(min(size.getPhysicalWidth() / 10.0f, size.getPhysicalHeight() / 8.0f), 1.5f);
	Size2F menuicondim = size.toPixels(Size2F { menuicondimcm, menuicondimcm });
	menuRect = Rectangle { size.pixelSize.width() - menuicondim.width(), 0, (float) size.pixelSize.width(), menuicondim.height() };
}

void DemoLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	scene->getWindow()->foregroundTimeListeners += *this;
	demoController.setScene(static_cast<SapphireScene*>(scene));

	if (descriptor != nullptr) {
		static_cast<SapphireScene*>(scene)->updateLevelState(descriptor, LevelState::UNFINISHED);
	}
	sounder.attachToScene(static_cast<SapphireScene*>(scene));

	SapphireScene* ss = static_cast<SapphireScene*>(scene);

	hudDrawer.setScene(ss);

	if (!drawer.hasDrawer()) {
		drawer.setDrawer(ss->getSettings().getArtStyleForLevel(level.getInfo().difficulty));
	}
	ss->settingsChangedListeners += *this;
}

void DemoLayer::onSettingsChanged(const SapphireSettings& settings) {
	drawer.setDrawer(settings.getArtStyleForLevel(level.getInfo().difficulty));
	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
}

void DemoLayer::onVisibilityToUserChanged(core::Window& window, bool visible) {
	if (!visible && hasInputFocus()) {
		showPausedDialog();
	}
}

bool DemoLayer::onBackRequested() {
	showPausedDialog();
	return true;
}

void DemoLayer::advanceMilliseconds(long long ms) {
	long long turnms = demoController.getTurnTime();
	if (turnms == 0) {
		if (pausedTurns == 0) {
			if (turnPercent < 1.0f) {
				long long prevturnms = demoController.getPreviousTurnTime();
				float msfloat = (float) ms / prevturnms;
				turnPercent += msfloat;
				if (prevturnms > 0) {
					if (turnPercent > 1.0f) {
						float diff = turnPercent - 1.0f;
						turnPercent = 1.0f;
						//advance the remaining to disable step-by-step lagging
						advanceMilliseconds((long long) (diff * prevturnms));
					}
				} else {
					if (turnPercent < 0.0f) {
						if (level.getTurn() > 0) {
							goToPreviousTurn(level.getTurn() - 1);
							turnPercent = 1.0f;
						} else {
							float tpms = (long long) (turnPercent * prevturnms);
							onPausedIdle(tpms);
							turnPercent = 0.0f;
						}
					}
				}
			} else {
				onPausedIdle(ms);
				if (pausedTurns > 0) {
					advanceMilliseconds(ms);
				}
			}
		} else {
			demoController.setPreviousTurnTime(SAPPHIRE_TURN_MILLIS);
			pausedTurns -= advanceTurns(ms, SAPPHIRE_TURN_MILLIS, pausedTurns);
		}
	} else {
		pausedTurns = 0;
		if (turnms > 0) {
			advanceTurns(ms, turnms);
		} else {
			turnPercent += (float) ms / turnms;
			unsigned int targetturn = level.getTurn();
			while (turnPercent < 0.0f) {
				if (targetturn > 0) {
					--targetturn;
					turnPercent += 1.0f;
				} else {
					turnPercent = 0.0f;
					break;
				}
			}
			if (targetturn != level.getTurn()) {
				goToPreviousTurn(targetturn);
			}
		}
	}
}
void DemoLayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (!hasInputFocus() || demoController.isControlsVisible()) {
		return;
	}
	long long ms = (long long) (time - previous);
	advanceMilliseconds(ms);
}
#define CACHE_STEP_RATE 96
unsigned int DemoLayer::advanceTurns(long long ms, long long turnms, unsigned int maxturns) {
	unsigned int result = 0;
	turnPercent += (float) ms / turnms;
	while (turnPercent >= 1.0f && result < maxturns) {
		++result;
		turnPercent -= 1.0f;
		goToNextTurn();

		sounder.playSoundsForTurn();
		int overturn = getOverTurns();
		if (overturn > 0 && level.getTurn() - overturn == 6) {
			onLevelEnded();
		}
		advanceUpdateBackwardCache();
	}
	return result;
}
void DemoLayer::advanceUpdateBackwardCache() {
	if (level.getTurn() == 0 || level.getTurn() % CACHE_STEP_RATE != 0) {
		return;
	}
	if (backwardCacheProto != nullptr) {
		if (backwardCacheProto->getTurn() != level.getTurn()) {
			if (levelBackwardLoaderTask == nullptr) {
				auto* temp = backwardCacheProto;
				backwardCacheProto = backwardNextCache;
				backwardNextCache = temp;
			}
			*backwardCacheProto = level;
		}
	} else {
		backwardCacheProto = new Level(level);
		backwardNextCache = new Level(level);
	}
}

void DemoLayer::rollbackToTurn(unsigned int turn) {
	ASSERT(turn < level.getTurn());
//	LOGI()<< "Rolling back to turn: " << turn;
	if (backwardCacheProto != nullptr) {
//		LOGI()<< "Caches: " << backwardCacheProto->getTurn() << " - " << backwardNextCache->getTurn() << (levelBackwardLoaderTask != nullptr ? " With task running" : "");
		//task can only be non-null when we already allocated cache
		if (levelBackwardLoaderTask != nullptr) {
			if (backwardLoaderCacheTargetTurn > turn) {
				levelBackwardLoaderTask->cancel();
				delete levelBackwardLoaderTask;
				levelBackwardLoaderTask = nullptr;
				*backwardNextCache = levelPrototype;
			}
		} else {
			if (backwardNextCache->getTurn() > turn) {
				*backwardNextCache = levelPrototype;
			}
		}
		if (backwardCacheProto->getTurn() > turn) {
			//we have to discard the cache, since we rewound more than it caches
			*backwardCacheProto = levelPrototype;
			if (levelBackwardLoaderTask == nullptr) {
				auto* temp = backwardCacheProto;
				backwardCacheProto = backwardNextCache;
				backwardNextCache = temp;
			}
		}
//		LOGI()<< "Caches adjusted: " << backwardCacheProto->getTurn() << " - " << backwardNextCache->getTurn();
	}

	unsigned int targetcache = turn - (turn % CACHE_STEP_RATE);
	Level* cache;
	if (targetcache < CACHE_STEP_RATE) {
		cache = &levelPrototype;
	} else {
		//recache
		if (backwardCacheProto == nullptr) {
			backwardCacheProto = new Level(levelPrototype);
			backwardNextCache = new Level(levelPrototype);
			ASSERT(targetcache <= demoController.getDemoStepsLength());
			playLevelTurns(demoController.getDemoSteps(), demoController.getDemoStepsLength(), targetcache, *backwardCacheProto);
			//DemoPlayer::playMoves(demoController.getDemoSteps(), targetcache, *backwardCacheProto);
		} else {
			if (backwardCacheProto->getTurn() < targetcache) {
				//play the remaining
				ASSERT(targetcache <= demoController.getDemoStepsLength());

				unsigned int offset = backwardCacheProto->getTurn() * level.getPlayerCount();
				playLevelTurns(demoController.getDemoSteps() + offset, demoController.getDemoStepsLength() - offset,
						targetcache - backwardCacheProto->getTurn(), *backwardCacheProto);
//				DemoPlayer::playMoves(demoController.getDemoSteps() + offset, targetcache - backwardCacheProto->getTurn(),
//						*backwardCacheProto);
			} else if (backwardCacheProto->getTurn() > targetcache) {
				if (levelBackwardLoaderTask != nullptr) {
					//task is loading some cache
					if (backwardLoaderCacheTargetTurn <= targetcache) {
						//loading a viable cache
						levelBackwardLoaderTask->finish();
					} else {
						//loading a non viable cache, cancel
						levelBackwardLoaderTask->cancel();
					}
					delete levelBackwardLoaderTask;
					levelBackwardLoaderTask = nullptr;
				}

				if (backwardNextCache->getTurn() <= targetcache) {
					//if backwardNextCache->getTurn() == targetcache: found good cache
					auto* temp = backwardCacheProto;
					backwardCacheProto = backwardNextCache;
					backwardNextCache = temp;

					if (backwardCacheProto->getTurn() < targetcache) {
						//potential cache, needs more turns
						unsigned int offset = backwardCacheProto->getTurn() * level.getPlayerCount();
						playLevelTurns(demoController.getDemoSteps() + offset, demoController.getDemoStepsLength() - offset,
								targetcache - backwardCacheProto->getTurn(), *backwardCacheProto);
//						DemoPlayer::playMoves(demoController.getDemoSteps() + offset, targetcache - backwardCacheProto->getTurn(),
//								*backwardCacheProto);
					}
				} else {
					//found no good cache, has to load something now
					*backwardCacheProto = levelPrototype;
					playLevelTurns(demoController.getDemoSteps(), demoController.getDemoStepsLength(), targetcache, *backwardCacheProto);
//					DemoPlayer::playMoves(demoController.getDemoSteps(), targetcache, *backwardCacheProto);
				}
			}
		}
		cache = backwardCacheProto;
		if (targetcache > CACHE_STEP_RATE) {
			startCacheLoaderTask(targetcache - CACHE_STEP_RATE);
		}
	}
	ASSERT(turn >= cache->getTurn());
	level = *cache;
//	LOGI()<< "Rewind to turn: " << turn << " using cache: " << level.getTurn();

	unsigned int offset = cache->getTurn() * level.getPlayerCount();
	playLevelTurns(demoController.getDemoSteps() + offset, demoController.getDemoStepsLength() - offset, turn - cache->getTurn(),
			this->level);

//	DemoPlayer::playMoves(demoController.getDemoSteps() + cache->getTurn() * level.getPlayerCount(), turn - cache->getTurn(), this->level);
}
void DemoLayer::startCacheLoaderTask(unsigned int cacheturn) {
	ASSERT(cacheturn > 0);
	ASSERT((cacheturn % CACHE_STEP_RATE) == 0);
	if (levelBackwardLoaderTask != nullptr) {
		if (backwardLoaderCacheTargetTurn == cacheturn) {
			return;
		}
		levelBackwardLoaderTask->cancel();
		delete levelBackwardLoaderTask;
		levelBackwardLoaderTask = nullptr;
	}
	if (backwardNextCache->getTurn() == cacheturn) {
		return;
	}
//	LOGI()<< "Starting async load to: " << cacheturn;
	backwardLoaderCacheTargetTurn = cacheturn;
	char* moves;
	unsigned int moveslen;
	if (backwardNextCache->getTurn() > cacheturn) {
		ASSERT(demoController.getDemoStepsLength() >= cacheturn * level.getPlayerCount());

		moveslen = cacheturn * level.getPlayerCount();
		moves = new char[moveslen];
		memcpy(moves, demoController.getDemoSteps(), cacheturn * level.getPlayerCount());
	} else {
		moveslen = (cacheturn - backwardNextCache->getTurn()) * level.getPlayerCount();
		moves = new char[moveslen];
		memcpy(moves, demoController.getDemoSteps() + backwardNextCache->getTurn() * level.getPlayerCount(),
				(cacheturn - backwardNextCache->getTurn()) * level.getPlayerCount());
	}
	levelBackwardLoaderTask = new AsynchronTask([=]() mutable {
		/*task*/
		/*LOGI() << "Async loading cache: " << cacheturn;*/
		if(backwardNextCache->getTurn() > cacheturn) {
			*backwardNextCache = levelPrototype;
		} else {
			cacheturn -= backwardNextCache->getTurn();
		}
		const char* movesptr = moves;
		while(cacheturn > 0) {
			if(levelBackwardLoaderTask->isCanceled()) {
				LOGI() << "Cancel Async loading";
				break;
			}
			playLevelTurns(movesptr, moveslen, CACHE_STEP_RATE, *backwardNextCache);
//			DemoPlayer::playMoves(movesptr, CACHE_STEP_RATE, *backwardNextCache);
			movesptr += CACHE_STEP_RATE * level.getPlayerCount();
			moveslen -= CACHE_STEP_RATE * level.getPlayerCount();
			cacheturn -= CACHE_STEP_RATE;
		}
		/*LOGI() << "Async load: " << backwardNextCache->getTurn() << (levelBackwardLoaderTask->isCanceled() ? " canceled." : " done.");*/
		delete[] moves;
	}, [=] {
		/*finish*/
		delete levelBackwardLoaderTask;
		levelBackwardLoaderTask = nullptr;
		backwardLoaderCacheTargetTurn = 0;
	}, [=] {
		/*cancel*/
		backwardLoaderCacheTargetTurn = 0;
	});

	levelBackwardLoaderTask->start(false);
//	LOGI()<< "Launched async load to: " << cacheturn;
}

void DemoLayer::advanceNextTurnPaused() {
	ASSERT(demoController.getTurnTime() == 0);
	++pausedTurns;
}

void DemoLayer::setRandomSeed(unsigned int seed) {
	level.setRandomSeed(seed);
	levelPrototype.setRandomSeed(seed);
	if (backwardCacheProto != nullptr) {
		if (levelBackwardLoaderTask != nullptr) {
			levelBackwardLoaderTask->cancel();
			delete levelBackwardLoaderTask;
			levelBackwardLoaderTask = nullptr;
		}
		backwardCacheProto->setRandomSeed(seed);
		backwardNextCache->setRandomSeed(seed);
	}
}

void DemoLayer::setPaddings(const Rectangle& paddings) {
	drawer.setPaddings(demoController.getPaddings() + paddings);
}

void DemoLayer::playLevelTurns(const char* steps, unsigned int stepslength, unsigned int turncount, Level& level) {
	unsigned int pcount = level.getPlayerCount();
	if (stepslength < turncount * pcount) {
		//play as many as we can
		unsigned int turns = stepslength / pcount;
		DemoPlayer::playMoves(steps, turns, level);
		turncount -= turns;
		level.clearControls();
		while (turncount-- > 0) {
			level.applyTurn();
		}
	} else {
		DemoPlayer::playMoves(steps, turncount, level);
	}
}

void DemoLayer::onLosingInput() {
	SapphireUILayer::onLosingInput();
	steamOverlayCallback.Unregister();
}

void DemoLayer::onGainingInput() {
	SapphireUILayer::onGainingInput();
	steamOverlayCallback.Register(this, &DemoLayer::OnGameOverlayActivated);
}

void DemoLayer::onInputFocusChanged(core::Window& window, bool inputFocused) {
	if (!inputFocused && hasInputFocus()) {
		showPausedDialog();
	}
}

void DemoLayer::goToPreviousTurn(unsigned int target) {
	rollbackToTurn(target);
}

void DemoLayer::goToTurn(unsigned int turn) {
	if (turn < level.getTurn()) {
		rollbackToTurn(turn);
	} else {
		unsigned int maxcache = turn - turn % CACHE_STEP_RATE;
		//go forward
		while (level.getTurn() < turn) {
			goToNextTurn();
			if (level.getTurn() == maxcache) {
				advanceUpdateBackwardCache();
			}
		}
	}
	turnPercent = 1.0f;
}

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)

void DemoLayer::OnGameOverlayActivated(GameOverlayActivated_t* pParam) {
	if (pParam->m_bActive) {
		showPausedDialog();
	}
}

#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

}
// namespace userapp

