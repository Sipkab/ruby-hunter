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
package bence.sipka.user.sapphire;

import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.TreeMap;
import java.util.TreeSet;

import javax.imageio.ImageIO;

import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.path.SakerPath;
import saker.build.file.path.WildcardPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class SapphireTextureConverterTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final int TILE_DIMENSION = 60;
	public static final int TILE_PADDING = 2;

	public static final String TASK_NAME = "sipka.rh.texture.convert";

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, SakerPath> assets;
		private NavigableSet<SakerPath> xmls;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, SakerPath> assets, NavigableSet<SakerPath> xmls) {
			this.assets = ImmutableUtils.makeImmutableNavigableMap(assets);
			this.xmls = ImmutableUtils.makeImmutableNavigableSet(xmls);
		}

		public NavigableMap<String, SakerPath> getAssets() {
			return assets;
		}

		public NavigableSet<SakerPath> getXmls() {
			return xmls;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			SerialUtils.writeExternalMap(out, assets);
			SerialUtils.writeExternalCollection(out, xmls);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			assets = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			xmls = SerialUtils.readExternalSortedImmutableNavigableSet(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((assets == null) ? 0 : assets.hashCode());
			result = prime * result + ((xmls == null) ? 0 : xmls.hashCode());
			return result;
		}

		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;
			if (obj == null)
				return false;
			if (getClass() != obj.getClass())
				return false;
			Output other = (Output) obj;
			if (assets == null) {
				if (other.assets != null)
					return false;
			} else if (!assets.equals(other.assets))
				return false;
			if (xmls == null) {
				if (other.xmls != null)
					return false;
			} else if (!xmls.equals(other.xmls))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = { "", "Input" }, required = true)
			private Collection<WildcardPath> inputOption = Collections.emptyList();

			private Collection<TextureModularFile> textures = new ArrayList<>();
			private Collection<Element> elements = new ArrayList<>();
			private Collection<Animation> animations = new ArrayList<>();

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				Collection<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				SakerDirectory genDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				genDirectory.clear();

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, collectionstrategies);

				NavigableMap<String, SakerPath> assets = new TreeMap<>();

				for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
					parseTexturesFile(entry.getValue());
				}

				TextureModularFile tex = null;
				for (Animation anim : animations) {
					for (Element elem : anim.elements) {
						if (tex == null || !tex.canAdd()) {
							tex = createNewTexture();
							textures.add(tex);
						}
						elem.resultTexture = tex;
						tex.add(elem);
						elements.remove(elem);
					}
				}
				SakerPath taskworkingdirpath = taskcontext.getTaskWorkingDirectoryPath();
				for (TextureModularFile texture : textures) {
					genDirectory.add(texture);
					SakerPath path = texture.getSakerPath();
					assets.put(taskworkingdirpath.relativize(path).toString(), path);
				}
				NavigableSet<SakerPath> xmls = new TreeSet<>();
				for (Animation anim : animations) {
					SakerFile xmlfile = new SakerFileBase(anim.originalFile.getName().substring(0,
							anim.originalFile.getName().length() - ".sapp.png".length()) + ".xml") {
						@Override
						public ContentDescriptor getContentDescriptor() {
							return anim.originalFile.getContentDescriptor();
						}

						@Override
						public void writeToStreamImpl(OutputStream os) throws IOException {
							PrintStream ps = new PrintStream(os);
							ps.println("<?xml version=\"1.0\"?>");
							ps.print("<FrameAnimation>");
							for (Element elem : anim.elements) {
								ps.print("<FrameAnimation.FrameAnimationElement");

								String pathtopng = taskworkingdirpath.relativize(elem.resultTexture.getSakerPath())
										.toString();
								ps.print(" texture=\"@res/" + pathtopng.substring(0, pathtopng.length() - 4) + "\"");
								ps.print(" x=\"" + elem.resultPos.x + "\"");
								ps.print(" y=\"" + elem.resultPos.y + "\"");

								ps.print(" w=\"" + elem.resultPos.width + "\"");
								ps.print(" h=\"" + elem.resultPos.height + "\"");
								ps.println("/>");
							}
							ps.println("</FrameAnimation>");
							ps.flush();
						}
					};
					genDirectory.add(xmlfile);
					SakerPath xmlpath = xmlfile.getSakerPath();
					xmls.add(xmlpath);
//					assets.put(taskworkingdirpath.relativize(xmlpath).toString(), xmlpath);
				}

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(genDirectory.getFilesRecursiveByPath(
								genDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				genDirectory.synchronize();

				Output result = new Output(assets, xmls);
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}

			private TextureModularFile createNewTexture() {
				int rem = elements.size();
				int dim = calcDim(rem);
				TextureModularFile result = new TextureModularFile("_sapphire_anim_texture_" + textures.size() + ".png",
						dim);
				return result;
			}

			private void parseTexturesFile(SakerFile file) throws IOException {
				BufferedImage img;
				try (InputStream is = file.openInputStream()) {
					img = ImageIO.read(is);
				}
				if (img.getWidth() % TILE_DIMENSION == 0 && img.getHeight() % TILE_DIMENSION == 0) {
					Animation anim = new Animation(file);
					animations.add(anim);

					int w = img.getWidth() / TILE_DIMENSION;
					int h = img.getHeight() / TILE_DIMENSION;
					// TODO paddingban 0 alpha de szin az megegyezik szelevel
					for (int i = 0; i < w; i++) {
						for (int j = 0; j < h; j++) {
							Element elem = new Element(img,
									new Rectangle(i * TILE_DIMENSION, 0, TILE_DIMENSION, TILE_DIMENSION), anim);
							elements.add(elem);
						}
					}
				}
			}
		};
	}

	private static int calcDim(int rem) {
		int dim = 16;
		while (rem < dim * dim / 4) {
			dim /= 2;
		}
		return dim;
	}
}
