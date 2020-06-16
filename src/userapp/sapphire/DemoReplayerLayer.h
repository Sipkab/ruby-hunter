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
 * DemoReplayerLayer.h
 *
 *  Created on: 2016. dec. 22.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DEMOREPLAYERLAYER_H_
#define TEST_SAPPHIRE_DEMOREPLAYERLAYER_H_

#include <sapphire/DemoLayer.h>

namespace userapp {
using namespace rhfw;
class LevelEditorLayer;
class EditTextDialogItem;

class DemoReplayerLayer final: public DemoLayer {
	int demoIndex = -1;

	unsigned int overTurn = 0;

	LevelEditorLayer* editorLayer = nullptr;

	bool playLevelAllowd = true;

	FixedString demoInfo;

	bool jumpTo(DialogLayer* layer, EditTextDialogItem* timeitem);
protected:
	virtual void onLevelEnded() override;
	virtual void showPausedDialog() override;

	virtual void goToNextTurn() override;
	virtual void goToPreviousTurn(unsigned int target) override;

	virtual bool isOver() override;
	virtual int getOverTurns() override;
public:
	DemoReplayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level, unsigned int demoindex);
	DemoReplayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level, FixedString steps,
			uint32 randomseed);
	~DemoReplayerLayer();

	void restartDemo();

	int getDemoIndex() const {
		return demoIndex;
	}

	const Demo* getDemo() {
		return this->level.getDemo(demoIndex);
	}

	void setLevelEditorLayer(LevelEditorLayer* layer) {
		this->editorLayer = layer;
	}

	virtual void setScene(Scene* scene) override;

	void setPlayLevelAllowed(bool allowed) {
		this->playLevelAllowd = allowed;
	}

	void setDemoInfo(FixedString info) {
		this->demoInfo = util::move(info);
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_DEMOREPLAYERLAYER_H_ */
