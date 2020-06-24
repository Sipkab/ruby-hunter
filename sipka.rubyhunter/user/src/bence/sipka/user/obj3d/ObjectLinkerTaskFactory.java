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

import bence.sipka.compiler.asset.AssetsAllocatorTaskFactory;
import bence.sipka.utils.RHFrontendParameterizableTask;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.identifier.TaskIdentifier;
import saker.build.task.utils.TaskBuilderResult;
import saker.build.task.utils.annot.SakerInput;
import saker.nest.utils.FrontendTaskFactory;

public class ObjectLinkerTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.obj3d.link";

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new RHFrontendParameterizableTask() {

			@SakerInput(value = { "", "Translated" }, required = true)
			public ObjectTranslatorTaskFactory.Output translaterOutputOption;

			@SakerInput(value = "Assets", required = true)
			public AssetsAllocatorTaskFactory.Output assetsOption;

			@Override
			protected TaskBuilderResult<?> createWorkerTask(TaskContext taskcontext) {
				return TaskBuilderResult.create(
						TaskIdentifier.builder(ObjectLinkerWorkerTaskFactory.class.getName()).build(),
						new ObjectLinkerWorkerTaskFactory(translaterOutputOption, assetsOption));
			}
		};
	}

}
