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
 * CommunityLayer.h
 *
 *  Created on: 2016. szept. 20.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_COMMUNITY_COMMUNITYLAYER_H_
#define TEST_SAPPHIRE_COMMUNITY_COMMUNITYLAYER_H_

#include <framework/animation/Animation.h>
#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/LifeCycleChain.h>
#include <gen/types.h>
#include <gen/resources.h>
#include <gen/shader/SimpleFontShader.h>
#include <appmain.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/SapphireUILayer.h>
#include <QuadIndexBuffer.h>
#include <sapphire/FastFontDrawer.h>
#include <sapphire/community/CommunityConnection.h>
#include <sapphire/SapphireScene.h>
#include <framework/utils/ContainerLinkedNode.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>

namespace userapp {
using namespace rhfw;
class SapphireLevelDescriptor;
class AsynchronTask;
class LevelsCommunityPage;
class DiscussionCommunityPage;

class CommunityLayer: public SapphireUILayer, private SapphireScene::SettinsChangedListener, private CommunityConnection::StateListener {
	friend class LevelsCommunityPage;
	friend class DiscussionCommunityPage;
private:
	class CommunityPage {
	protected:
		SapphireScene* scene;
		CommunityLayer* parent;
	public:
		CommunityPage(SapphireScene* scene, CommunityLayer* parent)
				: scene(scene), parent(parent) {
		}
		virtual ~CommunityPage() {
		}

		virtual void draw(const Matrix2D& mvp, float alpha) = 0;
		virtual void postFontDraw(const Matrix2D& mvp, float alpha) {
		}
		virtual bool onKeyEvent() = 0;
		virtual void touch(TouchAction action, const Vector2F& touchpos) = 0;
		virtual void sizeChanged(const rhfw::core::WindowSize& size) = 0;

		virtual void displayKeyboardSelection() {
		}
		virtual void hideKeyboardSelection() {
		}
		virtual void onCommunityConnected() {
		}
		virtual void onCommunityDisconnected() {
		}
		virtual void destroying() {
		}
		virtual const char* getTitle() = 0;

		virtual void onDisplayingPage() {
		}
		virtual void onDismissingPage() {
		}
	};

	static const unsigned int PAGE_COUNT = 2;
	rhfw::AutoResource<rhfw::render::Texture> backIconWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_arrow_back_white);
	rhfw::AutoResource<rhfw::render::Texture> networkConnectIconWhite = getTexture(
			rhfw::ResIds::gameres::game_sapphire::art::ic_network_connect);
	rhfw::AutoResource<rhfw::render::Texture> networkDisconnectIconWhite = getTexture(
			rhfw::ResIds::gameres::game_sapphire::art::ic_network_disconnect);
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	Rectangle backButtonPos;
	TouchPointer* backTouch = nullptr;

	float titleTextSize = 0.0f;

	char helloStringBuffer[SAPPHIRE_USERNAME_MAX_LEN + 32];

	float pageAlphas[PAGE_COUNT] { 1.0f, 0.0f };
	Rectangle pageTitleRectangles[PAGE_COUNT];
	Rectangle pageTitleDisplayRectangles[PAGE_COUNT];
	TouchPointer* pageTitleTouch[PAGE_COUNT] { nullptr };
	LifeCycleChain<Animation> pageAnimations[PAGE_COUNT];
	unsigned int currentPage = 0;

	CommunityPage* pages[PAGE_COUNT] { nullptr };
	CommunityPage* pageTouch = nullptr;
	TouchPointer* pageTouchPointer = nullptr;

	TouchPointer* helloTouch = nullptr;
	TouchPointer* connectionTouch = nullptr;
	Rectangle helloRectangle;
	Rectangle connectionRectangle;

	SapphireDifficulty uiDifficultyColor;
	core::time_millis connectedTime;
	SapphireScene::LevelStateListener::Listener levelStateListener = SapphireScene::LevelStateListener::make_listener(
			[=](const SapphireLevelDescriptor* desc, LevelState state) {
				SapphireScene* ss = static_cast<SapphireScene*>(getScene());
				auto diffcolor = ss->getUserDifficultyColor();
				if(diffcolor != uiDifficultyColor) {
					ss->getConnection().connect(ss);
					ss->getConnection().updatePlayerData(ss->getCurrentUserName(), diffcolor);
					uiDifficultyColor = diffcolor;
					setColors(uiDifficultyColor);
				}
			});
	CommunityConnection::CommunityInformationListener::Listener informationListener =
			CommunityConnection::CommunityInformationListener::make_listener([=](SapphireCommunityInformation info) {
				if(static_cast<SapphireScene*>(getScene())->getTopSapphireLayer() == this) {
					static_cast<SapphireScene*>(getScene())->getConnection().showCommunityInformationDialog(info, this);
				}
			});

	virtual void onConnected(CommunityConnection* connection) override;
	virtual void onLoggedIn(CommunityConnection* connection) override;
	virtual void onDisconnected(CommunityConnection* connection) override;
	virtual void onConnectionFailed(CommunityConnection* connection) override;

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	void fillHelloStringBuffer();

	void relayout(const core::WindowSize& size);

	void setCurrentPage(unsigned int index);

	void showUserDetailsEditDialog();
	void showShortNicknameInfoDialog(SapphireUILayer* parent);
	void showNickNameRequestDialog();

	FastFontDrawerPool fontDrawerPool;
	FastFontDrawer fontDrawer { fontDrawerPool };

	FastFontDrawer& getFontDrawer() {
		return fontDrawer;
	}

	const Color& getSelectedUiColor(bool highlight) {
		return highlight ? getUiSelectedColor() : SapphireUILayer::getUiColor();
	}

	float adjustSoftKeyboardTranslation(float y);

public:
	CommunityLayer(SapphireUILayer* parent);
	~CommunityLayer();

	virtual void drawImpl(float displaypercent) override;

	virtual bool onKeyEventImpl() override;

	virtual void displayKeyboardSelection() override {
		pages[currentPage]->displayKeyboardSelection();
	}
	virtual void hideKeyboardSelection() override {
		pages[currentPage]->hideKeyboardSelection();
	}

	virtual bool touchImpl() override;

	virtual void sizeChanged(const rhfw::core::WindowSize& size) override;

	virtual void setScene(Scene* scene) override;

	virtual void dismiss() override;

	void showInitDialogs();
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_COMMUNITY_COMMUNITYLAYER_H_ */
