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
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import javax.imageio.ImageIO;

import bence.sipka.user.sapphire.SapphireTextureConverterTaskFactory.Output;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class SapphireTextureConverterWorkerTaskFactory
		implements TaskFactory<SapphireTextureConverterTaskFactory.Output>,
		Task<SapphireTextureConverterTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final int TILE_DIMENSION = 60;
	public static final int TILE_PADDING = 2;

	private Set<FileCollectionStrategy> inputCollectionStrategies;

	/**
	 * For {@link Externalizable}.
	 */
	public SapphireTextureConverterWorkerTaskFactory() {
	}

	public SapphireTextureConverterWorkerTaskFactory(Set<FileCollectionStrategy> collectionstrategies) {
		this.inputCollectionStrategies = collectionstrategies;
	}

	@Override
	public Task<? extends Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(SapphireTextureConverterTaskFactory.TASK_NAME);

		Collection<TextureSakerFile> textures = new ArrayList<>();
		Collection<Element> elements = new ArrayList<>();
		Collection<Animation> animations = new ArrayList<>();

		SakerDirectory genDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(SapphireTextureConverterTaskFactory.TASK_NAME);
		genDirectory.clear();

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, inputCollectionStrategies);

		NavigableMap<String, SakerPath> assets = new TreeMap<>();

		for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
			parseTexturesFile(entry.getValue(), animations, elements);
		}

		TextureSakerFile tex = null;
		for (Animation anim : animations) {
			for (Element elem : anim.elements) {
				if (tex == null || !tex.canAdd()) {
					tex = createNewTexture(elements, textures);
					textures.add(tex);
				}
				elem.resultTexture = tex;
				tex.add(elem);
				elements.remove(elem);
			}
		}
		SakerPath taskworkingdirpath = taskcontext.getTaskWorkingDirectoryPath();
		for (TextureSakerFile texture : textures) {
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

						String pathtopng = taskworkingdirpath.relativize(elem.resultTexture.getSakerPath()).toString();
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
//			assets.put(taskworkingdirpath.relativize(xmlpath).toString(), xmlpath);
		}

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(genDirectory
				.getFilesRecursiveByPath(genDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		genDirectory.synchronize();

		Output result = new Output(assets, xmls);
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	private static TextureSakerFile createNewTexture(Collection<Element> elements, Collection<TextureSakerFile> textures) {
		int rem = elements.size();
		int dim = calcDim(rem);
		TextureSakerFile result = new TextureSakerFile("_sapphire_anim_texture_" + textures.size() + ".png", dim);
		return result;
	}

	private static void parseTexturesFile(SakerFile file, Collection<Animation> animations, Collection<Element> elements)
			throws IOException {
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

	private static int calcDim(int rem) {
		int dim = 16;
		while (rem < dim * dim / 4) {
			dim /= 2;
		}
		return dim;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, inputCollectionStrategies);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		inputCollectionStrategies = SerialUtils.readExternalImmutableLinkedHashSet(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((inputCollectionStrategies == null) ? 0 : inputCollectionStrategies.hashCode());
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
		SapphireTextureConverterWorkerTaskFactory other = (SapphireTextureConverterWorkerTaskFactory) obj;
		if (inputCollectionStrategies == null) {
			if (other.inputCollectionStrategies != null)
				return false;
		} else if (!inputCollectionStrategies.equals(other.inputCollectionStrategies))
			return false;
		return true;
	}

}
