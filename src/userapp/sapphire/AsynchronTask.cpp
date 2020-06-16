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
/*
 * AsynchronTask.cpp
 *
 *  Created on: 2016. okt. 22.
 *      Author: sipka
 */

#include <sapphire/AsynchronTask.h>
#include <sapphire/SapphireScene.h>

namespace userapp {

AsynchronTask::~AsynchronTask() {
	{
		Semaphore* sem;
		{
			MutexLocker lock { closeMutex };
			sem = this->finishSemaphore;
		}
		if (sem != nullptr) {
			sem->wait();
			delete sem;
			this->finishSemaphore = nullptr;
		}
	}
	delete runnable;
	auto* action = endAction;
	if (action != nullptr) {
		if (!canceled) {
			action->finish();
		}
		delete action;
	}
}

void AsynchronTask::start(bool tasks) {
	ASSERT(runnable != nullptr);
	ASSERT(finishSemaphore == nullptr);

	canceled = false;
	end = false;

	finishSemaphore = new Semaphore(Semaphore::auto_init { });

	core::GlobalMonotonicTimeListener::subscribeListener(*this);
	Thread t;
	t.start([=]() {
		(*runnable)();
		end = true;

		MutexLocker lock {closeMutex};
		Semaphore* sem = this->finishSemaphore;
		if (sem != nullptr) {
			sem->post();
		}
		return 0;
	});
}

void AsynchronTask::cancel() {
	Semaphore* sem;
	{
		MutexLocker lock { closeMutex };
		sem = this->finishSemaphore;
	}
	if (sem != nullptr) {
		core::GlobalMonotonicTimeListener::unsubscribeListener(*this);
		this->canceled = true;
		if (endAction != nullptr) {
			auto action = endAction;
			endAction = nullptr;
			if (end) {
				//thread already ended
				delete sem;
				this->finishSemaphore = nullptr;
				action->finish();
				delete action;
				return;
			} else {
				action->cancel();
			}
			delete action;
		}
		sem->wait();
		this->finishSemaphore = nullptr;
		delete sem;
	}
}

void AsynchronTask::finish() {
	Semaphore* sem;
	{
		MutexLocker lock { closeMutex };
		sem = this->finishSemaphore;
	}
	if (sem != nullptr) {
		core::GlobalMonotonicTimeListener::unsubscribeListener(*this);
		sem->wait();
		delete sem;
		this->finishSemaphore = nullptr;
		if (endAction != nullptr) {
			auto action = endAction;
			endAction = nullptr;
			action->finish();
			delete action;
		}
	}
}
void AsynchronTask::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	{
		MutexLocker { tasksMutex };
		tasks.clear([](LinkedNode<PostTask>* task) {
			auto&& got = task->get();
			if(got != nullptr) {
				(*got)();
			}
		});
	}
	if (end) {
		core::GlobalMonotonicTimeListener::unsubscribeListener(*this);

		{
			MutexLocker lock { closeMutex };
			if (this->finishSemaphore != nullptr) {
				delete this->finishSemaphore;
				this->finishSemaphore = nullptr;
			}
		}
		if (endAction != nullptr) {
			auto action = endAction;
			endAction = nullptr;
			if (!canceled) {
				action->finish();
			}
			delete action;
		}
	}
}
}  // namespace userapp

