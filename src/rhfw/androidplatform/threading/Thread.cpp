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
 * Thread.cpp
 *
 *  Created on: 2015 aug. 11
 *      Author: sipka
 */

#include <gen/log.h>

#include <androidplatform/threading/Thread.h>
#include <androidplatform/AndroidPlatform.h>
#include <framework/threading/Mutex.h>
#include <framework/utils/LinkedList.h>

#include <unistd.h>

namespace rhfw {

static JavaVM* vm = nullptr;

static LinkedList<JniThreadConnection, false> jniConnections;
static Mutex jniMutex { Mutex::auto_init { } };

JniThreadConnection::JniThreadConnection(pthread_t tid)
		: threadid { tid } {
	{
		MutexLocker l { jniMutex };
		jniConnections.addToEnd(*this);
	}
}

JniThreadConnection::~JniThreadConnection() {
	if (env != nullptr) {
		ASSERT(vm != nullptr) << "java vm is nullptr";
		jint result;
		result = vm->DetachCurrentThread();
		ASSERT(result == JNI_OK) << "failed to detach thread, result: " << result;
		LOGI()<<"Detached thread from Java VM, tid: " << gettid();

	}
	{
		MutexLocker l {jniMutex};
		removeLinkFromList();
	}
}

JNIEnv* JniThreadConnection::getEnv() {
	if (env == nullptr) {

		ASSERT(vm != nullptr) << "java vm is nullptr";
		jint result;
		result = vm->AttachCurrentThread(&env, nullptr);
		ASSERT(result == JNI_OK) << "failed to attach thread, result: " << result;
		LOGI()<<"Attached thread to Java VM, tid: " << gettid() << ", env: " << env;
	}
	return env;
}

JNIEnv* Thread::getJniEnvForCurrentThread() {
	auto tid = pthread_self();
	ASSERT(gettid() != getpid()) << "Use activity->env instead on main thread";
	{
		MutexLocker l { jniMutex };
		for (auto* n : jniConnections.nodes()) {
			JniThreadConnection* con = static_cast<JniThreadConnection*>(n);
			if (*con == tid) {
				return con->getEnv();
			}
		}
	}
	THROW()<<"Thread not found in pool "<< tid;
	return nullptr;
}

bool Thread::setJavaVm(JavaVM* vm) {
	ASSERT(::rhfw::vm == nullptr || ::rhfw::vm == vm) << "Setting virtual machine is not valid old: " << ::rhfw::vm << ", new: " << vm;
	if (::rhfw::vm == nullptr) {
		::rhfw::vm = vm;
		return true;
	}
	return false;
}
} // namespace rhfw

