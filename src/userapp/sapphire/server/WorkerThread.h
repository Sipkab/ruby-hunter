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
 * WorkerThread.h
 *
 *  Created on: 2016. okt. 18.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_WORKERTHREAD_H_
#define TEST_SAPPHIRE_SERVER_WORKERTHREAD_H_

#include <framework/utils/utility.h>
#include <framework/threading/Thread.h>
#include <framework/threading/Mutex.h>
#include <framework/threading/Semaphore.h>
#include <framework/utils/LinkedList.h>

namespace userapp {
using namespace rhfw;

class WorkerThread {
private:
	struct Job {
		virtual ~Job() {
		}

		virtual void operator()() = 0;
	};
	LinkedList<Job> jobs;
	Mutex jobsMutex { Mutex::auto_init { } };
	Semaphore jobsSemaphore { Semaphore::auto_init { } };
	Semaphore exitSemaphore { Semaphore::auto_init { } };

	static const char EXIT_STATE_RUNNING = 0;
	static const char EXIT_STATE_SIGNALED = 1;
	static const char EXIT_STATE_WAITED = 2;

	char exitState = EXIT_STATE_WAITED;
public:
	WorkerThread() {
	}
	WorkerThread(WorkerThread&& o) = delete;
	WorkerThread& operator=(WorkerThread&& o) = delete;

	~WorkerThread() {
		stop();
	}

	void reset() {
		stop();
		jobs.clear();
	}

	void start() {
		exitState = EXIT_STATE_RUNNING;
		Thread t;
		t.start([=] {
			while(true) {
				Job* op = nullptr;
				{
					MutexLocker lock {jobsMutex};
					if (!jobs.isEmpty()) {
						auto* node = *jobs.nodes().begin();
						node->removeLinkFromList();
						op = node->get();
					}
				}
				if (op == nullptr) {
					jobsSemaphore.wait();
					if (exitState != EXIT_STATE_RUNNING) {
						break;
					}
					continue;
				}
				(*op)();
				delete op;
			}
			exitSemaphore.post();
			return 0;
		});
	}
	void signalStop() {
		if (exitState >= EXIT_STATE_SIGNALED) {
			return;
		}
		exitState = EXIT_STATE_SIGNALED;
		jobsSemaphore.post();
	}
	void stop() {
		signalStop();
		if (exitState < EXIT_STATE_WAITED) {
			exitSemaphore.wait();
			exitState = EXIT_STATE_WAITED;
		}
	}

	template<typename Functor>
	bool post(Functor&& j) {
		if (exitState != EXIT_STATE_RUNNING) {
			return false;
		}

		struct ConcreteJob: public Job, public LinkedNode<Job> {
			typename util::remove_reference<Functor>::type func;

			ConcreteJob(Functor&& func)
					: func(util::forward<Functor>(func)) {
			}

			virtual void operator()() override {
				func();
			}

			virtual Job* get() override {
				return this;
			}
		};

		auto job = new ConcreteJob(util::forward<Functor>(j));
		{
			MutexLocker m { jobsMutex };
			jobs.addToEnd(*job);
		}
		jobsSemaphore.post();
		return true;
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_SERVER_WORKERTHREAD_H_ */
