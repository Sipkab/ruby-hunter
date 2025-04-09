package bence.sipka.compiler.audio;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.NavigableMap;
import java.util.Objects;
import java.util.TreeMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.enums.EnumType;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class AudioChooserWorkerTaskFactory
		implements TaskFactory<AudioChooserTaskFactory.Output>, Task<AudioChooserTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("audio");
	public static final BundleResourceSupplier descriptor_xaudio2_sdk_pre2_8 = BundleContentAccess
			.getBundleResourceSupplier("audio/xaudio2_sdk_pre2_8");

	public static final String TYPE_NAME_AUDIO_ENUM = "AudioConfig";

	private String platform;
	private List<String> audioApiOption;

	/**
	 * For {@link Externalizable}.
	 */
	public AudioChooserWorkerTaskFactory() {
	}

	public AudioChooserWorkerTaskFactory(String platform, List<String> audioApiOption) {
		this.platform = platform;
		this.audioApiOption = audioApiOption;
	}

	@Override
	public Task<? extends AudioChooserTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public AudioChooserTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
			LinkedHashMap<Object, Object> vals = new LinkedHashMap<>();
			vals.put("Platform", platform);
			vals.put("Audio API", audioApiOption);
			BuildTrace.setValues(vals, BuildTrace.VALUE_CATEGORY_TASK);
		}
		taskcontext.setStandardOutDisplayIdentifier(AudioChooserTaskFactory.TASK_NAME);
		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(AudioChooserTaskFactory.TASK_NAME);

		NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
		EnumType audioenum = new EnumType(TYPE_NAME_AUDIO_ENUM);
		typeDeclarations.put(audioenum.getName(), audioenum);

		for (String audioapi : audioApiOption) {
			switch (audioapi.toLowerCase(Locale.ENGLISH)) {
				case "xaudio2": {
					audioenum.add("XAudio2", audioenum.getCount());
					SakerDirectory xaudiodir = buildDirectory.getDirectoryCreate("xaudio2_sdk_pre2_8");
					for (String en : descriptor_xaudio2_sdk_pre2_8.getEntries()) {
						xaudiodir.add(new ByteArraySakerFile(en, descriptor_xaudio2_sdk_pre2_8.getBytes(en)));
					}
					break;
				}
				case "opensles10_android": {
					audioenum.add("OpenSLES10Android", audioenum.getCount());
					break;
				}
				case "openal": {
					SakerDirectory openalgluedir = buildDirectory.getDirectoryCreate("openalglue");
					audioenum.add("OpenAL10", audioenum.getCount());

					Objects.requireNonNull(platform, "platform name");
					String pldir;
					switch (platform.toLowerCase(Locale.ENGLISH)) {
						case "ios":
						case "macosx":
						case "macos": {
							pldir = "apple";
							break;
						}
						case "linux": {
							pldir = "linux";
							break;
						}
						default: {
							throw new UnsupportedOperationException(platform);
						}
					}

					openalgluedir.add(new ByteArraySakerFile("openalglue.h",
							descriptor.getBytes("openalglue/" + pldir + "/openalglue/openalglue.h")));
					break;
				}
				default: {
					throw new UnsupportedOperationException(audioapi);
				}
			}
		}

		SakerDirectory gendir = buildDirectory.getDirectoryCreate("gen");
		gendir.add(new TemplatedSourceSakerFile("audiomanagers.h",
				new TemplatedSource(descriptor::getInputStream, "gen/audiomanagers.h")));
		TemplatedSourceSakerFile audiomanagerscpp = new TemplatedSourceSakerFile("audiomanagers.cpp",
				new TemplatedSource(descriptor::getInputStream, "gen/audiomanagers.cpp")).setThis(audioenum);
		gendir.add(audiomanagerscpp);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(buildDirectory
				.getFilesRecursiveByPath(buildDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		buildDirectory.synchronize();

		AudioChooserTaskFactory.Output result = new AudioChooserTaskFactory.Output(typeDeclarations,
				buildDirectory.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(platform);
		SerialUtils.writeExternalCollection(out, audioApiOption);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		platform = SerialUtils.readExternalObject(in);
		audioApiOption = SerialUtils.readExternalImmutableList(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((audioApiOption == null) ? 0 : audioApiOption.hashCode());
		result = prime * result + ((platform == null) ? 0 : platform.hashCode());
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
		AudioChooserWorkerTaskFactory other = (AudioChooserWorkerTaskFactory) obj;
		if (audioApiOption == null) {
			if (other.audioApiOption != null)
				return false;
		} else if (!audioApiOption.equals(other.audioApiOption))
			return false;
		if (platform == null) {
			if (other.platform != null)
				return false;
		} else if (!platform.equals(other.platform))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "AudioChooserWorkerTaskFactory["
				+ (audioApiOption != null ? "audioApiOption=" + audioApiOption + ", " : "")
				+ (platform != null ? "platform=" + platform : "") + "]";
	}

}
