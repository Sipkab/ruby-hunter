package bence.sipka.compiler.types;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import org.w3c.dom.Document;

import bence.sipka.compiler.resource.ResourceCompilerTaskFactory;
import bence.sipka.compiler.source.SourceSakerFile;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.TypesTaskFactory.TypesSourcesData;
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
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class TypesWorkerTaskFactory
		implements TaskFactory<TypesTaskFactory.Output>, Task<TypesTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("types");

	private Set<FileCollectionStrategy> inputOption;
	private List<Map<String, TypeDeclaration>> externalTypesOption;
	private String nullptr_type;
	private String platformName;

	/**
	 * For {@link Externalizable}.
	 */
	public TypesWorkerTaskFactory() {
	}

	public TypesWorkerTaskFactory(Set<FileCollectionStrategy> inputOption,
			List<Map<String, TypeDeclaration>> externalTypesOption, String nullptr_type, String platformName) {
		this.inputOption = inputOption;
		this.externalTypesOption = externalTypesOption;
		this.nullptr_type = nullptr_type;
		this.platformName = platformName;
	}

	@Override
	public Task<? extends TypesTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public TypesTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(TypesTaskFactory.TASK_NAME);

		NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();

		typeDeclarations = ObjectUtils.cloneTreeMap(typeDeclarations);

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, inputOption);

		putTypeDeclaration(typeDeclarations, ByteType.INSTANCE);
		putTypeDeclaration(typeDeclarations, ShortType.INSTANCE);
		putTypeDeclaration(typeDeclarations, IntegerType.INSTANCE);
		putTypeDeclaration(typeDeclarations, LongType.INSTANCE);
		putTypeDeclaration(typeDeclarations, BooleanType.INSTANCE);
		putTypeDeclaration(typeDeclarations, StringType.INSTANCE);
		putTypeDeclaration(typeDeclarations, FloatType.INSTANCE);
		putTypeDeclaration(typeDeclarations, DoubleType.INSTANCE);

		List<Document> xmldocuments = new ArrayList<>();

		for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
			Document doc = ResourceCompilerTaskFactory.getDocument(entry.getValue());
			xmldocuments.add(doc);
		}

		new EnumerationsParser(typeDeclarations, xmldocuments).parse();
		new FlagsParser(typeDeclarations, xmldocuments).parse();

		for (Map<String, TypeDeclaration> typemap : externalTypesOption) {
			for (Entry<String, TypeDeclaration> entry : typemap.entrySet()) {
				TypeDeclaration prev = typeDeclarations.put(entry.getKey(), entry.getValue());
				if (prev != null) {
					throw new IllegalArgumentException("Duplicate type declarations for name: " + entry.getKey()
							+ " with " + prev + " " + entry.getValue());
				}

			}
		}

		TypesSourcesData typesData = new TypesSourcesData();
		typesData.typeDeclarations = typeDeclarations;
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
				.getDirectoryCreate(TypesTaskFactory.TASK_NAME);
		outputDirectory.clear();

		SakerDirectory sourcesdir = outputDirectory.getDirectoryCreate("sources");
		SakerDirectory genDirectory = sourcesdir.getDirectoryCreate("gen");
		SakerDirectory fwdDirectory = genDirectory.getDirectoryCreate("fwd");

		genDirectory.add(new TemplatedSourceSakerFile("types.h",
				new TemplatedSource(descriptor::getInputStream, "gen/types.h").setThis(typesData)));
		fwdDirectory.add(new TemplatedSourceSakerFile("types.h",
				new TemplatedSource(descriptor::getInputStream, "gen/fwd/types.h").setThis(typesData)));
		genDirectory.add(new TemplatedSourceSakerFile("serialize.h",
				new TemplatedSource(descriptor::getInputStream, "gen/serialize.h").setThis(typesData)));
		SourceSakerFile cppfile = new TemplatedSourceSakerFile("deserializetypes.cpp",
				new TemplatedSource(descriptor::getInputStream, "gen/deserializetypes.cpp").setThis(typesData));
		genDirectory.add(cppfile);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(outputDirectory
				.getFilesRecursiveByPath(outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		outputDirectory.synchronize();

		TypesTaskFactory.Output result = new TypesTaskFactory.Output(typeDeclarations, sourcesdir.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	private static void putTypeDeclaration(NavigableMap<String, TypeDeclaration> typeDeclarations,
			TypeDeclaration type) {
		typeDeclarations.put(type.getName(), type);
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, inputOption);
		SerialUtils.writeExternalCollection(out, externalTypesOption, SerialUtils::writeExternalMap);
		out.writeObject(nullptr_type);
		out.writeObject(platformName);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		inputOption = SerialUtils.readExternalImmutableLinkedHashSet(in);
		externalTypesOption = SerialUtils.readExternalCollection(new ArrayList<>(), in,
				SerialUtils::readExternalImmutableLinkedHashMap);
		nullptr_type = SerialUtils.readExternalObject(in);
		platformName = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((externalTypesOption == null) ? 0 : externalTypesOption.hashCode());
		result = prime * result + ((inputOption == null) ? 0 : inputOption.hashCode());
		result = prime * result + ((nullptr_type == null) ? 0 : nullptr_type.hashCode());
		result = prime * result + ((platformName == null) ? 0 : platformName.hashCode());
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
		TypesWorkerTaskFactory other = (TypesWorkerTaskFactory) obj;
		if (externalTypesOption == null) {
			if (other.externalTypesOption != null)
				return false;
		} else if (!externalTypesOption.equals(other.externalTypesOption))
			return false;
		if (inputOption == null) {
			if (other.inputOption != null)
				return false;
		} else if (!inputOption.equals(other.inputOption))
			return false;
		if (nullptr_type == null) {
			if (other.nullptr_type != null)
				return false;
		} else if (!nullptr_type.equals(other.nullptr_type))
			return false;
		if (platformName == null) {
			if (other.platformName != null)
				return false;
		} else if (!platformName.equals(other.platformName))
			return false;
		return true;
	}

}
