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
package bence.sipka.compiler.audio;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Objects;
import java.util.TreeMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.enums.EnumType;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class AudioChooserTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.audio.choose";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("audio");
	public static final BundleResourceSupplier descriptor_xaudio2_sdk_pre2_8 = BundleContentAccess
			.getBundleResourceSupplier("audio/xaudio2_sdk_pre2_8");

	public static final String TYPE_NAME_AUDIO_ENUM = "AudioConfig";

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, TypeDeclaration> typeDeclarations;
		private SakerPath sourceDirectory;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, TypeDeclaration> typeDeclarations, SakerPath genDirectory) {
			this.sourceDirectory = genDirectory;
			this.typeDeclarations = ImmutableUtils.makeImmutableNavigableMap(typeDeclarations);
		}

		public Map<String, TypeDeclaration> getTypeDeclarations() {
			return typeDeclarations;
		}

		public SakerPath getSourceDirectory() {
			return sourceDirectory;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(sourceDirectory);
			SerialUtils.writeExternalMap(out, typeDeclarations);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			sourceDirectory = SerialUtils.readExternalObject(in);
			typeDeclarations = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((sourceDirectory == null) ? 0 : sourceDirectory.hashCode());
			result = prime * result + ((typeDeclarations == null) ? 0 : typeDeclarations.hashCode());
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
			if (sourceDirectory == null) {
				if (other.sourceDirectory != null)
					return false;
			} else if (!sourceDirectory.equals(other.sourceDirectory))
				return false;
			if (typeDeclarations == null) {
				if (other.typeDeclarations != null)
					return false;
			} else if (!typeDeclarations.equals(other.typeDeclarations))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = { "", "AudioAPI" }, required = true)
			public List<String> audioApiOption;

			@SakerInput(value = "PlatformName")
			public String platform;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);

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
				gendir.add(new TemplatedSourceModularFile("audiomanagers.h",
						new TemplatedSource(descriptor::getInputStream, "gen/audiomanagers.h")));
				TemplatedSourceModularFile audiomanagerscpp = new TemplatedSourceModularFile("audiomanagers.cpp",
						new TemplatedSource(descriptor::getInputStream, "gen/audiomanagers.cpp")).setThis(audioenum);
				gendir.add(audiomanagerscpp);

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(buildDirectory.getFilesRecursiveByPath(
								buildDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				buildDirectory.synchronize();

				Output result = new Output(typeDeclarations, buildDirectory.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
	}
}
