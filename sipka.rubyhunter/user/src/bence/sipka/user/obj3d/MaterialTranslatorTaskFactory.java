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
package bence.sipka.user.obj3d;

import java.io.BufferedReader;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.ListIterator;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;
import java.util.regex.Pattern;

import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.path.WildcardPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.runtime.execution.SakerLog;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class MaterialTranslatorTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	private static final Pattern SPLIT_PATTERN = Pattern.compile("[\\s]+");

	public static final String TASK_NAME = "sipka.rh.mat3d.translate";

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, MaterialLibrary> materials;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, MaterialLibrary> materials) {
			this.materials = ImmutableUtils.makeImmutableNavigableMap(materials);
		}

		public NavigableMap<String, MaterialLibrary> getMaterials() {
			return materials;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			SerialUtils.writeExternalMap(out, materials);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			materials = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((materials == null) ? 0 : materials.hashCode());
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
			if (materials == null) {
				if (other.materials != null)
					return false;
			} else if (!materials.equals(other.materials))
				return false;
			return true;
		}

	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {
			@SakerInput(value = { "", "Input" }, required = true)
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			@SakerInput(value = "DiffuseToAmbientFactors")
			public List<List<Object>> diffuseToAmbientFactorsOption = Collections.emptyList();

			private NavigableMap<String, MaterialLibrary> materials = new TreeMap<>();

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				diffuseToAmbientFactorsOption = ObjectUtils.newArrayList(diffuseToAmbientFactorsOption);
				for (ListIterator<List<Object>> it = diffuseToAmbientFactorsOption.listIterator(); it.hasNext();) {
					List<Object> l = ObjectUtils.newArrayList(it.next());
					l.set(0, Pattern.compile(l.get(0).toString()));
					it.set(l);
				}

				Collection<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, collectionstrategies);

				for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
					SakerFile f = entry.getValue();

					String ambientfactor = null;// f.getAttribute("diffuse_to_ambient_factor");
					for (int i = diffuseToAmbientFactorsOption.size() - 1; i >= 0; i--) {
						List<Object> difentry = diffuseToAmbientFactorsOption.get(i);
						if (((Pattern) difentry.get(0)).matcher(f.getName()).matches()) {
							ambientfactor = difentry.get(1).toString();
							break;
						}
					}

					MaterialLibrary lib = new MaterialLibrary();
					Material currentmat = null;
					SakerPath path = entry.getKey();
					SakerDirectory parent = f.getParent();
					try (BufferedReader br = new BufferedReader(new InputStreamReader(f.openInputStream()))) {
						int linenumber = 0;
						for (String line; (line = br.readLine()) != null; linenumber++) {
							line = line.trim();
							if (line.startsWith("#") || line.length() == 0) {
								continue;
							}
							String[] split = SPLIT_PATTERN.split(line);
							if (split.length == 0) {
								continue;
							}

							switch (split[0]) {
								case "newmtl": {
									currentmat = new Material(split[1]);
									lib.materials.add(currentmat);
									break;
								}
								case "Kd": {
									currentmat.diffuseColor = new Vector(1, split);
									if (ambientfactor != null) {
										float factor = Float.parseFloat(ambientfactor);
										currentmat.ambientColor = new Vector(currentmat.diffuseColor.x * factor,
												currentmat.diffuseColor.y * factor, currentmat.diffuseColor.z * factor);
									}
									break;
								}
								case "d": {
									currentmat.transparency = Float.parseFloat(split[1]);
									break;
								}
								case "Ka": {
									if (ambientfactor == null) {
										currentmat.ambientColor = new Vector(1, split);
									}
									break;
								}
								case "Ks": {
									currentmat.specularColor = new Vector(1, split);
									break;
								}
								case "Ns": {
									currentmat.specularExponent = Float.parseFloat(split[1]);
									break;
								}
								case "map_Kd": {
									SakerFile res = taskcontext.getTaskUtilities().resolveFileAtPath(parent,
											SakerPath.valueOf(split[1]));
									if (res == null) {
										SakerLog.error().path(f.getSakerPath()).line(linenumber)
												.println("Material texture not found: " + split[1] + " in directory: "
														+ parent + " for material: " + f.getName());
										throw new RuntimeException("Material texture not found: " + split[1]
												+ " in directory: " + parent + " for object: " + f.getName());
									}
									currentmat.texture = res.getSakerPath();
									break;
								}
								default: {
									// System.out.println(path + ":" + (linenumber + 1) + ": Warning: Ignored object directive: " + split[0]);
									break;
								}
							}
						}
					}
					synchronized (materials) {
						materials.put(path.toString().replace('\\', '/'), lib);
					}
				}

				Output result = new Output(materials);
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
	}

}
