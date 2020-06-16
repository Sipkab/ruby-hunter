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
//package bence.sipka.user.font;
//
//import java.awt.Font;
//import java.awt.Graphics2D;
//import java.awt.RenderingHints;
//import java.io.ByteArrayOutputStream;
//import java.io.IOException;
//import java.io.InputStream;
//import java.io.PrintStream;
//import java.io.UncheckedIOException;
//import java.util.ArrayList;
//import java.util.Collection;
//import java.util.Collections;
//import java.util.Iterator;
//import java.util.List;
//
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import bence.sipka.user.sapphire.ImageModularFile;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularParameter;
//import modular.core.file.ByteArrayModularFile;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.file.path.WildcardPath;
//import modular.core.module.Module;
//import modular.core.module.ModuleOperation;
//
//@ModuleURI(FontConverter.MODULE_URL)
//public class FontConverter extends Module {
//	static class FontPackFailedException extends Exception {
//		private static final long serialVersionUID = 1L;
//
//		public FontPackFailedException(String message) {
//			super(message);
//		}
//
//		public FontPackFailedException(String message, Throwable cause) {
//			super(message, cause);
//		}
//
//	}
//
//	public static final String MODULE_URL = "bence.sipka.sapphire.fontconverter";
//
//	static void setupGraphics(Graphics2D g, Font font) {
//		g.setFont(font);
//		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
//		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
//	}
//
//	@Operation("convert")
//	public class ConvertOperation extends ModuleOperation {
//
//		@ModularParameter
//		private boolean Debug = false;
//
//		@ModularParameter
//		private int minCodePoint = Character.MIN_CODE_POINT;
//		@ModularParameter
//		private int maxCodePoint = Character.MAX_CODE_POINT;
//
//		@ModularParameter
//		private Collection<WildcardPath> FontFiles = Collections.emptyList();
//
//		@ModularData
//		private Collection<String> ApplicationAssets = new ArrayList<>();
//
//		@Override
//		public void run() throws Exception {
//			List<ModularFile> files = new ArrayList<>();
//			for (ModularFile file : WildcardPath.getFiles(getContext(), FontFiles)) {
//				files.add(file);
//			}
//
//			for (ModularFile f : files) {
//				Font font;
//				try (InputStream fontis = f.openInputStream()) {
//					font = Font.createFont(Font.TRUETYPE_FONT, fontis);
//				}
//				fontToPng(f, font);
//			}
//		}
//
//		private void fontToPng(ModularFile file, Font font) {
//			FontDescription desc = new FontDescription(font, minCodePoint, maxCodePoint);
//			desc.setDebug(Debug);
//			
//			ModularDirectory parentdir = file.getParent();
//			ImageModularFile imgfile = new ImageModularFile(file.getName() + "_atlas.png", desc::getImage);
//			parentdir.add(imgfile, ModularFile.FLAG_SHADOWFILE);
//
//			ApplicationAssets.add(imgfile.getModularPath().toString());
//
//			ByteArrayModularFile xmlfile = new ByteArrayModularFile(file.getName() + ".xml", () -> {
//				try {
//					desc.initialize();
//				} catch (FontPackFailedException e) {
//					throw new RuntimeException(e);
//				}
//				try (ByteArrayOutputStream baos = new ByteArrayOutputStream();
//						PrintStream ps = new PrintStream(baos)) {
//					String pathtopng = getContext().getWorkingDirectory().relativize(imgfile.getModularPath()).toString();
//					pathtopng = pathtopng.substring(0, pathtopng.length() - 4);
//
//					int imagedimension = desc.getImageDimension();
//
//					ps.println("<?xml version=\"1.0\"?>");
//					ps.print("<Font");
//					ps.print(" size=\"" + desc.getSize() / imagedimension + "\"");
//					ps.print(" ascent=\"" + desc.getAscent() / imagedimension + "\"");
//					ps.print(" descent=\"" + desc.getDescent() / imagedimension + "\"");
//					ps.print(" leading=\"" + desc.getLeading() / imagedimension + "\"");
//					ps.print(" texture=\"@res/" + pathtopng + "\"");
//					ps.println(">");
//
//					for (Iterator<CharRect> it = desc.getCharacters().stream().sorted((a, b) -> Integer.compare(a.codepoint, b.codepoint)).iterator(); it
//							.hasNext();) {
//						CharRect cr = it.next();
//						ps.print("<Font.Char ");
//						ps.print(" id=\"" + cr.codepoint + "\"");
//						ps.print(" xoffset=\"" + cr.xoffset / imagedimension + "\"");
//						ps.print(" x=\"" + (cr.resultPos.getX() - cr.xoffset) / imagedimension + "\"");
//						ps.print(" y=\"" + (cr.resultPos.getY() + cr.charRect.getY()) / imagedimension + "\"");
//
//						ps.print(" w=\"" + (cr.charRect.getWidth()) / imagedimension + "\"");
//						ps.print(" h=\"" + (cr.charRect.getHeight()) / imagedimension + "\"");
//
//						ps.print(" xadvance=\"" + cr.advance / imagedimension + "\"");
//						ps.print(" baseline=\"" + (-cr.charRect.getY()) / imagedimension + "\"");
//
//						ps.println("/>");
//					}
//					ps.println("</Font>");
//					return baos.toByteArray();
//				} catch (IOException e) {
//					throw new UncheckedIOException(e);
//				}
//			});
//			parentdir.add(xmlfile, ModularFile.FLAG_SHADOWFILE);
//			ApplicationAssets.add(xmlfile.getModularPath().toString());
//
//			parentdir.hide(file);
//
//		}
//	}
//}
