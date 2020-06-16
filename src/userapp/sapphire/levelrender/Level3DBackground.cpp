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
 * Level3DBackground.cpp
 *
 *  Created on: 2017. febr. 15.
 *      Author: sipka
 */

#include <sapphire/levelrender/Level3DBackground.h>
#include <appmain.h>
#include <sapphire/level/SapphireRandom.h>
#include <sapphire/sapphireconstants.h>

#include <framework/geometry/Vector.h>

#include <gen/log.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace userapp {
using namespace rhfw;

class Edge;
class Point;
class Triangle;
class Point {
public:
	Vector3F position;
	Edge* edge = nullptr;
};
class Edge {
public:
	Point* startPoint = nullptr;
	Edge* nextEdge = nullptr;
	Edge* pair = nullptr;
	Triangle* triangle = nullptr;

	float length() const {
		return (startPoint->position - pair->startPoint->position).length();
	}
	float lengthXY() const {
		return (startPoint->position - pair->startPoint->position).xy().length();
	}

	Vector3F getVector() const {
		return pair->startPoint->position - startPoint->position;
	}
	Vector2F getVectorXY() const {
		return (pair->startPoint->position - startPoint->position).xy();
	}
};
class Triangle {
public:
	Edge* edges[3] { nullptr, nullptr, nullptr };
	Triangle() {

	}
	Triangle(Edge * a, Edge * b, Edge * c)
			: edges { a, b, c } {
	}

	int getEdgeIndex(Edge* e) const {
		for (int i = 0; i < 3; ++i) {
			if (edges[i] == e) {
				return i;
			}
		}
		THROW()<< "Edge not found in triangle: " << e->startPoint->position << " -> " << e->pair->startPoint->position;
		return -1;
	}

	float area() const {
		float a = edges[0]->length();
		float b = edges[1]->length();
		float c = edges[2]->length();
		float s = (a + b + c) / 2.0f;
		return sqrtf(s * (s - a) * (s - b) * (s - c));
	}
	float areaXY() const {
		float a = edges[0]->lengthXY();
		float b = edges[1]->lengthXY();
		float c = edges[2]->lengthXY();
		float s = (a + b + c) / 2.0f;
		return sqrtf(s * (s - a) * (s - b) * (s - c));
	}

	unsigned int getLongestEdgeIndex() {
		unsigned int res = 0;
		float len = edges[0]->length();
		for (unsigned int i = 1; i < 3; ++i) {
			float l = edges[i]->length();
			if (l > len) {
				len = l;
				res = i;
			}
		}
		return res;
	}
	unsigned int getLongestXYEdgeIndex() {
		unsigned int res = 0;
		float len = edges[0]->lengthXY();
		for (unsigned int i = 1; i < 3; ++i) {
			float l = edges[i]->lengthXY();
			if (l > len) {
				len = l;
				res = i;
			}
		}
		return res;
	}
};

Level3DBackground::Level3DBackground(bool enabled, unsigned int randomseed)
		: randomSeed(randomseed) {
	if (enabled) {
		enable();
	}
}

Level3DBackground::~Level3DBackground() {
}

void Level3DBackground::regenerateBackground(unsigned int randomseed) {
	if (this->randomSeed == randomseed) {
		//already generated with this random seed
		return;
	}
	this->randomSeed = randomseed;
	regenerateBackground();
}
void Level3DBackground::regenerateBackground() {
	backgroundVertexBuffer->reinitialize();
}

void Level3DBackground::enable() {
	static const unsigned int MAX_TRIANGLE_COUNT = 4096;

	backgroundVertexBuffer = renderer->createVertexBuffer();
	backgroundVertexBuffer->setBufferInitializer<SapphirePhongShader::VertexInput>([=](SapphirePhongShader::VertexInput* initer) {
		static const unsigned int EDGE_COUNT = MAX_TRIANGLE_COUNT * 8; /*6 + 2 just in case*/

		unsigned int totalpointcount = 256;
		Edge* edges = new Edge[EDGE_COUNT];
		Point* points = new Point[totalpointcount];
		Triangle* triangles = new Triangle[MAX_TRIANGLE_COUNT];

		unsigned int edgecount = 16;
		unsigned int pointcount = 5;
		unsigned int tricount = 4;

		SapphireRandom random;
		random.setSeed(this->randomSeed);
		//LOGI() << "Generating background with random seed: " << random.getSeed();

			triangles[0] = Triangle {&edges[0], &edges[9], &edges[14]};
			triangles[1] = Triangle {&edges[1], &edges[11], &edges[8]};
			triangles[2] = Triangle {&edges[2], &edges[13], &edges[10]};
			triangles[3] = Triangle {&edges[3], &edges[15], &edges[12]};

			edges[0].triangle = &triangles[0];
			edges[1].triangle = &triangles[1];
			edges[2].triangle = &triangles[2];
			edges[3].triangle = &triangles[3];
			edges[4].triangle = nullptr;
			edges[5].triangle = nullptr;
			edges[6].triangle = nullptr;
			edges[7].triangle = nullptr;
			edges[8].triangle = &triangles[1];
			edges[9].triangle = &triangles[0];
			edges[10].triangle = &triangles[2];
			edges[11].triangle = &triangles[1];
			edges[12].triangle = &triangles[3];
			edges[13].triangle = &triangles[2];
			edges[14].triangle = &triangles[0];
			edges[15].triangle = &triangles[3];

			points[0].position = Vector3F {0, 0, 0};
			points[1].position = Vector3F {0, 1, 0};
			points[2].position = Vector3F {1, 1,0};
			points[3].position = Vector3F {1, 0, 0};
			points[4].position = Vector3F {(0.5f + (random.nextFloat() - 0.5f) * 0.5f),
				(0.5f + (random.nextFloat() - 0.5f) * 0.5f), 0};

			points[0].edge = &edges[0];
			points[1].edge = &edges[1];
			points[2].edge = &edges[2];
			points[3].edge = &edges[3];
			points[4].edge = &edges[9];

			edges[0].startPoint = &points[0];
			edges[1].startPoint = &points[1];
			edges[2].startPoint = &points[2];
			edges[3].startPoint = &points[3];

			edges[4].startPoint = &points[1];
			edges[5].startPoint = &points[2];
			edges[6].startPoint = &points[3];
			edges[7].startPoint = &points[0];

			edges[8].startPoint = &points[4];
			edges[9].startPoint = &points[1];
			edges[10].startPoint = &points[4];
			edges[11].startPoint = &points[2];
			edges[12].startPoint = &points[4];
			edges[13].startPoint = &points[3];
			edges[14].startPoint = &points[4];
			edges[15].startPoint = &points[0];

			edges[0].nextEdge = &edges[15];
			edges[1].nextEdge = &edges[9];
			edges[2].nextEdge = &edges[11];
			edges[3].nextEdge = &edges[13];
			edges[4].nextEdge = nullptr;
			edges[5].nextEdge = nullptr;
			edges[6].nextEdge = nullptr;
			edges[7].nextEdge = nullptr;
			edges[8].nextEdge = &edges[10];
			edges[9].nextEdge = &edges[4];
			edges[10].nextEdge = &edges[12];
			edges[11].nextEdge = &edges[5];
			edges[12].nextEdge = &edges[14];
			edges[13].nextEdge = &edges[6];
			edges[14].nextEdge = &edges[8];
			edges[15].nextEdge = &edges[7];

			edges[0].pair = &edges[4];
			edges[1].pair = &edges[5];
			edges[2].pair = &edges[6];
			edges[3].pair = &edges[7];
			edges[4].pair = &edges[0];
			edges[5].pair = &edges[1];
			edges[6].pair = &edges[2];
			edges[7].pair = &edges[3];
			edges[8].pair = &edges[9];
			edges[9].pair = &edges[8];
			edges[10].pair = &edges[11];
			edges[11].pair = &edges[10];
			edges[12].pair = &edges[13];
			edges[13].pair = &edges[12];
			edges[14].pair = &edges[15];
			edges[15].pair = &edges[14];

			while (edgecount + 8 <= EDGE_COUNT && pointcount + 2 <= totalpointcount && tricount + 2 <= MAX_TRIANGLE_COUNT) {
				Triangle* tri = edges[0].triangle;
				ASSERT(tri != nullptr);
				Edge* longestedge = &edges[0];
				float longestlength = longestedge->lengthXY();
				for (unsigned int i = 1; i < edgecount; ++i) {
					Edge* e = &edges[i];
					if(e->triangle == nullptr) {
						continue;
					}
					float l = e->lengthXY();
					if (l > longestlength) {
						longestlength = l;
						longestedge = e;
					}
				}
				tri = longestedge->triangle;
				bool splitedge = true;
				if (splitedge) {
					/*split an edge*/
					int randindex = tri->getEdgeIndex(longestedge);
					Edge* rand = longestedge;
					auto* ap = rand->startPoint;
					auto* bp = rand->pair->startPoint;
					const Vector3F& a = ap->position;
					const Vector3F& b = bp->position;

					float rands {random.nextFloat()};
					Vector3F npos {a + (b - a) * (0.5f + (rands - 0.5f) * 0.3f)};
					npos.z() += (random.nextFloat() - 0.5f) * 0.5f;
					if(npos.z() < -SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD * 0.95f) {
						npos.z() = -SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD* 0.95f;
					}
					if(npos.z() > SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD * 0.95f) {
						npos.z() = SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD * 0.95f;
					}
					if(rand->pair->nextEdge != nullptr) {
						/*point is not on and edge, both sides have faces*/
					} else {
						/*point is on an edge, should weld with other side*/
						/*find opposite*/
						Vector2F nposxy = npos.xy();
						Vector2F dir = rand->getVectorXY().normalized();
						const Vector2F target = nposxy + Vector2F {dir.y(), -dir.x()};
						Edge* it = rand->nextEdge;
						//LOGI() << "Pos: " << nposxy << " target: " << target << " with dir: " << dir;
						while (true) {
							while(it->nextEdge != nullptr) {
								it = it->nextEdge;
							}
							/*it points to a side edge, test it*/
							float dotres = it->getVectorXY().normalized().dot(dir);
							if (dotres >= 0.999 && dotres <= 1.001) {
								/*TODO*/
								if((target - it->pair->startPoint->position.xy()).dot(dir) < 0) {
									//LOGI() << "FOUND for: " << nposxy << ": " << it->startPoint->position.xy()
									//<< " fdir: " << it->getVectorXY().normalize();
									Point& np = points[pointcount++];
									np.position.xy() = target;
									np.position.z() = npos.z();
									np.edge = it->pair;

									Triangle& ntri = triangles[tricount++];

									Edge* ne = &edges[edgecount++];
									ne->startPoint = &np;
									ne->nextEdge = nullptr;/*it->nextEdge could be too, but it is nullptr*/
									ne->pair = &edges[edgecount++];
									ne->triangle = nullptr;

									ne->pair->pair = ne;
									ne->pair->startPoint = it->pair->startPoint;
									ne->pair->nextEdge = it->pair->nextEdge;
									ne->pair->triangle = &ntri;

									Edge* e = &edges[edgecount++];
									e->startPoint = it->pair->nextEdge->pair->startPoint;
									e->triangle = it->pair->triangle;
									e->nextEdge = it->pair->nextEdge->pair->nextEdge;
									e->pair = &edges[edgecount++];

									e->pair->triangle = &ntri;
									e->pair->nextEdge = ne;
									e->pair->startPoint = &np;
									e->pair->pair = e;

									ne->pair->nextEdge->pair->nextEdge = e;
									ne->pair->nextEdge->pair->triangle = &ntri;

									ntri = Triangle {ne->pair, e->pair, ne->pair->nextEdge->pair};

									it->pair->triangle->edges[(it->pair->triangle->getEdgeIndex(it->pair) + 2) % 3] = e;

									it->pair->startPoint = &np;
									it->pair->nextEdge = e->pair;

									break;
								}
							}
							it = it->pair;
						}
					}
					auto& p = points[pointcount];
					p.position = npos;
					p.edge = rand;

					Edge& randopp = edges[edgecount++];
					Edge& randopppair = edges[edgecount++];
					randopp.startPoint = ap;
					randopp.nextEdge = rand->nextEdge;
					randopp.pair = &randopppair;

					randopppair.startPoint = &p;
					randopppair.pair = &randopp;
					if(rand->pair->nextEdge != nullptr) {
						randopppair.nextEdge = rand;
					} else {
						randopppair.nextEdge = nullptr;
					}

					rand->startPoint = &p;
					rand->nextEdge = &randopppair;

					++pointcount;

					ASSERT(randopp.nextEdge != nullptr);
					ASSERT(randopp.nextEdge->pair->nextEdge->pair->nextEdge == rand->pair);

					auto* cp = randopp.nextEdge->pair->startPoint;
					Edge& ce = edges[edgecount++];
					Edge& cep = edges[edgecount++];
					ce.startPoint = cp;
					ce.nextEdge = randopp.nextEdge->pair->nextEdge;
					ce.pair = &cep;

					cep.startPoint = &p;
					cep.pair = &ce;
					cep.nextEdge = &randopppair;

					rand->nextEdge = &cep;
					randopp.nextEdge->pair->nextEdge = &ce;

					Triangle* ntri = &triangles[tricount++];
					ASSERT(tri->edges[(randindex + 2) % 3] == randopp.nextEdge->pair);
					tri->edges[(randindex + 2) % 3] = &ce;
					*ntri = Triangle {&randopp, &cep, randopp.nextEdge->pair};

					ce.triangle = tri;
					cep.triangle = ntri;
					randopp.triangle = ntri;
					randopp.nextEdge->pair->triangle = ntri;

					if (rand->pair->nextEdge != nullptr) {
						rand->pair->nextEdge->pair->nextEdge->pair->nextEdge = &randopp;

						auto* dp = rand->pair->nextEdge->pair->startPoint;
						Edge& de = edges[edgecount++];
						Edge& dep = edges[edgecount++];
						de.startPoint = dp;
						de.nextEdge = rand->pair->nextEdge->pair->nextEdge;
						de.pair = &dep;

						dep.startPoint = &p;
						dep.nextEdge = rand;
						dep.pair = &de;

						randopp.pair->nextEdge = &dep;
						rand->pair->nextEdge->pair->nextEdge = &de;

						Triangle* ntri = &triangles[tricount++];
						Triangle* oldtri = rand->pair->triangle;
						ASSERT(oldtri->edges[(oldtri->getEdgeIndex(rand->pair) + 1) % 3] == de.nextEdge->pair);
						oldtri->edges[(oldtri->getEdgeIndex(rand->pair) + 1) % 3] = &dep;
						*ntri = Triangle {&randopppair, de.nextEdge->pair, &de};

						de.nextEdge->pair->triangle = ntri;
						de.triangle = ntri;
						dep.triangle = oldtri;
						randopp.pair->triangle = ntri;

					}
				} else {
					/*disable for now, only edge split gives better looking result*/
					continue;
					/*make a point on a triangle*/
					split_triangle_start_label:

					int randindex = random.next(3);
					Edge* rand = tri->edges[randindex];
					ASSERT(rand->nextEdge != nullptr) << rand->nextEdge;
					if (rand->nextEdge == nullptr) {
						/*random again, since we found an edge that is on the side of the mesh*/
						goto split_triangle_start_label;
					}
					auto* ap = rand->startPoint;
					auto* bp = rand->nextEdge->pair->startPoint;
					auto* cp = rand->pair->startPoint;
					const Vector3F& a = ap->position;
					const Vector3F& b = bp->position;
					const Vector3F& c = cp->position;
					float rands[] {random.nextFloat(), random.nextFloat(), random.nextFloat()};
					Vector3F npos {(a * (0.5f + 0.5f * rands[0]) + b * (0.5f + 0.5f * rands[1]) + c * (0.5f + 0.5f * rands[2]))
						/ (3.0f * 0.5f + 0.5f * (rands[0] + rands[1] + rands[2]))};
					npos.z() += (random.nextFloat() - 0.5f) * 0.5f;
					auto& p = points[pointcount];
					auto& ae = edges[edgecount];
					auto& be = edges[edgecount + 1];
					auto& ce = edges[edgecount + 2];
					auto& aep = edges[edgecount + 3];
					auto& bep = edges[edgecount + 4];
					auto& cep = edges[edgecount + 5];

					p.position = npos;
					p.edge = &aep;

					ae.startPoint = ap;
					be.startPoint = bp;
					ce.startPoint = cp;
					aep.startPoint = &p;
					bep.startPoint = &p;
					cep.startPoint = &p;

					ae.nextEdge = rand->nextEdge;
					be.nextEdge = rand->nextEdge->pair->nextEdge;
					ce.nextEdge = rand->pair;
					aep.nextEdge = &cep;
					bep.nextEdge = &aep;
					cep.nextEdge = &bep;

					ae.pair = &aep;
					be.pair = &bep;
					ce.pair = &cep;
					aep.pair = &ae;
					bep.pair = &be;
					cep.pair = &ce;

					rand->nextEdge->pair->nextEdge->pair->nextEdge = &ce;
					rand->nextEdge->pair->nextEdge = &be;
					rand->nextEdge = &ae;

					Triangle* ta = tri;
					Triangle* tb = &triangles[tricount++];
					Triangle* tc = &triangles[tricount++];

					*tb = Triangle {&ae, &bep, ae.nextEdge->pair};
					*tc = Triangle {&cep, be.nextEdge->pair, &be};

					tri->edges[(randindex + 1) % 3] = &ce;
					tri->edges[(randindex + 2) % 3] = &aep;

					ASSERT(rand->triangle == ta);
					ae.triangle = tb;
					aep.triangle = ta;
					be.triangle = tc;
					bep.triangle = tb;
					ce.triangle = ta;
					cep.triangle = tc;

					be.nextEdge->pair->triangle = tc;
					ae.nextEdge->pair->triangle = tb;

					edgecount+= 6;
					++pointcount;
				}
			}

			for (unsigned int i = 0; i < tricount; ++i) {
				Triangle* tri = &triangles[i];

				auto& ap = *tri->edges[0]->startPoint;
				auto& bp = *tri->edges[1]->startPoint;
				auto& cp = *tri->edges[2]->startPoint;
				const Vector3F& a = ap.position;
				const Vector3F& b = bp.position;
				const Vector3F& c = cp.position;

				Vector3F u = c - a;
				Vector3F v = b - a;

				Vector4F normal;
				normal.x() = u.y() * v.z() - u.z() * v.y();
				normal.y() = u.z() * v.x() - u.x() * v.z();
				normal.z() = u.x() * v.y() - u.y() * v.x();
				normal.w() = 0.0f;
				normal.normalize();

				auto& va = initer[0];
				auto& vb = initer[1];
				auto& vc = initer[2];

				va.a_position.xyz() = b;
				vb.a_position.xyz() = a;
				vc.a_position.xyz() = c;
				va.a_position.w() = 1.0f;
				vb.a_position.w() = 1.0f;
				vc.a_position.w() = 1.0f;

				va.a_normal = normal;
				vb.a_normal = normal;
				vc.a_normal = normal;

				initer += 3;
				/*assert to be user facing normals*/
				ASSERT(normal.dot(Vector4F {0,0,1,0}) >= 0.0f) << normal.dot(Vector4F {0,0,1,0}) << " " << normal;
			}

			delete[] points;
			delete[] edges;
			delete[] triangles;

			backgroundTriangleCount = tricount;
			//LOGI() << "Background triangle count: " << backgroundTriangleCount;
		}, MAX_TRIANGLE_COUNT * 3);
	backgroundVertexBuffer->initialize(BufferType::IMMUTABLE);

	backgroundInputLayout = Resource<SapphirePhongShader::InputLayout> { sapphirePhongShader->createInputLayout(),
			[&](SapphirePhongShader::InputLayout* il) {
				il->setLayout<SapphirePhongShader::VertexInput>(backgroundVertexBuffer);
			} };
}

void Level3DBackground::disable() {
	backgroundTriangleCount = 0;
	backgroundInputLayout = nullptr;
	backgroundVertexBuffer = nullptr;
}

StandaloneLevel3DBackground::StandaloneLevel3DBackground(bool enabled, unsigned int randomseed)
		: Level3DBackground(enabled, randomseed) {
}

void StandaloneLevel3DBackground::drawSimpleBackground(float alpha, float aspectratio) {
	drawSimpleBackground(alpha, aspectratio, Vector2F { 0, 0 });
}

void StandaloneLevel3DBackground::drawSimpleBackground(float alpha, float aspectratio, const Vector2F& lightpos) {
	float depth = 5.0f;
	Matrix3D projtrans;
	projtrans.setOrthographic(renderer, -aspectratio / 2, aspectratio / 2, -0.5f, 0.5f, (depth - SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD),
			(depth + SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD));

	const Color ambientlighting { alpha, alpha, alpha, 1.0f };

	renderer->setFaceCulling(true);
	renderer->setDepthTest(true);
	renderer->initDraw();
	renderer->clearDepthBuffer();
	renderer->setTopology(Topology::TRIANGLES);

	activateInputLayout();
	Matrix3D viewtrans;
	Matrix3D viewinverse;

	viewtrans.setTranslate(0, 0, -depth);
	viewinverse.setTranslate(0, 0, depth);

	float backgroundscale = max(aspectratio, 1.0f) + 1.0f;
	sapphirePhongShader->useProgram();
	Vector4F lightingposition { lightpos, 7.0f + aspectratio, 1.0f };
	colorStateUniform->update( { viewtrans, viewinverse.transposed(), projtrans, lightingposition });
	colorAmbientLighting->update( { ambientlighting });

	colorMaterialUniform->update( { Color { SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR, SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR,
	SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR, alpha }, Color { 0.00f, 0.00f, 0.00f, alpha }, Color { 0.00f, 0.00f, 0.00f, alpha }, 0.0f });
	Matrix3D backmvp;
	Matrix3D backmvpinverse;
	backmvp.setIdentity().multTranslate(-0.5f, -0.5f, 0).multScale(backgroundscale, backgroundscale, 1);
	backmvpinverse.setIdentity().multScale(1.0f / backgroundscale, 1.0f / backgroundscale, 1.0f).multTranslate(0.5f, 0.5f, 0);
	colorShaderUniform->update( { backmvp, backmvpinverse.transposed() });

	sapphirePhongShader->set(colorMaterialUniform);
	sapphirePhongShader->set(colorStateUniform);
	sapphirePhongShader->set(colorAmbientLighting);
	sapphirePhongShader->set(colorShaderUniform);

	sapphirePhongShader->draw(0, getTriangleCount() * 3);
}

} // namespace userapp

