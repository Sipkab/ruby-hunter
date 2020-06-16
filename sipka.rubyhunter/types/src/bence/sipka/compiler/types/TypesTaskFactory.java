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
package bence.sipka.compiler.types;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import org.w3c.dom.Document;

import bence.sipka.compiler.resource.ResourceCompilerTaskFactory;
import bence.sipka.compiler.source.SourceModularFile;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.compiler.types.builtin.BooleanType;
import bence.sipka.compiler.types.builtin.ByteType;
import bence.sipka.compiler.types.builtin.DoubleType;
import bence.sipka.compiler.types.builtin.FloatType;
import bence.sipka.compiler.types.builtin.IntegerType;
import bence.sipka.compiler.types.builtin.LongType;
import bence.sipka.compiler.types.builtin.ShortType;
import bence.sipka.compiler.types.builtin.StringType;
import bence.sipka.compiler.types.enums.EnumerationsParser;
import bence.sipka.compiler.types.flags.FlagsParser;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
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
import saker.build.task.utils.annot.DataContext;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class TypesTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String CONFIG_NAMESPACE_URI = "bence.sipka.types.config";

	public static final String TASK_NAME = "sipka.rh.types.parse";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("types");

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
			@SakerInput(value = { "", "Input" }, required = true)
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			@SakerInput(value = "ExternalTypes")
			public List<Map<String, TypeDeclaration>> externalTypesOption = Collections.emptyList();

			@DataContext
			public TypesSourcesData typesData = new TypesSourcesData();

			private NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				Collection<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				externalTypesOption = ObjectUtils.cloneArrayList(externalTypesOption, ObjectUtils::cloneTreeMap);
				typeDeclarations = ObjectUtils.cloneTreeMap(typeDeclarations);

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, collectionstrategies);

				putTypeDeclaration(ByteType.INSTANCE);
				putTypeDeclaration(ShortType.INSTANCE);
				putTypeDeclaration(IntegerType.INSTANCE);
				putTypeDeclaration(LongType.INSTANCE);
				putTypeDeclaration(BooleanType.INSTANCE);
				putTypeDeclaration(StringType.INSTANCE);
				putTypeDeclaration(FloatType.INSTANCE);
				putTypeDeclaration(DoubleType.INSTANCE);

				List<Document> xmldocuments = new ArrayList<>();

				for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
					Document doc = ResourceCompilerTaskFactory.getDocument(entry.getValue());
					xmldocuments.add(doc);
				}

				new EnumerationsParser(typeDeclarations, xmldocuments).parse();
				new FlagsParser(typeDeclarations, xmldocuments).parse();

				for (Map<String, TypeDeclaration> typemap : externalTypesOption) {
					for (Entry<String, TypeDeclaration> entry : typemap.entrySet()) {
						TypeDeclaration prev = this.typeDeclarations.put(entry.getKey(), entry.getValue());
						if (prev != null) {
							throw new IllegalArgumentException("Duplicate type declarations for name: " + entry.getKey()
									+ " with " + prev + " " + entry.getValue());
						}

					}
				}

				typesData.typeDeclarations = this.typeDeclarations;
				if (typesData.nullptr_type == null) {
					String platformval = typesData.PlatformName;
					if (platformval != null) {
						switch (platformval) {
							case "ios":
							case "android": {
								typesData.nullptr_type = "decltype(nullptr)";
								break;
							}
							case "winstore":
							case "win32": {
								typesData.nullptr_type = " std::nullptr_t";
								break;
							}
							default: {
								typesData.nullptr_type = "decltype(nullptr)";
								break;
							}
						}
					}
				}

				SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				outputDirectory.clear();

				SakerDirectory sourcesdir = outputDirectory.getDirectoryCreate("sources");
				SakerDirectory genDirectory = sourcesdir.getDirectoryCreate("gen");
				SakerDirectory fwdDirectory = genDirectory.getDirectoryCreate("fwd");

				genDirectory.add(new TemplatedSourceModularFile("types.h",
						new TemplatedSource(descriptor::getInputStream, "gen/types.h").setThis(typesData)));
				fwdDirectory.add(new TemplatedSourceModularFile("types.h",
						new TemplatedSource(descriptor::getInputStream, "gen/fwd/types.h").setThis(typesData)));
				genDirectory.add(new TemplatedSourceModularFile("serialize.h",
						new TemplatedSource(descriptor::getInputStream, "gen/serialize.h").setThis(typesData)));
				SourceModularFile cppfile = new TemplatedSourceModularFile("deserializetypes.cpp",
						new TemplatedSource(descriptor::getInputStream, "gen/deserializetypes.cpp").setThis(typesData));
				genDirectory.add(cppfile);

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(outputDirectory.getFilesRecursiveByPath(
								outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				outputDirectory.synchronize();

				Output result = new Output(typeDeclarations, sourcesdir.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}

			private void putTypeDeclaration(TypeDeclaration type) {
				typeDeclarations.put(type.getName(), type);
			}
		};
	}

	public static class TypesSourcesData implements Externalizable {
		private static final long serialVersionUID = 1L;

		@SakerInput(value = "nullptr_type")
		public String nullptr_type = "decltype(nullptr)";

		@SakerInput(value = "PlatformName")
		public String PlatformName;

		public Map<String, TypeDeclaration> typeDeclarations;

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(nullptr_type);
			out.writeObject(PlatformName);
			SerialUtils.writeExternalMap(out, typeDeclarations);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			nullptr_type = SerialUtils.readExternalObject(in);
			PlatformName = SerialUtils.readExternalObject(in);
			typeDeclarations = SerialUtils.readExternalImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((PlatformName == null) ? 0 : PlatformName.hashCode());
			result = prime * result + ((nullptr_type == null) ? 0 : nullptr_type.hashCode());
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
			TypesSourcesData other = (TypesSourcesData) obj;
			if (PlatformName == null) {
				if (other.PlatformName != null)
					return false;
			} else if (!PlatformName.equals(other.PlatformName))
				return false;
			if (nullptr_type == null) {
				if (other.nullptr_type != null)
					return false;
			} else if (!nullptr_type.equals(other.nullptr_type))
				return false;
			if (typeDeclarations == null) {
				if (other.typeDeclarations != null)
					return false;
			} else if (!typeDeclarations.equals(other.typeDeclarations))
				return false;
			return true;
		}

	}

}
