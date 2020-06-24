package bence.sipka.opengl.registry;

import java.io.ByteArrayOutputStream;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.Serializable;
import java.io.UncheckedIOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeSet;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import bence.sipka.compiler.source.SourceTemplateTranslator;
import bence.sipka.compiler.source.SourceTemplateTranslator.TranslationHandler;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.opengl.registry.Command.Param;
import bence.sipka.opengl.registry.OpenGlRegistryParserTaskFactory.Output;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.SerializableContentDescriptor;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.runtime.execution.SakerLog;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class OpenGlRegistryParserWorkerTaskFactory implements TaskFactory<OpenGlRegistryParserTaskFactory.Output>,
		Task<OpenGlRegistryParserTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("render");

	private String feature;

	private String profile = "";

	private SakerPath directory;

	private String platformName;

	/**
	 * For {@link Externalizable}.
	 */
	public OpenGlRegistryParserWorkerTaskFactory() {
	}

	public OpenGlRegistryParserWorkerTaskFactory(String feature, String profile, SakerPath directory,
			String platformNameOption) {
		this.feature = feature;
		this.profile = profile;
		this.directory = directory;
		this.platformName = platformNameOption;
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
			LinkedHashMap<Object, Object> vals = new LinkedHashMap<>();
			vals.put("Feature", feature);
			vals.put("Profile", profile);
			vals.put("Platform", platformName);
			BuildTrace.setValues(vals, BuildTrace.VALUE_CATEGORY_TASK);
		}
		taskcontext.setStandardOutDisplayIdentifier(OpenGlRegistryParserTaskFactory.TASK_NAME);

		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(OpenGlRegistryParserTaskFactory.TASK_NAME);
		buildDirectory.clear();

		SakerDirectory srcdir = buildDirectory;
		if (directory != null) {
			srcdir = taskcontext.getTaskUtilities().resolveDirectoryAtRelativePathCreate(srcdir, directory);
		}
		srcdir.clear();
		SakerDirectory glgluedir = srcdir.getDirectoryCreate("glglue");

		GenerateData gendata = new GenerateData(feature, profile, platformName);

		SakerLog.log().verbose()
				.println("Generate OpenGL glue with: " + gendata.feature + " profile: " + gendata.profile);

		SakerFile header = gendata.getGlGlueHeaderFile("glglue.h");
		SakerFile platformheader = new ByteArraySakerFile("glglue_platform.h",
				descriptor.getBytes("opengl_registry/" + platformName + "/glglue_platform.h"));
		SakerFile glueclassheader = new TemplatedSourceSakerFile("glglueclass.h",
				new TemplatedSource(
						() -> descriptor.getInputStream("opengl_registry/" + platformName + "/glglueclass.h"))
								.setThis(gendata));
		SakerFile gluecpp = gendata.getGlGlueSourceFile("glglue.cpp");

		glgluedir.add(header);
		glgluedir.add(platformheader);
		glgluedir.add(glueclassheader);
		glgluedir.add(gluecpp);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(
				srcdir.getFilesRecursiveByPath(srcdir.getSakerPath(), DirectoryVisitPredicate.everything())));
		srcdir.synchronize();

		Output result = new Output(buildDirectory.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(feature);
		out.writeObject(profile);
		out.writeObject(directory);
		out.writeObject(platformName);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		feature = SerialUtils.readExternalObject(in);
		profile = SerialUtils.readExternalObject(in);
		directory = SerialUtils.readExternalObject(in);
		platformName = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((directory == null) ? 0 : directory.hashCode());
		result = prime * result + ((feature == null) ? 0 : feature.hashCode());
		result = prime * result + ((platformName == null) ? 0 : platformName.hashCode());
		result = prime * result + ((profile == null) ? 0 : profile.hashCode());
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
		OpenGlRegistryParserWorkerTaskFactory other = (OpenGlRegistryParserWorkerTaskFactory) obj;
		if (directory == null) {
			if (other.directory != null)
				return false;
		} else if (!directory.equals(other.directory))
			return false;
		if (feature == null) {
			if (other.feature != null)
				return false;
		} else if (!feature.equals(other.feature))
			return false;
		if (platformName == null) {
			if (other.platformName != null)
				return false;
		} else if (!platformName.equals(other.platformName))
			return false;
		if (profile == null) {
			if (other.profile != null)
				return false;
		} else if (!profile.equals(other.profile))
			return false;
		return true;
	}

	public static class GenerateData implements Serializable {
		private static final long serialVersionUID = 1L;

		private String feature;

		private String profile = "";

		private String platformNameOption;

		private transient Registry reg;
		private transient Set<Feature> features = new TreeSet<>((o1, o2) -> o1.name.compareTo(o2.name));
		private transient Set<Command> commands = new TreeSet<>((o1, o2) -> o1.name.compareTo(o2.name));
		private transient Set<String> enums = new TreeSet<>();
		private transient List<Type> types = new ArrayList<>();
		private transient List<RequireDefinition> requires = new ArrayList<>();
		private transient List<RemoveDefinition> removes = new ArrayList<>();
		private transient Set<Extension> extensions = new TreeSet<>((o1, o2) -> o1.name.compareTo(o2.name));
		private transient Feature ft;

		public GenerateData(String feature, String profile, String platformName) {
			this.feature = feature;
			this.profile = profile;
			this.platformNameOption = platformName;
		}

		public synchronized Registry getRegistry() throws IOException {
			if (reg == null) {
				DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
				dbFactory.setNamespaceAware(true);
				DocumentBuilder dBuilder;
				try {
					dBuilder = dbFactory.newDocumentBuilder();
					Document doc;
					try (InputStream is = descriptor.getInputStream("opengl_registry/opengl_registry.xml")) {
						doc = dBuilder.parse(is);
					}

					reg = new Registry(doc.getFirstChild());
					ft = reg.features.get(feature);
					if (ft == null) {
						throw new RuntimeException("No feature with name: " + feature);
					}
					for (Entry<String, Feature> entry : reg.features.entrySet()) {
						if (entry.getValue().api.equals(ft.api) && entry.getKey().compareTo(ft.name) <= 0) {
							SakerLog.log().verbose().println("Using feature: " + entry.getKey());
							features.add(entry.getValue());
						}
					}
					for (Feature f : features) {
						for (RequireDefinition req : f.requires) {
							if (req.profile != null && !req.profile.equals(profile))
								continue;
							requires.add(req);
						}

						for (RemoveDefinition rem : f.removes) {
							if (rem.profile != null && !rem.profile.equals(profile))
								continue;
							removes.add(rem);
						}
					}
					for (Extension ext : reg.extensions.values()) {
						if (ext.supportsApi(ft.api)) {
							// println("Using extension: " + ext.name);
							extensions.add(ext);
							for (RequireDefinition req : ext.requires) {
								if (req.profile != null && !req.profile.equals(profile))
									continue;
								requires.add(req);
							}
							for (RemoveDefinition rem : ext.removes) {
								if (rem.profile != null && !rem.profile.equals(profile))
									continue;
								removes.add(rem);
							}
						}
					}
					for (RequireDefinition req : requires) {
						for (String cmd : req.commands) {
							Command c = reg.commands.get(cmd);
							commands.add(c);
						}
						for (String st : req.types) {
							Type t = reg.getType(st, ft.api);
							if (!types.contains(t)) {
								types.add(t);
							}
						}
						for (String se : req.enums) {
							enums.add(se);
						}
					}
					for (RemoveDefinition rem : removes) {
						for (String cmd : rem.commands) {
							Command c = reg.commands.get(cmd);
							commands.remove(c);
						}
						for (String st : rem.types) {
							Type t = reg.getType(st, ft.api);
							types.remove(t);
						}
						for (String se : rem.enums) {
							enums.remove(se);
						}
					}

					for (Command c : commands) {
						if (!c.returnType.equals("void")) {
							Type t = reg.getType(c.returnType, ft.api);
							int index = types.size();
							while (!types.contains(t) && t != null) {
								types.add(index, t);
								if (t.requires != null) {
									t = reg.getType(t.requires, ft.api);
								} else {
									break;
								}
							}

						}
						for (Param p : c.params) {
							Type t = reg.getType(p.type, ft.api);
							int index = types.size();
							while (!types.contains(t) && t != null) {
								types.add(index, t);
								if (t.requires != null) {
									t = reg.getType(t.requires, ft.api);
								} else {
									break;
								}
							}
						}
					}
					types.sort((a, b) -> {
						return Integer.compare(reg.typesOrderList.indexOf(a), reg.typesOrderList.indexOf(b));
					});
				} catch (ParserConfigurationException | SAXException e) {
					throw new IOException();
				}
			}
			return reg;
		}

		private byte[] getGlGlueHeaderBytes() {
			Registry reg;
			try {
				reg = getRegistry();
			} catch (IOException e1) {
				throw new UncheckedIOException(e1);
			}
			try (ByteArrayOutputStream baos = new ByteArrayOutputStream();
					PrintStream out = new PrintStream(baos)) {
				try (InputStream headeris = descriptor.getInputStream("opengl_registry/glglue.h")) {
					Map<String, Object> valmap = new HashMap<>();
					valmap.put("types", types);
					valmap.put("extensions", extensions);
					valmap.put("commands", commands);
					valmap.put("features", features);
					valmap.put("registry", reg);
					SourceTemplateTranslator.translate(headeris, out, valmap, new TranslationHandler() {
						@Override
						public void replaceKey(String key, OutputStream os) throws IOException {
							PrintStream out = new PrintStream(os);
							switch (key) {
								case "glglue_enums": {
									for (Group group : reg.groups.values()) {
										boolean had = false;
										for (String enumname : (Iterable<String>) group.enums.stream()
												.sorted()::iterator) {
											if (enums.remove(enumname)) {
												if (!had) {
													had = true;
													out.println("/**");
													out.println(" * " + group.name);
													if (group.comment != null) {
														out.println(" * " + group.comment);
													}
													out.println(" */");
												}
												EnumValue val = reg.getEnumValue(enumname, ft.api);
												if (val.alias != null) {
													out.println("/* Alias: " + val.alias + " - " + val.name + " */");
												}
												out.println("#define " + val.name + " " + val.value);
											}
										}
										if (had) {
											out.println("/**");
											out.println(" * End of group: ");
											out.println(" * " + group.name);
											out.println(" */");
											out.println();
										}
									}
									for (String enumname : (Iterable<String>) enums.stream().sorted()::iterator) {
										EnumValue val = reg.getEnumValue(enumname, ft.api);
										if (val.alias != null) {
											out.println("/* Alias: " + val.alias + " - " + val.name + " */");
										}
										out.println("#define " + val.name + " " + val.value);
									}
									break;
								}
								case "glglue_commands": {
									for (Feature f : features) {
										out.println("/**");
										out.println(" * " + f.name);
										if (f.comment != null) {
											out.println(" * " + f.comment);
										}
										out.println(" */");
										for (RequireDefinition req : f.requires) {
											for (String cmdstr : req.commands) {
												Command cmd = reg.commands.get(cmdstr);
												if (commands.contains(cmd)) {
													out.println("typedef " + cmd.untilname + " ("
															+ Registry.APIENTRY_DEFINE + " * GLPROTO_" + cmd.name
															+ ") (" + cmd.getParametersString() + ");");
												}
											}
										}
										out.println("/**");
										out.println(" * End of feature api: ");
										out.println(" * " + f.name);
										out.println(" */");
										out.println();
									}
									break;
								}
								default: {
									TranslationHandler.super.replaceKey(key, out);
									break;
								}
							}
						}
					}, null);
				}
				return baos.toByteArray();
			} catch (IOException e) {
				throw new UncheckedIOException(e);
			}
		}

		private byte[] getGlGlueSourceBytes() {
			try (ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
				writeGlGlueSourceBytes(baos);
				return baos.toByteArray();
			} catch (IOException e) {
				throw new UncheckedIOException(e);
			}
		}

		private void writeGlGlueSourceBytes(OutputStream os) throws IOException {
			Registry reg = getRegistry();
			try (InputStream cppis = descriptor
					.getInputStream("opengl_registry/" + platformNameOption + "/glglue.cpp")) {
				Map<String, Object> valmap = new HashMap<>();
				valmap.put("registry", reg);
				valmap.put("features", features);
				valmap.put("commands", commands);
				SourceTemplateTranslator.translate(cppis, os, valmap, null, null);
			}
		}

		public SakerFile getGlGlueSourceFile(String name) {
			return new SakerFileBase(name) {
				@Override
				public ContentDescriptor getContentDescriptor() {
					return new SerializableContentDescriptor(GenerateData.this);
				}

				@Override
				public void writeToStreamImpl(OutputStream os) throws IOException {
					writeGlGlueSourceBytes(os);
				}
			};
		}

		public SakerFile getGlGlueHeaderFile(String name) {
			return new SakerFileBase(name) {
				@Override
				public ContentDescriptor getContentDescriptor() {
					return new SerializableContentDescriptor(GenerateData.this);
				}

				@Override
				public void writeToStreamImpl(OutputStream os) throws IOException {
					os.write(getGlGlueHeaderBytes());
				}
			};
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((feature == null) ? 0 : feature.hashCode());
			result = prime * result + ((platformNameOption == null) ? 0 : platformNameOption.hashCode());
			result = prime * result + ((profile == null) ? 0 : profile.hashCode());
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
			GenerateData other = (GenerateData) obj;
			if (feature == null) {
				if (other.feature != null)
					return false;
			} else if (!feature.equals(other.feature))
				return false;
			if (platformNameOption == null) {
				if (other.platformNameOption != null)
					return false;
			} else if (!platformNameOption.equals(other.platformNameOption))
				return false;
			if (profile == null) {
				if (other.profile != null)
					return false;
			} else if (!profile.equals(other.profile))
				return false;
			return true;
		}

	}

}
