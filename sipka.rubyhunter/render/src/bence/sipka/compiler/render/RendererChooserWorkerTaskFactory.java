package bence.sipka.compiler.render;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.TreeMap;
import java.util.stream.Collectors;

import bence.sipka.compiler.source.SourceSakerFile;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.enums.EnumType;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class RendererChooserWorkerTaskFactory implements TaskFactory<RendererChooserTaskFactory.Output>,
		Task<RendererChooserTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("render");

	private String platformName;
	private NavigableSet<String> renderApi;

	/**
	 * For {@link Externalizable}.
	 */
	public RendererChooserWorkerTaskFactory() {
	}

	public RendererChooserWorkerTaskFactory(String platformName, NavigableSet<String> renderapi) {
		this.platformName = platformName;
		this.renderApi = renderapi;
	}

	@Override
	public Task<? extends RendererChooserTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public RendererChooserTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
			LinkedHashMap<Object, Object> vals = new LinkedHashMap<>();
			vals.put("Platform", platformName);
			vals.put("Render API", renderApi);
			BuildTrace.setValues(vals, BuildTrace.VALUE_CATEGORY_TASK);
		}
		taskcontext.setStandardOutDisplayIdentifier(RendererChooserTaskFactory.TASK_NAME);
		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(RendererChooserTaskFactory.TASK_NAME);

		NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
		EnumType renderenum = new EnumType("RenderConfig");

		SakerDirectory sourcedir = buildDirectory;
		if (!ObjectUtils.isNullOrEmpty(platformName)) {
			sourcedir = sourcedir.getDirectoryCreate(platformName);
		}
		sourcedir.clear();

		SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");

		int idx = 0;
		for (String api : renderApi) {
			String enumname;
			switch (api.toLowerCase(Locale.ENGLISH)) {
				case "opengles20": {
					enumname = "OpenGlEs20";
					if ("ios".equalsIgnoreCase(platformName)) {
						sourcedir.getDirectoryCreate("KHR")
								.add(new TemplatedSourceSakerFile("khrplatform.h", new TemplatedSource(
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

		gendir.add(new TemplatedSourceSakerFile("renderers.h",
				new TemplatedSource(descriptor::getInputStream, "gen/renderers.h").setThis(renderApi)));
		SourceSakerFile rendererscpp = new TemplatedSourceSakerFile("renderers.cpp",
				new TemplatedSource(descriptor::getInputStream, "gen/renderers.cpp").setThis(renderenum.getValues()
						.entrySet().stream().sorted((a, b) -> Integer.compare(a.getValue(), b.getValue()))
						.map(e -> e.getKey()).collect(Collectors.toList())));
		gendir.add(rendererscpp);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(
				sourcedir.getFilesRecursiveByPath(sourcedir.getSakerPath(), DirectoryVisitPredicate.everything())));
		sourcedir.synchronize();

		RendererChooserTaskFactory.Output result = new RendererChooserTaskFactory.Output(typeDeclarations,
				sourcedir.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(platformName);
		SerialUtils.writeExternalCollection(out, renderApi);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		platformName = SerialUtils.readExternalObject(in);
		renderApi = SerialUtils.readExternalSortedImmutableNavigableSet(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((platformName == null) ? 0 : platformName.hashCode());
		result = prime * result + ((renderApi == null) ? 0 : renderApi.hashCode());
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
		RendererChooserWorkerTaskFactory other = (RendererChooserWorkerTaskFactory) obj;
		if (platformName == null) {
			if (other.platformName != null)
				return false;
		} else if (!platformName.equals(other.platformName))
			return false;
		if (renderApi == null) {
			if (other.renderApi != null)
				return false;
		} else if (!renderApi.equals(other.renderApi))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "RendererChooserWorkerTaskFactory[" + (platformName != null ? "platformName=" + platformName + ", " : "")
				+ (renderApi != null ? "renderapi=" + renderApi : "") + "]";
	}

}
