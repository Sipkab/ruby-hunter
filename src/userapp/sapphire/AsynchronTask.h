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
 * AsynchronTask.h
 *
 *  Created on: 2016. okt. 14.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_ASYNCHRONTASK_H_
#define TEST_SAPPHIRE_ASYNCHRONTASK_H_

#include <framework/utils/ContainerLinkedNode.h>
#include <framework/utils/FixedString.h>
#include <framework/core/timing.h>
#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>
#include <framework/threading/Mutex.h>

namespace userapp {
using namespace rhfw;
class SapphireScene;

class AsynchronTask: private core::TimeListener {
	struct PostTask: public LinkedNode<PostTask> {
		virtual void operator()() = 0;

		virtual PostTask* get() override {
			return this;
		}
	};
	struct EndingAction {
		virtual ~EndingAction() {
		}
		virtual void finish() = 0;
		virtual void cancel() = 0;
	};
	struct Runnable {
		virtual ~Runnable() {
		}

		virtual void operator()() = 0;
	};
	template<typename Run>
	struct TypedRunnable: public Runnable {
		typename util::remove_reference<Run>::type run;

		TypedRunnable(Run&& run)
				: run(util::move(run)) {
		}

		virtual void operator()() override {
			run();
		}
	};
	template<typename Finish, typename Cancel>
	struct Ender: public EndingAction {
		typename util::remove_reference<Finish>::type finisher;
		typename util::remove_reference<Cancel>::type canceler;

		Ender(Finish&& finisher, Cancel&& canceler)
				: finisher(util::move(finisher)), canceler(util::move(canceler)) {
		}
		virtual void finish() override {
			finisher();
		}
		virtual void cancel() override {
			canceler();
		}
	};

	bool canceled = false;
	bool end = false;
	Semaphore* finishSemaphore = nullptr;
	EndingAction* endAction = nullptr;
	Runnable* runnable = nullptr;
	Mutex tasksMutex { Mutex::auto_init { } };
	Mutex closeMutex { Mutex::auto_init { } };
	LinkedList<PostTask> tasks;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;
public:
	template<typename Function, typename Finish, typename Cancel>
	AsynchronTask(Function&& f, Finish&& fin, Cancel&& cancel)
			: AsynchronTask(util::forward<Finish>(fin), util::forward<Cancel>(cancel)) {
		setRunnable(f);
	}
	template<typename Finish, typename Cancel>
	AsynchronTask(Finish&& fin, Cancel&& cancel) {
		endAction = new Ender<Finish, Cancel>(util::move(fin), util::move(cancel));
	}
	AsynchronTask() {
	}
	AsynchronTask(AsynchronTask&& o) = delete;
	AsynchronTask& operator=(AsynchronTask&& o) = delete;
	~AsynchronTask();
	template<typename Finish, typename Cancel>
	void setActions(Finish&& fin, Cancel&& cancel) {
		delete endAction;
		endAction = new Ender<Finish, Cancel>(util::move(fin), util::move(cancel));
	}

	void start(bool tasks = true);

	template<typename Run>
	void setRunnable(Run&& r) {
		delete this->runnable;

		this->runnable = new TypedRunnable<Run>(util::forward<Run>(r));
	}

	template<typename Task>
	void postTask(Task&& t) {
		ASSERT(core::TimeListener::isSubscribed());
		struct RealTask: public PostTask {
			typename util::remove_reference<Task>::type task;

			RealTask(Task&& t)
					: task(util::move(t)) {

			}
			virtual void operator()() override {
				task();
			}
		};
		auto* task = new RealTask(util::move(t));
		MutexLocker { tasksMutex };
		tasks.addToEnd(*task);
	}
	void cancel();
	void finish();

	bool isCanceled() const {
		return canceled;
	}

};

} // namespace userapp

#endif /* TEST_SAPPHIRE_ASYNCHRONTASK_H_ */
