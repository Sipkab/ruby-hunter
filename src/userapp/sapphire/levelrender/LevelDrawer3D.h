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
 * LevelDrawer3D.h
 *
 *  Created on: 2016. jul. 21.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_LEVELDRAWER3D_H_
#define TEST_SAPPHIRE_LEVEL_LEVELDRAWER3D_H_

#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <gen/assets.h>
#include <gen/shader/SapphirePhongShader.h>
#include <appmain.h>
#include <sapphire/level/Level.h>
#include <sapphire/level/SapphireRandom.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <sapphire/levelrender/Level3DBackground.h>

namespace userapp {
using namespace rhfw;

class SapphireObjectBuffer: public ShareableResource {
public:
	struct Material {
		Color ambient;
		Color specular;
		float specularExponent;
	};
	struct TexturedMaterial: public Material {
		AutoResource<render::Texture> diffuseColorMap;
	};
	struct ColoredMaterial: public Material {
		Color diffuseColor;
	};

	AutoResource<render::VertexBuffer> coloredBuffer;
	AutoResource<SapphirePhongShader::InputLayout> coloredInputLayout;
	AutoResource<render::VertexBuffer> texturedBuffer;
	AutoResource<SapphireTexturedPhongShader::InputLayout> texturedInputLayout;

	ColoredMaterial* colorMaterials = nullptr;
	TexturedMaterial* textureMaterials = nullptr;
	uint32 colorMaterialCount = 0;
	uint32 textureMaterialCount = 0;
private:
	ResId resourceId;
protected:
	virtual bool load() override;
	virtual void free() override;
public:
	explicit SapphireObjectBuffer(ResId resid)
			: resourceId(resid) {
	}

	Resource<SapphirePhongShader::InputLayout> getColoredInputLayout() {
		return coloredInputLayout;
	}
	Resource<SapphireTexturedPhongShader::InputLayout> getTexturedInputLayout() {
		return texturedInputLayout;
	}

	ColoredMaterial& getColoredMaterial(unsigned int index) {
		ASSERT(index < colorMaterialCount);
		return colorMaterials[index];
	}
	TexturedMaterial& getTexturedMaterial(unsigned int index) {
		ASSERT(index < textureMaterialCount);
		return textureMaterials[index];
	}

	uint32 getColoredMaterialCount() const {
		return colorMaterialCount;
	}
	uint32 getTexturedMaterialCount() const {
		return textureMaterialCount;
	}
};

class Sapphire3DObject: public ShareableResource {
public:
	struct DrawingRange {
		uint32 materialIndex;
		uint32 vertexIndex;
		uint32 vertexCount;
	};
private:
	DrawingRange* coloredRanges = nullptr;
	DrawingRange* texturedRanges = nullptr;
	uint32 coloredCount = 0;
	uint32 texturedCount = 0;

	Vector3F origin { 0, 0, 0 };

	ResId resourceId;
protected:
	virtual bool load() override;
	virtual void free() override;
public:

	explicit Sapphire3DObject(ResId resid)
			: resourceId(resid) {
	}

	const Vector3F& getOrigin() const {
		return origin;
	}

	unsigned int getColoredCount() const {
		return coloredCount;
	}
	unsigned int getTexturedCount() const {
		return texturedCount;
	}

	DrawingRange& getColored(unsigned int index) {
		ASSERT(index < coloredCount);
		return coloredRanges[index];
	}
	DrawingRange& getTextured(unsigned int index) {
		ASSERT(index < texturedCount);
		return texturedRanges[index];
	}
};

class Level;
class LevelDrawer3D: public LevelDrawer::DrawerImpl {

	AutoResource<SapphirePhongShader::UMaterial> colorMaterialUniform = sapphirePhongShader->createUniform_UMaterial();
	AutoResource<SapphirePhongShader::ShaderUniform> colorShaderUniform = sapphirePhongShader->createUniform_ShaderUniform();
	AutoResource<SapphirePhongShader::UState> colorStateUniform = sapphirePhongShader->createUniform_UState();
	AutoResource<SapphirePhongShader::UFragmentLighting> colorAmbientLighting = sapphirePhongShader->createUniform_UFragmentLighting();

	AutoResource<SapphireTexturedPhongShader::UMaterial> textureMaterialUniform = sapphireTexturedPhongShader->createUniform_UMaterial();
	AutoResource<SapphireTexturedPhongShader::UState> textureStateUniform = sapphireTexturedPhongShader->createUniform_UState();
	AutoResource<SapphireTexturedPhongShader::ShaderUniform> textureShaderUniform =
			sapphireTexturedPhongShader->createUniform_ShaderUniform();
	AutoResource<SapphireTexturedPhongShader::UFragmentLighting> textureAmbientLighting =
			sapphireTexturedPhongShader->createUniform_UFragmentLighting();

	class DrawCommand: public LinkedNode<DrawCommand> {
	public:
		Matrix3D mvp;
		Matrix3D mvpInverseTranspose;
		unsigned int vertexIndex;
		unsigned int vertexCount;

		DrawCommand* get() override {
			return this;
		}
	};
	class CommandsHolder {
	public:
		MoveablePointer<LinkedList<DrawCommand, false>> coloreds;
		MoveablePointer<LinkedList<DrawCommand, false>> textureds;

		CommandsHolder() {
		}
		CommandsHolder(SapphireObjectBuffer* buf) {
			init(buf);
		}
		CommandsHolder(CommandsHolder&&) = default;
		~CommandsHolder() {
			delete[] coloreds;
			delete[] textureds;
		}

		void init(SapphireObjectBuffer* buf) {
			coloreds = new LinkedList<DrawCommand, false> [buf->getColoredMaterialCount()];
			textureds = new LinkedList<DrawCommand, false> [buf->getTexturedMaterialCount()];
		}

	};
	class ColoredCommands: public LinkedNode<ColoredCommands> {
	public:
		Color color;
		CommandsHolder commands;

		ColoredCommands() {
		}
		ColoredCommands(SapphireObjectBuffer* buf)
				: commands(buf) {
		}

		ColoredCommands* get() override {
			return this;
		}
		void init(SapphireObjectBuffer* buf) {
			commands.init(buf);
		}

		void add(Sapphire3DObject* mesh, const Matrix<4>& u_m, Matrix<4> u_minv, LinkedList<DrawCommand>& cache);
		void addBuilding(Sapphire3DObject* mesh, const Matrix<4>& u_m, Matrix<4> u_minv, float percent, LinkedList<DrawCommand>& cache);
	};

	AutoResource<SapphireObjectBuffer> objectsBuffer = getApplicationResource<SapphireObjectBuffer>(
			ResIds::sapphire_yours::objects_3d_collection);

	ColoredCommands defaultCommands { objectsBuffer };
	ColoredCommands dirtCommands { objectsBuffer };
	ColoredCommands laserCommands { objectsBuffer };
	ColoredCommands minerSinkCommands { objectsBuffer };
	ColoredCommands dispenserCommands { objectsBuffer };
	ColoredCommands turnAppearingCommands { objectsBuffer };
	ColoredCommands explosioncommands[4];
	LinkedList<ColoredCommands, false> drawCommands;

	LinkedList<DrawCommand> drawCommandCache;

	bool lightingFixed = false;
	Vector2F lightingPosition { 0, 0 };

#define DECLARE_MESH(name) AutoResource<Sapphire3DObject> name##mesh = getApplicationResource<Sapphire3DObject>(\
		ResIds::gameres::game_sapphire::objects::name##_sapphire_obj\
		)

	DECLARE_MESH(bug_body);
	DECLARE_MESH(bug_leg_general_left);
	DECLARE_MESH(bug_leg_general_right);

	DECLARE_MESH(robot);
	DECLARE_MESH(lorry);
	DECLARE_MESH(swamp);
	DECLARE_MESH(drop);

	DECLARE_MESH(wheel);

	DECLARE_MESH(emerald);
	DECLARE_MESH(ruby);
	DECLARE_MESH(bag);
	DECLARE_MESH(safe);

	DECLARE_MESH(sapphire);
	DECLARE_MESH(sapphire_break);

	DECLARE_MESH(citrine);
	DECLARE_MESH(citrine_breaking);
	DECLARE_MESH(citrine_shattering_1);
	DECLARE_MESH(citrine_shattering_2);
	DECLARE_MESH(citrine_shattering_3);
	DECLARE_MESH(citrine_shattering_4);

	DECLARE_MESH(wall);
	DECLARE_MESH(stonewall2_emerald_part);
	DECLARE_MESH(stonewall2_sapphire_part);
	DECLARE_MESH(stonewall2_ruby_part);
	DECLARE_MESH(stonewall2_citrine_part);
	DECLARE_MESH(roundstonewall);
	DECLARE_MESH(roundstonewall_downopen);
	DECLARE_MESH(glass);
	DECLARE_MESH(sand2);
	DECLARE_MESH(sand2_top);
	DECLARE_MESH(sand2_bot);
	DECLARE_MESH(cushion);
	DECLARE_MESH(rock);

	DECLARE_MESH(tnt);
	DECLARE_MESH(bomb);

	DECLARE_MESH(explosion);

	DECLARE_MESH(dispenser);
	DECLARE_MESH(pusher);
	DECLARE_MESH(pusher_right);

	DECLARE_MESH(exit_doorway);
	DECLARE_MESH(exit_door_left);
	DECLARE_MESH(exit_door_right);

	DECLARE_MESH(yamyam_head);
	DECLARE_MESH(yamyam_mouthtop);
	DECLARE_MESH(yamyam_mouthbottom);
	DECLARE_MESH(yamyam_teethtop);
	DECLARE_MESH(yamyam_teethbottom);
	DECLARE_MESH(yamyam_mouthinside);

	DECLARE_MESH(tickbomb);
	DECLARE_MESH(tickbomb_led_bombon);
	DECLARE_MESH(tickbomb_led_bomboff);
	DECLARE_MESH(tickbomb_led_tickon);
	DECLARE_MESH(tickbomb_led_tickoff);

	DECLARE_MESH(elevator);
	DECLARE_MESH(elevator_ledoff);
	DECLARE_MESH(elevator_ledon);

	DECLARE_MESH(key_blue);
	DECLARE_MESH(key_red);
	DECLARE_MESH(key_green);
	DECLARE_MESH(key_yellow);

	class MinerMesh {
	public:
		AutoResource<Sapphire3DObject> headmesh;
		AutoResource<Sapphire3DObject> bodymesh;
		AutoResource<Sapphire3DObject> armleftmesh;
		AutoResource<Sapphire3DObject> armrightmesh;
		AutoResource<Sapphire3DObject> legleftmesh;
		AutoResource<Sapphire3DObject> legrightmesh;
	};

	MinerMesh miner1mesh {
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_head_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_body_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_armleft_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_armright_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_legleft_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_legright_sapphire_obj),
	};
	//TODO
	MinerMesh miner2mesh {
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner2_head_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner2_body_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner2_armleft_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner2_armright_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_legleft_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::miner_legright_sapphire_obj),
	};

	DECLARE_MESH(miner_pickaxe);
	DECLARE_MESH(miner_drill);

#define DECLARE_DOOR(color) \
	DECLARE_MESH(door_##color##_left); \
	DECLARE_MESH(door_##color##_right); \
	DECLARE_MESH(door_cover_##color)

	DECLARE_DOOR(red);
	DECLARE_DOOR(green);
	DECLARE_DOOR(blue);
	DECLARE_DOOR(yellow);
	DECLARE_DOOR(onetime);
#undef DECLARE_DOOR

	DECLARE_MESH(door_cover_onetime_closed);
	DECLARE_MESH(door_onetime_lock);

	DECLARE_MESH(earth_dirt);

	class Sided3DObject {
	public:
		AutoResource<Sapphire3DObject> covermesh;
		AutoResource<Sapphire3DObject> bottommesh;
		AutoResource<Sapphire3DObject> leftbottommesh;
		AutoResource<Sapphire3DObject> leftbottomrightmesh;
		AutoResource<Sapphire3DObject> allmesh;
		AutoResource<Sapphire3DObject> leftrightmesh;
	};
	Sided3DObject stonewallSidedMesh {
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::stonewall2_cover_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::stonewall2_bottom_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::stonewall2_leftbottom_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::stonewall2_leftbottomright_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::stonewall2_all_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::stonewall2_leftright_sapphire_obj)
	};
	Sided3DObject invisiblestonewallSidedMesh {
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::invisiblestonewall2_cover_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::invisiblestonewall2_bottom_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::invisiblestonewall2_leftbottom_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::invisiblestonewall2_leftbottomright_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::invisiblestonewall2_all_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::invisiblestonewall2_leftright_sapphire_obj)
	};

	Sided3DObject earth2SidedMesh {
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::earth2_cover_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::earth2_bottom_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::earth2_leftbottom_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::earth2_leftbottomright_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::earth2_all_sapphire_obj),
		getApplicationResource<Sapphire3DObject>(ResIds::gameres::game_sapphire::objects::earth2_leftright_sapphire_obj)
	};

	DECLARE_MESH(acid_body);
	DECLARE_MESH(acid_waves_left);
	DECLARE_MESH(acid_waves_right);
	DECLARE_MESH(acid_side);

	DECLARE_MESH(converter_base);
	DECLARE_MESH(converter_ring);
	DECLARE_MESH(converter_lightning);

	DECLARE_MESH(laser_vertical);
	DECLARE_MESH(laser_left);
#undef DECLARE_MESH

	void keyObjectTransform(Matrix3D& u_m, Matrix3D& u_minv, float turnpercent, unsigned int turn);

	Sapphire3DObject* getPast3DObject(SapphireObject object);
	Sapphire3DObject* getPast3DObject(SapphireObject object, Matrix3D& u_m, Matrix3D& u_minv, float turnpercent);

	template<typename Tester>
	void drawSidedObject(const Level::GameObject& o, Sided3DObject& obj, const Matrix<4>& u_m, const Matrix<4>& u_minv, Tester&& test);

	void drawObject(Sapphire3DObject* mesh, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& commands) {
		commands.add(mesh, u_m, u_minv, drawCommandCache);
	}
	void drawObject(Sapphire3DObject* mesh, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
		drawObject(mesh, u_m, u_minv, defaultCommands);
	}
	void drawObjectAtOrigin(Sapphire3DObject* mesh, const Matrix<4>& u_m, const Matrix<4>& u_minv,
			ColoredCommands& commands) {
		drawObject(mesh, Matrix3D().setTranslate(Vector3F {mesh->getOrigin()}) *= u_m,
				Matrix3D(u_minv).multTranslate(Vector3F {-mesh->getOrigin()}), commands);
	}
	void drawObjectAtOrigin(Sapphire3DObject* mesh, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
		drawObjectAtOrigin(mesh, u_m, u_minv, defaultCommands);
	}

	void drawBuildingObject(Sapphire3DObject* mesh, const Matrix<4>& u_m, const Matrix<4>& u_minv,
			float percent, ColoredCommands& commands) {
		commands.addBuilding(mesh, u_m, u_minv, percent, drawCommandCache);
	}
	void drawBuildingObject(Sapphire3DObject* mesh, const Matrix<4>& u_m, const Matrix<4>& u_minv,
			float percent) {
		drawBuildingObject(mesh, u_m, u_minv, percent, defaultCommands);
	}

	void drawSand(const Level::GameObject& o, const Matrix<4>& u_m, const Matrix<4>& u_minv);

	void drawKey(float turnpercent, Sapphire3DObject* mesh, Matrix<4> u_m, Matrix<4> u_minv, unsigned int levelturn);
	void drawExit(float openpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv);

	void drawElevator(bool on, const Matrix<4>& u_m, const Matrix<4>& u_minv);

	template<typename... Args>
	void drawMiner(MinerMesh& miner, float turnpercent, unsigned int levelturn, Matrix<4> u_m, Matrix<4> u_minv, SapphireDirection direction, SapphireDirection facing,
			bool dig, bool move, bool push, Args&&... args);
	template<typename... Args>
	void drawMinerVerticalWalkingLegs(float turnpercent, Matrix<4> u_m, Matrix<4> u_minv, SapphireDirection direction,
			Sapphire3DObject* frontleg, Sapphire3DObject* backleg, Args&&... args);

	void drawAcid(float turnpercent, const Level::GameObject& o, const Matrix<4>& u_m, const Matrix<4>& u_minv);
	void drawConverter(float turnpercent, const Level::GameObject& o,const Matrix<4>& u_m, const Matrix<4>& u_minv);

	void drawBug(float turnpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv);
	void drawBugLegs(float turnpercent, Matrix<4> u_m, Matrix<4> u_minv, Sapphire3DObject* leg, float anglemult);

	void drawDoor(float turnpercent, Sapphire3DObject* cover, Sapphire3DObject* left, Sapphire3DObject* right, Matrix<4> u_m, Matrix<4> u_minv,
			const Level::GameObject& o);

	void drawSingle(const Level::GameObject& o, float turnpercent);

	const Level* level;
	bool needBackground = true;
	bool orthographic = false;

	Level3DBackground background;

	void generateBackground();

	void drawGem(Sapphire3DObject* mesh, float rotation, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawObject(mesh, Matrix3D().setRotate(rotation * 3.1415f, 0, 1, 0) *= u_m, Matrix3D(u_minv).multRotate(rotation * 3.1415f, 0, -1, 0), cmd);
	}

public:
	LevelDrawer3D();
	LevelDrawer3D(LevelDrawer& parent, const Level* level);
	LevelDrawer3D(const LevelDrawer3D&) = delete;
	LevelDrawer3D(LevelDrawer3D&&) = default;
	~LevelDrawer3D();

	virtual void draw(LevelDrawer& parent, float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end,
			const Vector2F& mid, const Size2F& objectSize) override;
	void draw(float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end,
			const Vector2F& mid, const Size2F& objectSize, bool fulldispenser, const Size2UI& pixelsize, const Rectangle& paddings);
	void finishDrawing(float alpha, float turnpercent, const Size2F& objectSize,const Size2UI& pixelsize, const Rectangle& paddings,
			float depth, const Vector2F& mid);

	void drawRobot(float turnpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv, unsigned int levelturn, ColoredCommands& cmd);

	void drawOpeningBag(float openpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawBuildingObject(bagmesh, u_m, u_minv, 1.0f - openpercent, cmd);
		drawObject(emeraldmesh, Matrix3D().setScale(openpercent, openpercent, openpercent) *= u_m,
				Matrix3D(u_minv).multScale(1 / openpercent, 1 / openpercent, 1 / openpercent), cmd);
	}
	void drawBag(const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawObject(bagmesh, u_m, u_minv, cmd);
	}
	void drawSafe(const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawObject(safemesh, u_m, u_minv, cmd);
	}

	void drawSapphire(float rotation, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawGem(sapphiremesh, rotation, u_m, u_minv, cmd);
	}
	void drawEmerald(float rotation, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawGem(emeraldmesh, rotation, u_m, u_minv, cmd);
	}
	void drawRuby(float rotation, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawGem(rubymesh, rotation, u_m, u_minv, cmd);
	}
	void drawCitrine(float rotation, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawGem(citrinemesh, rotation, u_m, u_minv, cmd);
	}
	void drawBomb(float rotation, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawGem(bombmesh, rotation, u_m, u_minv, cmd);
	}
	void drawYamYam(float openpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd);
	void drawYamYam(float openpercent, float turnpercent, Matrix<4> modeltrans, Matrix<4> modelinverse, SapphireDirection olddir, SapphireDirection newdir, ColoredCommands& cmd);
	void drawBuildingSwamp(float buildpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
		drawBuildingObject(swampmesh, u_m, u_minv, buildpercent, cmd);
	}
	void drawDropHit(float turnpercent, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse, ColoredCommands& cmd) {
		drawObject(dropmesh, Matrix3D().setScale(1, 1 - turnpercent, 1).multTranslate(0, -turnpercent / 2.0f * 0.75f, 0) *=
				modeltrans,
				Matrix3D(modelinverse).multTranslate(0, turnpercent / 2.0f * 0.75f, 0).multScale(1, 1 / (1 - turnpercent), 1), cmd);
	}
	void drawFallingDrop(float turnpercent, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse, unsigned int mod, ColoredCommands& cmd);

	void drawStandingMiner(MinerMesh& minermesh, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse, ColoredCommands& cmd);
	void drawWalkingMiner(MinerMesh& minermesh, float turnpercent, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse,
			SapphireDirection dir, SapphireDirection facing, unsigned int mod, ColoredCommands& cmd);

	void drawTickBomb(bool bombon, bool tickon, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd);

	virtual void levelReloaded() override;

	MinerMesh& getMiner1Mesh() {
		return miner1mesh;
	}
	MinerMesh& getMiner2Mesh() {
		return miner2mesh;
	}

	void setFixedLighting(const Vector2F& pos) {
		lightingFixed = true;
		lightingPosition = pos;
	}

	void setOrthographic(bool ortho) {
		this->orthographic = ortho;
	}

	bool isOrthographic() const {
		return orthographic;
	}

	void disableBackground();

	ColoredCommands& getManualColoredCommand() {
		return dispenserCommands;
	}
	ColoredCommands& getDefaultColoredCommand() {
		return defaultCommands;
	}
};

}
 // namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_LEVELDRAWER3D_H_ */
