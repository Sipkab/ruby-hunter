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
 * Level3DBackground.h
 *
 *  Created on: 2017. febr. 15.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVELRENDER_LEVEL3DBACKGROUND_H_
#define TEST_SAPPHIRE_LEVELRENDER_LEVEL3DBACKGROUND_H_

#include <gen/shader/SapphirePhongShader.h>
#include <appmain.h>

namespace userapp {
using namespace rhfw;

class Level3DBackground {
	AutoResource<render::VertexBuffer> backgroundVertexBuffer;
	AutoResource<SapphirePhongShader::InputLayout> backgroundInputLayout;

	unsigned int randomSeed = 0;
	unsigned int backgroundTriangleCount = 0;
public:
	Level3DBackground(bool enabled, unsigned int randomseed);
	~Level3DBackground();

	void regenerateBackground();
	void regenerateBackground(unsigned int randomseed);

	void enable();
	void disable();

	bool isEnabled() const {
		return backgroundTriangleCount != 0;
	}

	void activateInputLayout() {
		backgroundInputLayout->activate();
	}

	unsigned int getTriangleCount() const {
		return backgroundTriangleCount;
	}
};

class StandaloneLevel3DBackground: public Level3DBackground {
	AutoResource<SapphirePhongShader::UMaterial> colorMaterialUniform = sapphirePhongShader->createUniform_UMaterial();
	AutoResource<SapphirePhongShader::ShaderUniform> colorShaderUniform = sapphirePhongShader->createUniform_ShaderUniform();
	AutoResource<SapphirePhongShader::UState> colorStateUniform = sapphirePhongShader->createUniform_UState();
	AutoResource<SapphirePhongShader::UFragmentLighting> colorAmbientLighting = sapphirePhongShader->createUniform_UFragmentLighting();
public:
	StandaloneLevel3DBackground(bool enabled, unsigned int randomseed);

	void drawSimpleBackground(float alpha, float aspectratio, const Vector2F& lightpos);
	void drawSimpleBackground(float alpha, float aspectratio);
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_LEVELRENDER_LEVEL3DBACKGROUND_H_ */
