package bence.sipka.utils;

import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.identifier.TaskIdentifier;
import saker.build.task.utils.SimpleStructuredObjectTaskResult;
import saker.build.task.utils.TaskBuilderResult;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.trace.BuildTrace;

public abstract class RHFrontendParameterizableTask implements ParameterizableTask<Object> {

	@Override
	public Object run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_FRONTEND);
		}

		TaskBuilderResult<?> task = createWorkerTask(taskcontext);
		TaskIdentifier taskid = task.getTaskIdentifier();
		taskcontext.startTask(taskid, task.getTaskFactory(), null);
		SimpleStructuredObjectTaskResult result = new SimpleStructuredObjectTaskResult(taskid);
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	protected abstract TaskBuilderResult<?> createWorkerTask(TaskContext taskcontext);

}
