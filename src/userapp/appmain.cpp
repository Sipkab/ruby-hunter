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
 * appmain.cpp
 *
 *  Created on: 2016. febr. 23.
 *      Author: sipka
 */

#include <framework/render/Renderer.h>
#include <framework/core/AppInterface.h>
#include <framework/core/Window.h>
#include <framework/scene/SceneManager.h>
#include <framework/resource/PackageResource.h>
#include <framework/resource/ResourcePool.h>
#include <framework/resource/ResourceInputSource.h>
#include <framework/resource/BitmapInputSource.h>
#include <framework/geometry/VertexDataBaseChanger.h>
#include <framework/utils/utility.h>
#include <framework/render/IndexBuffer.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>

#include <appmain.h>
#include <QuadIndexBuffer.h>
#include <sapphire/levelrender/LevelDrawer3D.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/sapphireconstants.h>

#include <gen/configuration.h>
#include <gen/resources.h>
#include <gen/log.h>
#include <gen/xmlcompile.h>
#include <gen/renderers.h>
#include <gen/audiomanagers.h>
#include <gen/platform.h>

#include <gen/shader/SimpleColorShader.h>
#include <gen/shader/SimpleFontShader.h>

#include <sapphire/steam_opt.h>

#if defined(RHFW_PLATFORM_WIN32)
#include <win32platform/gamepad/Win32GamePadContext.h>
#endif /* defined(RHFW_PLATFORM_WIN32) */

using namespace rhfw;

namespace userapp {

FixedString numberToString(unsigned int i) {
	if (i == 0) {
		return "0";
	}
	char buffer[32];
	buffer[sizeof(buffer) - 1] = 0;
	char* ptr = buffer + sizeof(buffer) - 1;
	for (; i; i /= 10) {
		*--ptr = '0' + i % 10;
	}
	return FixedString { ptr };
}
unsigned int stringToNumber(const char* str, unsigned int len, unsigned int defaultvalue) {
	if (len == 0) {
		return defaultvalue;
	}
	unsigned int result = str[0] - '0';
	for (unsigned int i = 1; i < len; ++i) {
		result = result * 10 + (str[i] - '0');
	}
	return result;
}
unsigned int stringToNumber(const char* str, unsigned int len) {
	return stringToNumber(str, len, 0);
}
unsigned int stringToNumber(const FixedString& str) {
	return stringToNumber(str, str.length());
}
unsigned int stringToNumber(const FixedString& str, unsigned int defaultvalue) {
	return stringToNumber(str, str.length(), defaultvalue);
}

class QuadIndexBuffer;

StartConfiguration startConfig;

GamePadContext* gamepadContext = nullptr;

Resource<audio::AudioManager> audioManager;
Resource<rhfw::render::Renderer> renderer;
rhfw::AutoResource<rhfw::SimpleColorShader> simpleColorShader;
rhfw::AutoResource<rhfw::SimpleFontShader> simpleFontShader;

rhfw::AutoResource<rhfw::SapphireTextureShader> sapphireTextureShader;
rhfw::AutoResource<rhfw::SapphirePhongShader> sapphirePhongShader;
rhfw::AutoResource<rhfw::SapphireTexturedPhongShader> sapphireTexturedPhongShader;

const StartConfiguration& getStartConfiguration() {
	return startConfig;
}

static render::Renderer* instantiateRenderer(RenderConfig start) {
	auto current = start;
	rhfw::render::Renderer* res = nullptr;
	do {
		res = rhfw::instantiateRenderer(current);
		current = (RenderConfig) (((unsigned int) current + 1) % (unsigned int) RenderConfig::_count_of_entries);
	} while (res == nullptr && current != start);
	ASSERT(res != nullptr) << "Failed to create renderer";
	if (res != nullptr) {
		auto type = res->getRendererType();
		if (type != start) {
			startConfig.renderConfig = type;
		}
	}
	return res;
}

void changeRenderer(rhfw::RenderConfig renderconfig) {
	LOGTRACE() << "Loading renderer: " << renderconfig;
	auto* nrenderer = userapp::instantiateRenderer(renderconfig);
	LOGTRACE() << "Instantiated renderer: " << renderconfig;
	auto* old = renderer.replace(nrenderer);
	*nrenderer = util::move(*old);
	delete old;
	LOGTRACE() << "Loaded renderer: " << renderconfig;
}

void swapRenderer() {
	unsigned int val = ((unsigned int) renderer->getRendererType() + 1) % (unsigned int) RenderConfig::_count_of_entries;
	changeRenderer((RenderConfig) val);
}
void reloadRenderer() {
	LOGTRACE() << "Reloading renderer: " << renderer->getRendererType();
	renderer.reload();
}
void addNewWindow() {
#if PLATFORM == PLATFORM_WIN32

#endif
}

class Main_WindowListener: public core::WindowStateListener {
public:
	int argc = 0;
	char** argv = nullptr;

	virtual void onWindowCreated(core::Window& window, void* args) override {
		window.attachToRenderer(renderer);

		SceneManager* manager = new SceneManager();
		Scene* created = manager->create(&window, ResIds::gameres::scene_sapphire);
		static_cast<SapphireScene*>(created)->setProgramArguments(argc, argv);
		window.setTag(manager);
	}

	virtual void onWindowDestroyed(core::Window& window) override {
		delete static_cast<SceneManager*>(window.getTag());

		window.detachFromRenderer();
	}
};
static Main_WindowListener main_windowlistener;
static AutoResource<render::VertexBuffer> generalVertexBuffer;

static AutoResource<render::VertexBuffer> sapphireTextureVertexBuffer;
static AutoResource<SapphireTextureShader::InputLayout> sapphireTextureInputLayout;
static AutoResource<SimpleColorShader::InputLayout> sapphireColorInputLayout;
static AutoResource<SapphireTextureShader::MVP> sapphireTextureMVP;
static AutoResource<SapphireTextureShader::UTexture> sapphireTextureUTexture;
static AutoResource<SapphireTextureShader::UColorMultiply> sapphireTextureUColorMultiply;

static AutoResource<SimpleColorShader::MVP> color_mvpu;
static AutoResource<SimpleColorShader::ColorUniform> color_coloru;

static AutoResource<SimpleFontShader::MVP> font_mvpu;
static AutoResource<SimpleFontShader::UTexture> font_textu;
static AutoResource<SimpleFontShader::InputLayout> font_il;

template<typename T>
static void ensureGeneralVertexBufferSize(unsigned count) {
	if (generalVertexBuffer->getAllocatedByteCount() < count * sizeof(T)) {
		generalVertexBuffer->allocate<T>(count, BufferType::DYNAMIC);
	}
}
void drawRectangleColor(const rhfw::Matrix<4>& mvp, const rhfw::Color& color, const rhfw::Rectangle& target) {
	sapphireColorInputLayout->activate();

	color_mvpu->update( {
			Matrix3D().setMatrix(Matrix2D().setScale(target.width(), target.height()).multTranslate(target.left, target.top)) *= mvp });
	color_coloru->update( { color });

	renderer->setTopology(Topology::TRIANGLE_STRIP);

	simpleColorShader->useProgram();
	simpleColorShader->set(color_mvpu);
	simpleColorShader->set(color_coloru);

	simpleColorShader->drawCount(4);
}
void drawRectangleColor(const rhfw::Matrix<3>& mvp, const rhfw::Color& color, const rhfw::Rectangle& target) {
	sapphireColorInputLayout->activate();

	color_mvpu->update(
			{ Matrix3D().setMatrix(Matrix2D().setScale(target.width(), target.height()).multTranslate(target.left, target.top) *= mvp) });
	color_coloru->update( { color });

	renderer->setTopology(Topology::TRIANGLE_STRIP);

	simpleColorShader->useProgram();
	simpleColorShader->set(color_mvpu);
	simpleColorShader->set(color_coloru);

	simpleColorShader->drawCount(4);
}
void prepareSapphireTextureDraw() {
	renderer->setTopology(Topology::TRIANGLE_STRIP);
	sapphireTextureShader->useProgram();
	sapphireTextureInputLayout->activate();
}
void drawSapphireTexturePrepared(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, float alpha, const rhfw::Rectangle& target,
		const rhfw::Rectangle& textureSource) {
	drawSapphireTexturePrepared(mvp, text, Color { 1, 1, 1, alpha }, target, textureSource);
}
void drawSapphireTexturePrepared(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, const rhfw::Color& colormult,
		const rhfw::Rectangle& target, const rhfw::Rectangle& textureSource) {
	sapphireTextureMVP->update( { //
			Matrix3D().setMatrix(Matrix<3>(). //
			setScale(target.width(), target.height()) //
			.multTranslate(target.left, target.top) *= mvp), //

			Matrix2D() //
			.setScale(textureSource.width(), textureSource.height()) //
			.multTranslate(textureSource.leftTop()) //
			});
	sapphireTextureUTexture->update( { &text });
	sapphireTextureUColorMultiply->update( { colormult });

	sapphireTextureShader->set(sapphireTextureMVP);
	sapphireTextureShader->set(sapphireTextureUTexture);
	sapphireTextureShader->set(sapphireTextureUColorMultiply);

	sapphireTextureShader->drawCount(4);
}

void drawSapphireTexture(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, float alpha, const rhfw::Rectangle& target,
		const rhfw::Rectangle& textureSource) {
	drawSapphireTexture(mvp, text, Color { 1, 1, 1, alpha }, target, textureSource);
}
void drawSapphireTexture(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, const rhfw::Color& colormult,
		const rhfw::Rectangle& target, const rhfw::Rectangle& textureSource) {
	sapphireTextureMVP->update( { //
			Matrix3D().setMatrix(Matrix<3>(). //
			setScale(target.width(), target.height()) //
			.multTranslate(target.left, target.top) *= mvp), //

			Matrix2D() //
			.setScale(textureSource.width(), textureSource.height()) //
			.multTranslate(textureSource.leftTop()) //
			});
	sapphireTextureUTexture->update( { &text });
	sapphireTextureUColorMultiply->update( { colormult });

	renderer->setTopology(Topology::TRIANGLE_STRIP);

	sapphireTextureShader->useProgram();
	sapphireTextureShader->set(sapphireTextureMVP);
	sapphireTextureShader->set(sapphireTextureUTexture);
	sapphireTextureShader->set(sapphireTextureUColorMultiply);

	sapphireTextureInputLayout->activate();
	sapphireTextureShader->drawCount(4);
}

QuadIndexBuffer quadIndexBuffer;

float drawString(const rhfw::Matrix<3>& mvp, const char* str, rhfw::Font& font, const rhfw::Color& color, const rhfw::Vector2F& pos,
		float size, rhfw::Gravity gravity) {
	ASSERT(str != nullptr) << "String to draw is nullptr";

	unsigned int length = 0;
	const float pxwidth = font.measureText(str, size, &length);
	if (length == 0) {
		return 0.0f;
	}
	{
		ensureGeneralVertexBufferSize<SimpleFontShader::VertexInput>(length * 4);
		auto initer = generalVertexBuffer->mapped<SimpleFontShader::VertexInput>(0, length * 4);

		SimpleFontShader::VertexInput* input = (SimpleFontShader::VertexInput*) &initer[0];
		font.fillBufferDataWithCharacters(str, str + length, input, size, pos, gravity, pxwidth,
				[&](SimpleFontShader::VertexInput& i, const Vector2F& pos, const Vector2F& tex) {
					i.a_position = Vector4F {pos.xy(), 0.0f, 1.0f};
					i.a_texcoord = tex;
					i.a_color = color;
				});
	}

	font_il->activate();

	font_mvpu->update( { Matrix<4>().setMatrix(mvp) });
	font_textu->update( { &font.getTexture() });

	renderer->setTopology(Topology::TRIANGLES);

	simpleFontShader->useProgram();
	simpleFontShader->set(font_mvpu);
	simpleFontShader->set(font_textu);

	quadIndexBuffer.ensureLength(length);
	quadIndexBuffer.activate();

	simpleFontShader->drawIndexedCount(length * 6);
	return pxwidth;
}
float drawString(const rhfw::Matrix<3>& mvp, const char* begin, const char* end, rhfw::Font& font, const rhfw::Color& color,
		const rhfw::Vector2F& pos, float size, rhfw::Gravity gravity) {
	if (begin == end) {
		return 0.0f;
	}

	unsigned int length = end - begin;
	const float pxwidth = font.measureText(begin, end, size);
	{
		ensureGeneralVertexBufferSize<SimpleFontShader::VertexInput>(length * 4);
		auto initer = generalVertexBuffer->mapped<SimpleFontShader::VertexInput>(0, length * 4);

		SimpleFontShader::VertexInput* input = (SimpleFontShader::VertexInput*) &initer[0];
		font.fillBufferDataWithCharacters(begin, end, input, size, pos, gravity, pxwidth,
				[&](SimpleFontShader::VertexInput& i, const Vector2F& pos, const Vector2F& tex) {
					i.a_position = Vector4F {pos.xy(), 0.0f, 1.0f};
					i.a_texcoord = tex;
					i.a_color = color;
				});
	}

	font_il->activate();

	font_mvpu->update( { Matrix<4>().setMatrix(mvp) });
	font_textu->update( { &font.getTexture() });

	renderer->setTopology(Topology::TRIANGLES);

	simpleFontShader->useProgram();
	simpleFontShader->set(font_mvpu);
	simpleFontShader->set(font_textu);

	quadIndexBuffer.ensureLength(length);
	quadIndexBuffer.activate();

	simpleFontShader->drawIndexedCount(length * 6);
	return pxwidth;
}
void updateStartConfig(rhfw::RenderConfig renderconfig, rhfw::VSyncOptions vsync, unsigned int msaa) {
	if (renderconfig == startConfig.renderConfig && vsync == startConfig.vsyncOptions && startConfig.msaaFactor == msaa) {
		//no changes
		return;
	}
	startConfig.renderConfig = renderconfig;
	startConfig.msaaFactor = msaa;
	startConfig.vsyncOptions = vsync;
	StorageFileDescriptor configini { StorageDirectoryDescriptor::Root() + "config.ini" };
	startConfig.save(configini);
}

} // namespace userapp

using namespace userapp;

namespace rhfw {

CREATE_ENDIAN_DESERIALIZE_FUNCTION(Resource<render::Texture>, is, outdata){
{
	ResId id;
	bool success = SerializeHandler<ResId>::deserialize<ENDIAN>(is, id);
	if(!success) {
		return false;
	}
	outdata = userapp::getTexture(id);
	return true;
}
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(Resource<render::Texture>)
CREATE_ENDIAN_DESERIALIZE_FUNCTION(Resource<userapp::FrameAnimation>, is, outdata){
{
	ResId id;
	bool success = SerializeHandler<ResId>::deserialize<ENDIAN>(is, id);
	if(!success) {
		return false;
	}
	outdata = userapp::getAnimation(id);
	return true;
}
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(Resource<userapp::FrameAnimation>)

}  // namespace rhfw

static const RenderConfig DEFAULT_RENDER_CONFIG =
#if defined(RHFW_PLATFORM_ANDROID)
		RenderConfig::OpenGlEs20
#elif defined(RHFW_PLATFORM_IOS)
		RenderConfig::OpenGlEs20
#elif defined(RHFW_PLATFORM_WINDOWSSTORE)
		RenderConfig::DirectX11
#elif defined(RHFW_PLATFORM_WIN32)
		RenderConfig::DirectX11
#elif defined(RHFW_PLATFORM_MACOSX)
RenderConfig::OpenGl30
#elif defined(RHFW_PLATFORM_LINUX)
RenderConfig::OpenGl30
#else
static_assert(false, "unknown platform");
#endif
;

static const AudioConfig DEFAULT_AUDIO_CONFIG =
#if defined(RHFW_PLATFORM_ANDROID)
		AudioConfig::OpenSLES10Android
#elif defined(RHFW_PLATFORM_IOS)
		AudioConfig::OpenAL10
#elif defined(RHFW_PLATFORM_LINUX)
		AudioConfig::OpenAL10
#elif defined(RHFW_PLATFORM_MACOSX)
		AudioConfig::OpenAL10
#elif defined(RHFW_PLATFORM_WINDOWSSTORE) || defined(RHFW_PLATFORM_WIN32)
		AudioConfig::XAudio2
#else
static_assert(false, "unknown platform");
#endif
;

#if defined(RHFW_PLATFORM_LINUX)
#include <stdlib.h>
void userapp::showFatalDialogAndExit(const char* text, int exitvalue) {
	system(FixedString {"zenity --error --text=\""}+ text + "\"");
	exit(exitvalue);
}
#elif defined(RHFW_PLATFORM_WIN32)
#include <Windows.h>
void userapp::showFatalDialogAndExit(const char* text, int exitvalue) {
	MessageBoxA(NULL, text, "Error", MB_ICONERROR | MB_OK);
	exit(exitvalue);
}
#else
#include <stdio.h>
void userapp::showFatalDialogAndExit(const char* text, int exitvalue) {
	printf("%s", text);
	fprintf(stderr, "%s", text);
	exit(exitvalue);
}
#endif

class StorageDirectoryMover {
public:

#if (defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_LINUX))
	static void executeMoving() {
		LOGTRACE() << "Move from old storage directory to new";
		StorageDirectoryDescriptor rootdir { StorageDirectoryDescriptor::Root() };
		for (auto&& x : rootdir.enumerate()) {
			//root dir is not empty, do not copy anything there
			return;
		}
		StorageDirectoryDescriptor olddir = StorageDirectoryDescriptor::ApplicationDataDirectory(SAPPHIRE_GAME_OLD_NAME);
		for (auto&& p : olddir.enumerate()) {
			StorageFileDescriptor oldfile { olddir.getPath() + p };
			StorageFileDescriptor nfile { rootdir.getPath() + p };
			oldfile.move(nfile);
		}
	}
#else
	static void executeMoving() {
		//ignore
	}
#endif /* (defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_LINUX)) */

};

void user_app_initialize(int argc, char* argv[]) {
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	if (SteamAPI_RestartAppIfNecessary(SAPPHIRE_STEAM_APP_ID)) {
		LOGI() << "RestartAppIfNecessary restarting";
		exit(1);
	}
	if (!SteamAPI_Init()) {
		LOGI() << "SteamAPI_Init failed";
		showFatalDialogAndExit("SteamAPI_Init failed", 2);
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

	StorageDirectoryMover::executeMoving();

	StorageFileDescriptor configini { StorageDirectoryDescriptor::Root() + "config.ini" };
	startConfig.load(configini);
	if (startConfig.renderConfig == RenderConfig::_count_of_entries) {
		startConfig.renderConfig = DEFAULT_RENDER_CONFIG;
	}

	render::RenderingContextOptions renderoptions;
	renderoptions.multiSamplingFactor = startConfig.msaaFactor;
	renderoptions.vsyncOptions = startConfig.vsyncOptions;

	audioManager = Resource<audio::AudioManager> { new ResourceBlock(audio::instantiateAudioManager(DEFAULT_AUDIO_CONFIG)) };
	{
		bool loadres;
		auto start = startConfig.renderConfig;
		auto current = start;
		do {
			renderer = Resource<render::Renderer> { new ResourceBlock(userapp::instantiateRenderer(startConfig.renderConfig)) };
			renderer->setRenderingContextOptions(renderoptions);
			loadres = renderer.load();
			if (!loadres) {
				current = (RenderConfig) (((unsigned int) current + 1) % (unsigned int) RenderConfig::_count_of_entries);
			} else {
				break;
			}
		} while (current != start);
		if (!loadres) {
			showFatalDialogAndExit("Failed to set up rendering layer.", -1);
		}
	}
	main_windowlistener.argc = argc;
	main_windowlistener.argv = argv;
	core::WindowStateListener::subscribeListener(main_windowlistener);

	simpleColorShader = Resource<rhfw::SimpleColorShader> { renderer->getShaderProgram(UnifiedShaders::SimpleColorShader) };
	simpleFontShader = Resource<rhfw::SimpleFontShader> { renderer->getShaderProgram(UnifiedShaders::SimpleFontShader) };

	sapphireTextureShader = Resource<rhfw::SapphireTextureShader> { renderer->getShaderProgram(UnifiedShaders::SapphireTextureShader) };
	sapphirePhongShader = Resource<rhfw::SapphirePhongShader> { renderer->getShaderProgram(UnifiedShaders::SapphirePhongShader) };
	sapphireTexturedPhongShader = Resource<rhfw::SapphireTexturedPhongShader> { renderer->getShaderProgram(
			UnifiedShaders::SapphireTexturedPhongShader) };

	quadIndexBuffer.load();
	generalVertexBuffer = renderer->createVertexBuffer();
	ensureGeneralVertexBufferSize<Vector4F>(4096);

	sapphireTextureVertexBuffer = renderer->createVertexBuffer();
	sapphireTextureVertexBuffer->setBufferInitializer<SapphireTextureShader::VertexInput>([](SapphireTextureShader::VertexInput* ptr) {
		/*left bottom*/
		ptr[0].a_position= {0,1,0,1};
		ptr[0].a_texcoord= {0,1};

		/*right bottom*/
		ptr[1].a_position= {1,1,0,1};
		ptr[1].a_texcoord= {1,1};

		/*left top*/
		ptr[2].a_position= {0,0,0,1};
		ptr[2].a_texcoord= {0,0};

		/*right top*/
		ptr[3].a_position= {1,0,0,1};
		ptr[3].a_texcoord= {1,0};
	}, 4);
	sapphireTextureVertexBuffer->initialize(BufferType::IMMUTABLE);
	sapphireTextureMVP = sapphireTextureShader->createUniform_MVP();
	sapphireTextureUTexture = sapphireTextureShader->createUniform_UTexture();
	sapphireTextureUColorMultiply = sapphireTextureShader->createUniform_UColorMultiply();

	color_mvpu = simpleColorShader->createUniform_MVP();
	color_coloru = simpleColorShader->createUniform_ColorUniform();

	font_mvpu = simpleFontShader->createUniform_MVP();
	font_textu = simpleFontShader->createUniform_UTexture();

	font_il = Resource<SimpleFontShader::InputLayout> { simpleFontShader->createInputLayout(), [](SimpleFontShader::InputLayout* il) {
		il->setLayout(generalVertexBuffer);
	} };
	sapphireTextureInputLayout = Resource<SapphireTextureShader::InputLayout> { sapphireTextureShader->createInputLayout(),
			[](SapphireTextureShader::InputLayout* il) {
				il->setLayout(sapphireTextureVertexBuffer);
			} };
	sapphireColorInputLayout = Resource<SimpleColorShader::InputLayout> { simpleColorShader->createInputLayout(),
			[](SimpleColorShader::InputLayout* il) {
				il->setLayout<SapphireTextureShader::VertexInput>(sapphireTextureVertexBuffer);
			} };

#if defined(RHFW_PLATFORM_WIN32)
	gamepadContext = new Win32GamePadContext();
#endif /* defined(RHFW_PLATFORM_WIN32) */

}

void user_app_terminate() {
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	SteamAPI_Shutdown();
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
	delete gamepadContext;
	gamepadContext = nullptr;

	color_mvpu = nullptr;
	color_coloru = nullptr;

	font_mvpu = nullptr;
	font_textu = nullptr;

	font_il = nullptr;
	sapphireColorInputLayout = nullptr;

	simpleFontShader = nullptr;
	simpleColorShader = nullptr;

	sapphireTextureInputLayout = nullptr;
	sapphireTextureVertexBuffer = nullptr;
	sapphireTextureMVP = nullptr;
	sapphireTextureUTexture = nullptr;
	sapphireTextureUColorMultiply = nullptr;

	sapphireTextureShader = nullptr;
	sapphirePhongShader = nullptr;
	sapphireTexturedPhongShader = nullptr;

	generalVertexBuffer = nullptr;
	quadIndexBuffer.free();

	PackageResource::purge();

	renderer.free();

	core::WindowStateListener::unsubscribeListener(main_windowlistener);
	renderer = nullptr;
	audioManager = nullptr;

}

namespace userapp {

rhfw::Resource<rhfw::render::Texture> getTexture(rhfw::ResId id) {
	return PackageResource::getResourceOrFactory<render::Texture>(id, [&] {
		auto texture = renderer->createTexture();
		auto* is = new BitmapInputSource {new ResourceInputSource {id}};
		is->setColorFormat(ColorFormat::RGBA_8888);
		texture->setInputSource(is);
		return util::move(texture);
	});
}

rhfw::Resource<rhfw::Font> getFont(rhfw::ResId id) {
	return PackageResource::getResourceOrFactory<Font>(id, [&]() {
		return Resource<Font> {new ResourceBlock {new Font(id)}};
	});
}

rhfw::Resource<FrameAnimation> getAnimation(rhfw::ResId id) {
	return PackageResource::getResourceOrFactory<FrameAnimation>(id, [&]() {
		return Resource<FrameAnimation> {new ResourceBlock {new FrameAnimation {id}}};
	});
}

}  // namespace userapp
