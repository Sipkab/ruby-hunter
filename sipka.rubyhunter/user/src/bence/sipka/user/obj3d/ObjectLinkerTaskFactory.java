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

import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import bence.sipka.compiler.asset.AssetsAllocatorTaskFactory;
import bence.sipka.user.obj3d.ObjectCollection.DuplicateObjectData;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.utils.annot.SakerInput;
import saker.nest.utils.FrontendTaskFactory;

public class ObjectLinkerTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.obj3d.link";

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = { "", "Translated" }, required = true)
			public ObjectTranslatorTaskFactory.Output translaterOutputOption;

			@SakerInput(value = "Assets", required = true)
			public AssetsAllocatorTaskFactory.Output assetsOption;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				SakerDirectory genDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				genDirectory.clear();

				Map<String, Entry<SakerPath, Integer>> assetsIdentifierMap = assetsOption.getAssetIdentifiers();
				Map<String, Integer> simpleAssetsIdentifierMap = new TreeMap<>();

				for (Entry<String, Entry<SakerPath, Integer>> entry : assetsIdentifierMap.entrySet()) {
					simpleAssetsIdentifierMap.put(entry.getKey(), entry.getValue().getValue());
				}

				ObjectCollection objcoll = translaterOutputOption.getObjectCollection();
				ObjectConfiguration objconfig = new ObjectConfiguration(objcoll,
						taskcontext.getTaskWorkingDirectoryPath(), simpleAssetsIdentifierMap);
				byte[] data = objconfig.getBytes();

				for (ObjectData objdata : objcoll.objects) {
					String meshfilename = objdata.fileName + ".mesh";
					genDirectory.add(new ObjectModularFile(meshfilename, objconfig, objdata));
				}

				for (DuplicateObjectData dd : objcoll.duplicateDatas) {
					genDirectory.add(
							new DuplicatObjectModularFile(dd.fileName, objconfig, dd.objectData, dd.materialLibrary));
				}

				SakerFile objcollfile = new ByteArraySakerFile("objects_3d_collection", data);
				genDirectory.add(objcollfile);

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(genDirectory.getFilesRecursiveByPath(
								genDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				genDirectory.synchronize();

				// TODO Auto-generated method stub
				return null;
			}
		};
	}

}
