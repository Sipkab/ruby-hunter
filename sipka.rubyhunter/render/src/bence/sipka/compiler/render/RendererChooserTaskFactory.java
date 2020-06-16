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
package bence.sipka.compiler.render;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.Collections;
import java.util.Locale;
import java.util.Map;
import java.util.NavigableMap;
import java.util.TreeMap;
import java.util.stream.Collectors;

import bence.sipka.compiler.source.SourceModularFile;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.enums.EnumType;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
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
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class RendererChooserTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.renderer.choose";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("render");

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
			@SakerInput(value = { "RenderAPI" }, required = true)
			public Collection<String> renderapi = Collections.emptyList();
			@SakerInput(value = { "PlatformName" })
			public String platformNameOption;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);

				NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
				EnumType renderenum = new EnumType("RenderConfig");

				renderapi = ImmutableUtils.makeImmutableNavigableSet(renderapi);

				SakerDirectory sourcedir = buildDirectory;
				if (!ObjectUtils.isNullOrEmpty(platformNameOption)) {
					sourcedir = sourcedir.getDirectoryCreate(platformNameOption);
				}
				sourcedir.clear();

				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");

				int idx = 0;
				for (String api : renderapi) {
					String enumname;
					switch (api.toLowerCase(Locale.ENGLISH)) {
						case "opengles20": {
							enumname = "OpenGlEs20";
							if ("ios".equalsIgnoreCase(platformNameOption)) {
								sourcedir.getDirectoryCreate("KHR")
										.add(new TemplatedSourceModularFile("khrplatform.h", new TemplatedSource(
												descriptor::getInputStream, "opengl_registry/ios/KHR/khrplatform.h")));
							}
							break;
						}
						case "opengl30": {
							enumname = "OpenGl30";
							break;
						}
						case "directx11": {
							enumname = "DirectX11";
							break;
						}
						default: {
							throw new UnsupportedOperationException(api);
						}
					}
					renderenum.add(enumname, idx++);
				}

				typeDeclarations.put(renderenum.getName(), renderenum);

				gendir.add(new TemplatedSourceModularFile("renderers.h",
						new TemplatedSource(descriptor::getInputStream, "gen/renderers.h").setThis(renderapi)));
				SourceModularFile rendererscpp = new TemplatedSourceModularFile("renderers.cpp",
						new TemplatedSource(descriptor::getInputStream, "gen/renderers.cpp")
								.setThis(renderenum.getValues().entrySet().stream()
										.sorted((a, b) -> Integer.compare(a.getValue(), b.getValue()))
										.map(e -> e.getKey()).collect(Collectors.toList())));
				gendir.add(rendererscpp);

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(sourcedir.getFilesRecursiveByPath(sourcedir.getSakerPath(),
								DirectoryVisitPredicate.everything())));
				sourcedir.synchronize();

				Output result = new Output(typeDeclarations, sourcedir.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
	}
}
