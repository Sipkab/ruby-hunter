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
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import bence.sipka.user.sapphire.ImageSakerFile;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;
import saker.build.trace.BuildTrace;

public class FontConverterWorkerTaskFactory
		implements TaskFactory<FontConverterTaskFactory.Output>, Task<FontConverterTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	private Set<FileCollectionStrategy> collectionstrategies;
	private boolean debugOption;

	private int minCodePoint = Character.MIN_CODE_POINT;
	private int maxCodePoint = Character.MAX_CODE_POINT;

	/**
	 * For {@link Externalizable}.
	 */
	public FontConverterWorkerTaskFactory() {
	}

	public FontConverterWorkerTaskFactory(Set<FileCollectionStrategy> collectionstrategies, boolean debugOption,
			int minCodePoint, int maxCodePoint) {
		this.collectionstrategies = collectionstrategies;
		this.debugOption = debugOption;
		this.minCodePoint = minCodePoint;
		this.maxCodePoint = maxCodePoint;
	}

	@Override
	public Task<? extends FontConverterTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public FontConverterTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(FontConverterTaskFactory.TASK_NAME);

		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(FontConverterTaskFactory.TASK_NAME);
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
			fontToPng(taskcontext, entry.getKey(), font, taskcontext.getTaskWorkingDirectoryPath(), buildDirectory,
					assets, xmls);
		}

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(buildDirectory
				.getFilesRecursiveByPath(buildDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		buildDirectory.synchronize();

		FontConverterTaskFactory.Output result = new FontConverterTaskFactory.Output(assets, xmls);
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

		ImageSakerFile imgfile = new ImageSakerFile(imgfilename, desc.getImage());
		outputdir.add(imgfile);

		ByteArraySakerFile xmlfile = new ByteArraySakerFile(filepath.getFileName() + ".xml", bytes);
		outputdir.add(xmlfile);

		assets.put(pathstrtopng, pathtopng);
		xmls.add(xmlfile.getSakerPath());
//		assets.put(workingdir.relativize(filepath).toString() + ".xml", xmlfile.getSakerPath());
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

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, collectionstrategies);
		out.writeBoolean(debugOption);
		out.writeInt(minCodePoint);
		out.writeInt(maxCodePoint);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		collectionstrategies = SerialUtils.readExternalImmutableLinkedHashSet(in);
		debugOption = in.readBoolean();
		minCodePoint = in.readInt();
		maxCodePoint = in.readInt();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((collectionstrategies == null) ? 0 : collectionstrategies.hashCode());
		result = prime * result + (debugOption ? 1231 : 1237);
		result = prime * result + maxCodePoint;
		result = prime * result + minCodePoint;
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
		FontConverterWorkerTaskFactory other = (FontConverterWorkerTaskFactory) obj;
		if (collectionstrategies == null) {
			if (other.collectionstrategies != null)
				return false;
		} else if (!collectionstrategies.equals(other.collectionstrategies))
			return false;
		if (debugOption != other.debugOption)
			return false;
		if (maxCodePoint != other.maxCodePoint)
			return false;
		if (minCodePoint != other.minCodePoint)
			return false;
		return true;
	}

}
