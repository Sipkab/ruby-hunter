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
//package bence.sipka.user.sapphire;
//
//import java.awt.Rectangle;
//import java.awt.image.BufferedImage;
//import java.io.IOException;
//import java.io.InputStream;
//import java.io.OutputStream;
//import java.io.PrintStream;
//import java.util.ArrayList;
//import java.util.Collection;
//import java.util.Collections;
//
//import javax.imageio.ImageIO;
//
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularParameter;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.file.content.FileContentDescriptor;
//import modular.core.module.ModuleOperation;
//import saker.build.file.path.WildcardPath;
//import sun.security.pkcs11.Secmod.Module;
//
//@ModuleURI(SapphireTextureConverter.MODULE_URL)
//public class SapphireTextureConverter extends Module {
//	public static final String MODULE_URL = "bence.sipka.sapphire.textureconverter";
//
//	static final int TILE_DIMENSION = 60;
//	static final int TILE_PADDING = 2;
//
//	private static int calcDim(int rem) {
//		int dim = 16;
//		while (rem < dim * dim / 4) {
//			dim /= 2;
//		}
//		return dim;
//	}
//
//	@Operation("convert")
//	public class ConvertOperation extends ModuleOperation {
//
//		private Collection<TextureModularFile> textures = new ArrayList<>();
//		private Collection<Element> elements = new ArrayList<>();
//		private Collection<Animation> animations = new ArrayList<>();
//
//		@ModularParameter
//		private Collection<WildcardPath> SapphireAnimationFiles = Collections.emptyList();
//
//		@ModularData
//		private Collection<String> ApplicationAssets = new ArrayList<>();
//
//		private TextureModularFile createNewTexture() {
//			int rem = elements.size();
//			int dim = calcDim(rem);
//			TextureModularFile result = new TextureModularFile("_sapphire_anim_texture_" + textures.size() + ".png",
//					dim);
//			return result;
//		}
//
//		@Override
//		public void run() throws Exception {
//			ModularDirectory convertdir = getContext().getWorkingModularDirectory()
//					.getDirectoryCreate("converted_sapphire_tex", ModularFile.FLAG_SHADOWFILE);
//
//			for (ModularFile file : WildcardPath.getFiles(getContext(), SapphireAnimationFiles)) {
//				parseTexturesFile(file);
//			}
//			TextureModularFile tex = null;
//			for (Animation anim : animations) {
//				for (Element elem : anim.elements) {
//					if (tex == null || !tex.canAdd()) {
//						tex = createNewTexture();
//						textures.add(tex);
//					}
//					elem.resultTexture = tex;
//					tex.add(elem);
//					elements.remove(elem);
//				}
//			}
//			for (TextureModularFile texture : textures) {
//				convertdir.add(texture, ModularFile.FLAG_SHADOWFILE);
//				ApplicationAssets.add(texture.getModularPath().toString());
//			}
//			for (Animation anim : animations) {
//				ModularFile xmlfile = new ModularFile(anim.originalFile.getName().substring(0,
//						anim.originalFile.getName().length() - ".sapp.png".length()) + ".xml") {
//					@Override
//					public FileContentDescriptor getContentDescriptor() {
//						return anim.originalFile.getContentDescriptor();
//					}
//
//					@Override
//					public void writeToStream(OutputStream os) throws IOException {
//						PrintStream ps = new PrintStream(os);
//						ps.println("<?xml version=\"1.0\"?>");
//						ps.print("<FrameAnimation>");
//						for (Element elem : anim.elements) {
//							ps.print("<FrameAnimation.FrameAnimationElement");
//							String pathtopng = getContext().getWorkingDirectory()
//									.relativize(elem.resultTexture.getModularPath()).toString();
//							ps.print(" texture=\"@res/" + pathtopng.substring(0, pathtopng.length() - 4) + "\"");
//							ps.print(" x=\"" + elem.resultPos.x + "\"");
//							ps.print(" y=\"" + elem.resultPos.y + "\"");
//
//							ps.print(" w=\"" + elem.resultPos.width + "\"");
//							ps.print(" h=\"" + elem.resultPos.height + "\"");
//							ps.println("/>");
//						}
//						ps.println("</FrameAnimation>");
//						ps.flush();
//					}
//				};
//				anim.originalFile.getParent().add(xmlfile, ModularFile.FLAG_SHADOWFILE);
//				ApplicationAssets.add(xmlfile.getModularPath().toString());
//			}
//		}
//
//		private void parseTexturesFile(ModularFile file) throws IOException {
//			BufferedImage img;
//			try (InputStream is = file.openInputStream()) {
//				img = ImageIO.read(is);
//			}
//			if (img.getWidth() % TILE_DIMENSION == 0 && img.getHeight() % TILE_DIMENSION == 0) {
//				file.getParent().hide(file);
//
//				Animation anim = new Animation(file);
//				animations.add(anim);
//
//				int w = img.getWidth() / TILE_DIMENSION;
//				int h = img.getHeight() / TILE_DIMENSION;
//				// TODO paddingban 0 alpha de szin az megegyezik szelevel
//				for (int i = 0; i < w; i++) {
//					for (int j = 0; j < h; j++) {
//						Element elem = new Element(img,
//								new Rectangle(i * TILE_DIMENSION, 0, TILE_DIMENSION, TILE_DIMENSION), anim);
//						elements.add(elem);
//					}
//				}
//			}
//		}
//	}
//}
