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
package bence.sipka.user.font;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.PrintStream;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.TreeMap;
import java.util.TreeSet;

import bence.sipka.user.sapphire.ImageModularFile;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
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
import saker.build.thirdparty.saker.util.function.LazySupplier;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;
import saker.nest.utils.FrontendTaskFactory;

public class FontConverterTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.font.convert";

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
			public Collection<WildcardPath> inputOption;

			@SakerInput("Debug")
			public boolean debugOption;

			@SakerInput("MinCodePoint")
			private int minCodePoint = Character.MIN_CODE_POINT;
			@SakerInput("MaxCodePoint")
			private int maxCodePoint = Character.MAX_CODE_POINT;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				Collection<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				buildDirectory.clear();

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, collectionstrategies);

				NavigableMap<String, SakerPath> assets = new TreeMap<>();
				NavigableSet<SakerPath> xmls = new TreeSet<>();

				for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
					SakerFile f = entry.getValue();
					Font font;
					try (InputStream fontis = f.openInputStream()) {
						font = Font.createFont(Font.TRUETYPE_FONT, fontis);
					}
					fontToPng(taskcontext, entry.getKey(), font, taskcontext.getTaskWorkingDirectoryPath(),
							buildDirectory, assets, xmls);
				}

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(buildDirectory.getFilesRecursiveByPath(
								buildDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				buildDirectory.synchronize();

				Output result = new Output(assets, xmls);
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}

			private void fontToPng(TaskContext taskcontext, SakerPath filepath, Font font, SakerPath workingdir,
					SakerDirectory outputdir, NavigableMap<String, SakerPath> assets, NavigableSet<SakerPath> xmls) {
				FontDescription desc = new FontDescription(font, minCodePoint, maxCodePoint);
				desc.setDebug(debugOption);

				outputdir = taskcontext.getTaskUtilities().resolveDirectoryAtRelativePathCreate(outputdir,
						workingdir.relativize(filepath).getParent());
				String imgfilename = filepath.getFileName() + "_atlas.png";
				try {
					desc.initialize();
				} catch (FontPackFailedException e) {
					throw new RuntimeException(e);
				}

				SakerPath pathtopng = outputdir.getSakerPath().resolve(imgfilename);
				SakerPath relpathtopng = workingdir.relativize(pathtopng);
				String pathstrtopng = relpathtopng.toString();
				String respathtopng = pathstrtopng.substring(0, pathstrtopng.length() - 4);

				ByteArrayRegion bytes;
				try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream();
						PrintStream ps = new PrintStream(baos)) {

					int imagedimension = desc.getImageDimension();

					ps.println("<?xml version=\"1.0\"?>");
					ps.print("<Font");
					ps.print(" size=\"" + desc.getSize() / imagedimension + "\"");
					ps.print(" ascent=\"" + desc.getAscent() / imagedimension + "\"");
					ps.print(" descent=\"" + desc.getDescent() / imagedimension + "\"");
					ps.print(" leading=\"" + desc.getLeading() / imagedimension + "\"");
					ps.print(" texture=\"@res/" + respathtopng + "\"");
					ps.println(">");

					for (Iterator<CharRect> it = desc.getCharacters().stream()
							.sorted((a, b) -> Integer.compare(a.codepoint, b.codepoint)).iterator(); it.hasNext();) {
						CharRect cr = it.next();
						ps.print("<Font.Char ");
						ps.print(" id=\"" + cr.codepoint + "\"");
						ps.print(" xoffset=\"" + cr.xoffset / imagedimension + "\"");
						ps.print(" x=\"" + (cr.resultPos.getX() - cr.xoffset) / imagedimension + "\"");
						ps.print(" y=\"" + (cr.resultPos.getY() + cr.charRect.getY()) / imagedimension + "\"");

						ps.print(" w=\"" + (cr.charRect.getWidth()) / imagedimension + "\"");
						ps.print(" h=\"" + (cr.charRect.getHeight()) / imagedimension + "\"");

						ps.print(" xadvance=\"" + cr.advance / imagedimension + "\"");
						ps.print(" baseline=\"" + (-cr.charRect.getY()) / imagedimension + "\"");

						ps.println("/>");
					}
					ps.println("</Font>");
					bytes = baos.toByteArrayRegion();
				}

				ImageModularFile imgfile = new ImageModularFile(imgfilename, LazySupplier.of(desc::getImage));
				outputdir.add(imgfile);

				ByteArraySakerFile xmlfile = new ByteArraySakerFile(filepath.getFileName() + ".xml", bytes);
				outputdir.add(xmlfile);

				assets.put(pathstrtopng, pathtopng);
				xmls.add(xmlfile.getSakerPath());
//				assets.put(workingdir.relativize(filepath).toString() + ".xml", xmlfile.getSakerPath());
			}
		};
	}

	static class FontPackFailedException extends Exception {
		private static final long serialVersionUID = 1L;

		public FontPackFailedException(String message) {
			super(message);
		}

		public FontPackFailedException(String message, Throwable cause) {
			super(message, cause);
		}

	}

	static void setupGraphics(Graphics2D g, Font font) {
		g.setFont(font);
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
	}
}
