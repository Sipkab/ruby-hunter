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
 * LevelDrawer2D.h
 *
 *  Created on: 2016. apr. 26.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_LEVELDRAWER2D_H_
#define TEST_SAPPHIRE_LEVEL_LEVELDRAWER2D_H_

#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <framework/geometry/Matrix.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <gen/resources.h>
#include <gen/assets.h>
#include <appmain.h>

namespace userapp {
class Level;
} // namespace userapp

namespace userapp {

class LevelDrawer2D: public LevelDrawer::DrawerImpl {
private:
	class MinerAnimations {
	public:
		AutoResource<FrameAnimation> walkleftanim;
		AutoResource<FrameAnimation> walkdownanim;
		AutoResource<FrameAnimation> walkupanim;
		AutoResource<FrameAnimation> anim;
		AutoResource<FrameAnimation> pushleftanim;
		AutoResource<FrameAnimation> digleftanim;
		AutoResource<FrameAnimation> digdownanim;
		AutoResource<FrameAnimation> digupanim;
	};
	MinerAnimations man1Animations { //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1walklft_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1walkdwn_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1walkup_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1man_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1pushlft_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1diglft_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1digdwn_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_1digup_anim) //
	};
	MinerAnimations man2Animations { //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2walklft_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2walkdwn_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2walkup_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2man_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2pushlft_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2diglft_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2digdwn_anim), //
	getAnimation(ResIds::build::sipka_rh_texture_convert::_2digup_anim) //
	};

	AutoResource<FrameAnimation> acidanim = getAnimation(ResIds::build::sipka_rh_texture_convert::acid_anim);
	AutoResource<FrameAnimation> acidleftanim = getAnimation(ResIds::build::sipka_rh_texture_convert::acidright);
	AutoResource<FrameAnimation> acidrightanim = getAnimation(ResIds::build::sipka_rh_texture_convert::acidleft);
	AutoResource<FrameAnimation> acidbothanim = getAnimation(ResIds::build::sipka_rh_texture_convert::acidboth);
	AutoResource<FrameAnimation> explosionanim = getAnimation(ResIds::build::sipka_rh_texture_convert::explode_anim);

	AutoResource<FrameAnimation> laservertanim = getAnimation(ResIds::build::sipka_rh_texture_convert::laserv_anim);
	AutoResource<FrameAnimation> laserbotrightanim = getAnimation(ResIds::build::sipka_rh_texture_convert::laserbr_anim);

	AutoResource<FrameAnimation> pusherleftanim = getAnimation(ResIds::build::sipka_rh_texture_convert::pushlft);
	AutoResource<FrameAnimation> pusherrightanim = getAnimation(ResIds::build::sipka_rh_texture_convert::pushrgt);

	AutoResource<FrameAnimation> redkeyanim = getAnimation(ResIds::build::sipka_rh_texture_convert::keyred_anim);
	AutoResource<FrameAnimation> greenkeyanim = getAnimation(ResIds::build::sipka_rh_texture_convert::keygreen_anim);
	AutoResource<FrameAnimation> bluekeyanim = getAnimation(ResIds::build::sipka_rh_texture_convert::keyblue_anim);
	AutoResource<FrameAnimation> yellowkeyanim = getAnimation(ResIds::build::sipka_rh_texture_convert::keyellow_anim);

	AutoResource<FrameAnimation> reddooranim = getAnimation(ResIds::build::sipka_rh_texture_convert::doorred_anim);
	AutoResource<FrameAnimation> greendooranim = getAnimation(ResIds::build::sipka_rh_texture_convert::doorgree_anim);
	AutoResource<FrameAnimation> bluedooranim = getAnimation(ResIds::build::sipka_rh_texture_convert::doorblue_anim);
	AutoResource<FrameAnimation> yellowdooranim = getAnimation(ResIds::build::sipka_rh_texture_convert::doorylow_anim);
	AutoResource<FrameAnimation> onetimedooranim = getAnimation(ResIds::build::sipka_rh_texture_convert::dooronetime_anim);

	AutoResource<FrameAnimation> tntanim = getAnimation(ResIds::build::sipka_rh_texture_convert::bomb10);

	AutoResource<FrameAnimation> rubyanim = getAnimation(ResIds::build::sipka_rh_texture_convert::ruby_anim);
	AutoResource<FrameAnimation> sapphireanim = getAnimation(ResIds::build::sipka_rh_texture_convert::sapphire_anim);
	AutoResource<FrameAnimation> sapphirebreakanim = getAnimation(ResIds::build::sipka_rh_texture_convert::saphbrk_anim);
	AutoResource<FrameAnimation> emeraldanim = getAnimation(ResIds::build::sipka_rh_texture_convert::emerald_anim);
	AutoResource<FrameAnimation> citrineanim = getAnimation(ResIds::build::sipka_rh_texture_convert::citrine_anim);
	AutoResource<FrameAnimation> citrinebreakanim = getAnimation(ResIds::build::sipka_rh_texture_convert::citrine_break_anim);
	AutoResource<FrameAnimation> citrineshatteranim = getAnimation(ResIds::build::sipka_rh_texture_convert::citrine_shatter_anim);

	AutoResource<FrameAnimation> converteranim = getAnimation(ResIds::build::sipka_rh_texture_convert::convert_anim);
	AutoResource<FrameAnimation> dispenseranim = getAnimation(ResIds::build::sipka_rh_texture_convert::dispenser);
	AutoResource<FrameAnimation> rockanim = getAnimation(ResIds::build::sipka_rh_texture_convert::stone_anim);
	AutoResource<FrameAnimation> bombanim = getAnimation(ResIds::build::sipka_rh_texture_convert::bomb_anim);
	AutoResource<FrameAnimation> dropanim = getAnimation(ResIds::build::sipka_rh_texture_convert::drop_anim);
	AutoResource<FrameAnimation> droptorightanim = getAnimation(ResIds::build::sipka_rh_texture_convert::droprgt_anim);
	AutoResource<FrameAnimation> droptoleftanim = getAnimation(ResIds::build::sipka_rh_texture_convert::droplft_anim);
	AutoResource<FrameAnimation> droptodownanim = getAnimation(ResIds::build::sipka_rh_texture_convert::dropdwn_anim);
	AutoResource<FrameAnimation> droptoupanim = getAnimation(ResIds::build::sipka_rh_texture_convert::dropup_anim);
	AutoResource<FrameAnimation> drophitanim = getAnimation(ResIds::build::sipka_rh_texture_convert::drophit_anim);
	AutoResource<FrameAnimation> swampanim = getAnimation(ResIds::build::sipka_rh_texture_convert::swamp_anim);

	AutoResource<FrameAnimation> exitanim = getAnimation(ResIds::build::sipka_rh_texture_convert::dooropen_anim);
	AutoResource<FrameAnimation> baganim = getAnimation(ResIds::build::sipka_rh_texture_convert::bag_anim);
	AutoResource<FrameAnimation> bagopenanim = getAnimation(ResIds::build::sipka_rh_texture_convert::bagopen_anim);
	AutoResource<FrameAnimation> glassanim = getAnimation(ResIds::build::sipka_rh_texture_convert::glaswall);

	AutoResource<FrameAnimation> roundstonewallanim = getAnimation(ResIds::build::sipka_rh_texture_convert::rndwall);
	AutoResource<FrameAnimation> wallanim = getAnimation(ResIds::build::sipka_rh_texture_convert::wall);
	AutoResource<FrameAnimation> earthanim = getAnimation(ResIds::build::sipka_rh_texture_convert::earth_anim);

	AutoResource<FrameAnimation> earthdigupanim = getAnimation(ResIds::build::sipka_rh_texture_convert::earthup_anim);
	AutoResource<FrameAnimation> earthdigleftanim = getAnimation(ResIds::build::sipka_rh_texture_convert::earthlft_anim);

	AutoResource<FrameAnimation> safeanim = getAnimation(ResIds::build::sipka_rh_texture_convert::box);
	AutoResource<FrameAnimation> darkwallanim = getAnimation(ResIds::build::sipka_rh_texture_convert::invisi);
	AutoResource<FrameAnimation> stonewallanim = getAnimation(ResIds::build::sipka_rh_texture_convert::stonewll);
	AutoResource<FrameAnimation> sandanim = getAnimation(ResIds::build::sipka_rh_texture_convert::sand);

	AutoResource<FrameAnimation> timebombanim = getAnimation(ResIds::build::sipka_rh_texture_convert::timebomb);
	AutoResource<FrameAnimation> tickingbombanim = getAnimation(ResIds::build::sipka_rh_texture_convert::tickbomb_anim);
	AutoResource<FrameAnimation> cushionanim = getAnimation(ResIds::build::sipka_rh_texture_convert::cushion_anim);

	AutoResource<FrameAnimation> yamyamanim = getAnimation(ResIds::build::sipka_rh_texture_convert::yamyam_anim);
	AutoResource<FrameAnimation> yamyamdirsanim = getAnimation(ResIds::build::sipka_rh_texture_convert::yamyamdirs_anim);
	AutoResource<FrameAnimation> wheelanim = getAnimation(ResIds::build::sipka_rh_texture_convert::wheel_anim);
	AutoResource<FrameAnimation> robotanim = getAnimation(ResIds::build::sipka_rh_texture_convert::robot_anim);

	AutoResource<FrameAnimation> lorryanim = getAnimation(ResIds::build::sipka_rh_texture_convert::lorrylft_anim);
	AutoResource<FrameAnimation> buganim = getAnimation(ResIds::build::sipka_rh_texture_convert::buglft_anim);

	AutoResource<FrameAnimation> elevatoranim = getAnimation(ResIds::build::sipka_rh_texture_convert::elevator_anim);

	AutoResource<FrameAnimation> stonewallgemmask = getAnimation(ResIds::build::sipka_rh_texture_convert::stonegem_mask);

	FrameAnimation::Element getPlayerElement(const Level::GameObject& o, float turnpercent);
	FrameAnimation::Element getPlayerElement(MinerAnimations& mineranims, bool pushing, bool digging, bool moving, SapphireDirection dir,
			float turnpercent);

	const FrameAnimation::Element& getPastObjectElement(const Level::GameObject& o, float turnpercent);

	template<SapphireObject obj>
	void drawObject(const Level::GameObject& o, const Matrix2D& mvp, const Matrix2D& expmvp, float alpha, float turnpercent);

	void drawDoor(const FrameAnimation& anim, const Level::GameObject& o, const Matrix2D& mvp, const Matrix2D& omvp, float alpha,
			float turnpercent);

	void drawEarth(const Level::GameObject& o, const Matrix2D& mvp, float alpha);

	const Level* level;
public:
	LevelDrawer2D(LevelDrawer& parent, const Level* level);
	LevelDrawer2D(const LevelDrawer2D&) = default;
	LevelDrawer2D(LevelDrawer2D&&) = default;

	virtual void draw(LevelDrawer& parent, float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end, const Vector2F& mid,
			const Size2F& objectSize) override;

};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_LEVELDRAWER2D_H_ */
