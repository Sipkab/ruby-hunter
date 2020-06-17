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
 * servermain.cpp
 *
 *  Created on: 2016. szept. 26.
 *      Author: sipka
 */

#include <framework/io/network/ipv4/tcp/TCPIPv4Socket.h>
#include <framework/threading/Thread.h>
#include <framework/utils/MemoryInput.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/LinkedList.h>
#include <framework/io/byteorder.h>
#include <framework/resource/Resource.h>
#include <framework/io/stream/LoopedInputStream.h>
#include <framework/io/stream/BufferedInputStream.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>
#include <framework/threading/Semaphore.h>
#include <framework/threading/Mutex.h>

#include <sapphire/sapphireconstants.h>
#include <sapphireserver/storage/local/LocalSapphireDataStorage.h>
#include <sapphireserver/client/ClientConnection.h>
#include <sapphire/level/SapphireUUID.h>

#include <sapphireserver/servermain.h>

#include <gen/log.h>
#include <gen/fwd/types.h>
#include <gen/platform.h>
#include <gen/configuration.h>

#include <string.h>
#include <stdio.h>
#include <time.h>

using namespace rhfw;
using namespace userapp;

namespace userapp {

static StorageFileDescriptor* LogFile = nullptr;
static FileOutput* LogFileOutput = nullptr;

SapphireDataStorage* DataStorage;
LinkedList<ClientConnection, false> ClientConnections;

UserStateListener::Events UserStateEvents;
LogEventListener::Events LogEvents;
MaintenanceListener::Events MaintenanceEvents;

WorkerThread MainWorkerThread;

Resource<RandomContext> MainRandomContext;
Randomer* MainRandomer = nullptr;
static bool MaintenanceMode = false;
static bool MaintenanceQueued = false;
static Mutex MaintenanceMutex { Mutex::auto_init { } };
static unsigned int SuggestedUpgradeVersion = 2;

bool IsMaintenanceMode() {
	MutexLocker locker { MaintenanceMutex };
	return MaintenanceMode;
}
bool IsMaintenanceQueued() {
	MutexLocker locker { MaintenanceMutex };
	return MaintenanceQueued;
}
bool IsMaintenanceConnectionDisabled() {
	MutexLocker locker { MaintenanceMutex };
	return MaintenanceMode || MaintenanceQueued;
}
unsigned int GetSuggestedUpgradeVersion() {
	MutexLocker locker { MaintenanceMutex };
	return SuggestedUpgradeVersion;
}
static void SetSuggestedUpgradeVersion(unsigned int version) {
	{
		MutexLocker locker { MaintenanceMutex };
		SuggestedUpgradeVersion = version;
	}
	EndianOutputStream<Endianness::Big>::wrap(
			StorageFileDescriptor { StorageDirectoryDescriptor::Root() + "suggestedversion" }.openOutputStream()).serialize<uint32>(
			version);
}
static void InitSuggestedVersion() {
	EndianInputStream<Endianness::Big>::wrap(
			StorageFileDescriptor { StorageDirectoryDescriptor::Root() + "suggestedversion" }.openInputStream()).deserialize<uint32>(
			SuggestedUpgradeVersion);
}

class MaintenanceTask: public LinkedNode<MaintenanceTask> {
public:
	virtual ~MaintenanceTask() {
	}
	virtual void doWork() = 0;

	virtual MaintenanceTask* get() override {
		return this;
	}
};
static LinkedList<MaintenanceTask> MaintenanceTasks;

static void writeLogEvent(const FixedString& log) {
	if (LogFileOutput != nullptr) {
		char buffer[256];
		time_t rawtime;

		time(&rawtime);
		struct tm * timeinfo;
		timeinfo = gmtime(&rawtime);
		int dlen = sprintf(buffer, "%d.%02d.%02d %02d:%02d:%02d\t", 1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday,
				timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

		LOGI() << "Log event: " << buffer << "\t" << log;
		LogFileOutput->write(buffer, dlen);
		LogFileOutput->write((const char*) log, log.length());
		LogFileOutput->write("\r\n", 2);

		static unsigned int LogCounter = 0;
		if (++LogCounter % 16 == 0) {
			//flush at a frequency
			LogFileOutput->flushBuffer();
			LogFileOutput->flushDisk();
		}
	}
}

static void archiveLog() {
	if (LogFileOutput != nullptr) {
		writeLogEvent("Server\tArchiving log");
		delete LogFileOutput;
		LogFileOutput = nullptr;

	}
	char buffer[256];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = gmtime(&rawtime);
	int dlen = sprintf(buffer, "serverlog.archive.%d.%02d.%02d_%02dh%02dm%02ds.log", 1900 + timeinfo->tm_year, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	StorageFileDescriptor logfilefd { StorageDirectoryDescriptor::Root() + buffer };
	LogFile->move(logfilefd);
	LogFileOutput = LogFile->createOutput();
	if (LogFileOutput != nullptr) {
		LogFileOutput->setAppend(true);
		LogFileOutput->open();
		writeLogEvent("Server\tArchived log");
	}
}

void postLogEvent(const FixedString& log) {
	MainWorkerThread.post([=] {
		writeLogEvent(log);
		for (auto&& l : LogEvents.foreach()) {
			l(log);
		}
	});
}
static void postServerLogEvent(const FixedString& log) {
	postLogEvent(FixedString { "Server\t" } + log);
}
void postConnectionLogEvent(const SapphireUUID& connectionid, const FixedString& data) {
	postLogEvent(FixedString { "Connection\t" } + connectionid.asString() + "\t" + data);
}
void postConnectionUserLogEvent(const SapphireUUID& connectionid, const SapphireUUID& uuid, const FixedString& data) {
	postConnectionLogEvent(connectionid, FixedString { "User\t" } + uuid.asString() + "\t" + data);
}

void MaintenanceOpportunity() {
	MutexLocker locker { MaintenanceMutex };

	if (MaintenanceTasks.isEmpty()) {
		return;
	}
	postServerLogEvent("MaintenanceOpportunity\tStart");
	MaintenanceTasks.clear([](LinkedNode<MaintenanceTask>* node) {
		node->get()->doWork();
	});
	postServerLogEvent("MaintenanceOpportunity\tEnd");
}
template<typename Task>
static void QueueMaintenanceTask(Task&& task) {
	class LocalMaintenanceTask: public MaintenanceTask {
		typename util::remove_reference<Task>::type task;
	public:
		LocalMaintenanceTask(Task&& task)
				: task(util::forward<Task>(task)) {
		}
		virtual void doWork() override {
			task();
		}
	};

	MutexLocker locker { MaintenanceMutex };
	if (MaintenanceMode) {
		task();
	} else {
		MaintenanceTasks.addToEnd(*new LocalMaintenanceTask(util::forward<Task>(task)));
		MainWorkerThread.post([] {
			if(ClientConnections.isEmpty()) {
				MaintenanceOpportunity();
			}
		});
	}
}

}  // namespace userapp
static TCPIPv4Socket* AcceptorSocket = nullptr;
static TCPIPv4Socket* ControlSocket = nullptr;
static TCPConnection* ControlConnection = nullptr;
static Semaphore ControlSemaphore { Semaphore::auto_init { } };

static void removeWhiteSpace(char* buffer, unsigned int& bufcount) {
	for (int i = bufcount - 1; i > 0; --i) {
		if (buffer[i] == '\b') {
			memmove(buffer + i - 1, buffer + i + 1, bufcount - i - 1);
			bufcount -= 2;
			--i;
		}
	}
	unsigned int c = 0;
	while (c < bufcount && (buffer[c] < ' ' || buffer[c] >= 127)) {
		++c;
	}
	bufcount -= c;
	memmove(buffer, buffer + c, bufcount);
}
static bool consumeBufferWord(const char* buffer, unsigned int bufcount, const char* str, unsigned int strlen) {
	if (bufcount < strlen + 1) {
		return false;
	}
	if (memcmp(buffer, str, strlen) == 0 && (buffer[strlen] == ' ' || buffer[strlen] == '\n' || buffer[strlen] == '\r')) {
		return true;
	}
	return false;
}
static bool consumeBufferLine(char* buffer, unsigned int bufcount, const char* str, unsigned int strlen) {
	if (bufcount < strlen + 1) {
		return false;
	}
	if (memcmp(buffer, str, strlen) == 0) {
		if (buffer[strlen] == '\r') {
			++strlen;
		}
		if (strlen < bufcount && buffer[strlen] == '\n') {
			++strlen;
		} else {
			//we require '\n'
			return false;
		}
		return true;
	}
	return false;
}
static int newLineIndex(const char* buffer, unsigned int bufcount) {
	for (int i = 0; i < bufcount; ++i) {
		if (buffer[i] == '\n' || buffer[i] == '\r') {
			return i;
		}
	}
	return -1;
}

static bool handleControlConnection(TCPConnection& socket, Semaphore& sem) {
#define BufferStartsWithWord(str) consumeBufferWord(buffer, bufcount, str, sizeof(str) - 1)
#define BufferStartsWithLine(str) (nlindex == sizeof(str) - 1 && memcmp(buffer, str, sizeof(str)) == 0)
#define writeString(str) write(str, sizeof(str) - 1)

	UserStateListener::Listener userstatelistener;
	LogEventListener::Listener loglistener;

	MainWorkerThread.post([&] {
		ControlConnection = &socket;
		sem.post();
	});
	sem.wait();
	socket.writeString("> Hello!\r\n");

	char buffer[4097];
	unsigned int bufcount = 0;
	bool result = true;
	for (int read; (read = socket.read(buffer + bufcount, 4096 - bufcount)) > 0;) {
		bufcount += read;
		removeWhiteSpace(buffer, bufcount);
		int nlindex = newLineIndex(buffer, bufcount);
		if (nlindex < 0) {
			continue;
		}
		buffer[nlindex] = 0;
		socket.writeString("> Command: ");
		socket.write(buffer, nlindex);
		socket.writeString(".\r\n");
		LOGI() << "Control command: " << buffer << " len: " << nlindex;
		if (BufferStartsWithLine("?") || BufferStartsWithLine("help")) {
			socket.writeString("> Commands: shutdown, logoff|bye|exit, onlineusers, subuser, unsubuser, sublog, unsublog, archivelog, "
					"entermaintenance, queueentermaintenance, exitmaintenance, ismaintenance, removelevel <uuid>.\r\n");
		} else if (BufferStartsWithLine("shutdown")) {
			socket.writeString("> Exiting.\r\n");
			MainWorkerThread.post([&] {
				AcceptorSocket->destroy();
				sem.post();
			});
			sem.wait();
			result = false;
			break;
		} else if (BufferStartsWithLine("onlineusers")) {
			socket.writeString("> Online users:\r\n");
			MainWorkerThread.post([&] {
				for (auto&& c : ClientConnections.objects()) {
					auto&& usern = c.getUserName();

					socket.writeString("> ");
					socket.write((const char*)usern, usern.length());
					socket.writeString("\r\n");
				}
				sem.post();
			});
			sem.wait();
			socket.writeString("> .\r\n");
		} else if (BufferStartsWithLine("subuser")) {
			if (userstatelistener == nullptr) {
				userstatelistener = UserStateListener::new_listener([&] (const ClientConnectionState& c, UserState state) {
					switch (state) {
						case UserState::CONNECTED: {
							socket.writeString("> User connected.\r\n");
							break;
						}
						case UserState::AUTHORIZED: {
							socket.writeString("> User authorized: ");
							auto&& usern = c.userName;
							socket.write((const char*)usern, usern.length());
							socket.writeString(".\r\n");
							break;
						}
						case UserState::DISCONNECTED: {
							auto&& usern = c.userName;
							if(usern.length() > 0) {
								socket.writeString("> User disconnected: ");
								socket.write((const char*)usern, usern.length());
								socket.writeString(".\r\n");
							} else {
								socket.writeString("> User disconnected.\r\n");
							}
							break;
						}
						default: {
							break;
						}
					}
				});
				MainWorkerThread.post([&] {
					UserStateEvents += userstatelistener;
					sem.post();
				});
				sem.wait();
				socket.writeString("> Subscribed to user notifications.\r\n");
			} else {
				socket.writeString("> Already subscribed to user notifications.\r\n");
			}
		} else if (BufferStartsWithLine("unsubuser")) {
			if (userstatelistener != nullptr) {
				MainWorkerThread.post([&] {
					userstatelistener = nullptr;
					sem.post();
				});
				sem.wait();
				socket.writeString("> Unsubscribed from user notifications.\r\n");
			} else {
				socket.writeString("> Not subscribed to user notifications.\r\n");
			}
		} else if (BufferStartsWithLine("sublog")) {
			if (loglistener == nullptr) {
				loglistener = LogEventListener::new_listener([&] (const FixedString& log) {
					socket.writeString("> Log event: ");
					socket.write((const char*)log, log.length());
					socket.writeString(".\r\n");
				});
				MainWorkerThread.post([&] {
					LogEvents += loglistener;
					sem.post();
				});
				sem.wait();
				socket.writeString("> Subscribed to log events.\r\n");
			} else {
				socket.writeString("> Already subscribed to log events.\r\n");
			}
		} else if (BufferStartsWithLine("unsublog")) {
			if (loglistener != nullptr) {
				MainWorkerThread.post([&] {
					loglistener = nullptr;
					sem.post();
				});
				sem.wait();
				socket.writeString("> Unsubscribed from log events.\r\n");
			} else {
				socket.writeString("> Not subscribed to log events.\r\n");
			}
		} else if (BufferStartsWithLine("archivelog")) {
			MainWorkerThread.post([&] {
				archiveLog();
				sem.post();
			});
			sem.wait();
			socket.writeString("> Log archived.\r\n");
		} else if (BufferStartsWithLine("entermaintenance")) {
			bool activated = false;
			{
				MutexLocker locker { MaintenanceMutex };
				if (!MaintenanceMode) {
					MaintenanceMode = true;
					for (auto&& l : MaintenanceEvents.foreach()) {
						l(true);
					}
					postServerLogEvent("Maintenance\tEnter");
					activated = true;
				}
			}
			if (activated) {
				socket.writeString("> Maintenance mode activated.\r\n");
			} else {
				socket.writeString("> Already in maintenance mode.\r\n");
			}
		} else if (BufferStartsWithLine("queueentermaintenance")) {
			MutexLocker locker { MaintenanceMutex };
			if (!IsMaintenanceMode()) {
				if (IsMaintenanceQueued()) {
					socket.writeString("> Maintenance mode already queued.\r\n");
				} else {
					socket.writeString("> Maintenance mode queued.\r\n");
					MaintenanceQueued = true;
					QueueMaintenanceTask([=] {
						MutexLocker locker {MaintenanceMutex};
						if (!MaintenanceMode) {
							MaintenanceMode = true;
							MaintenanceQueued = false;
							for (auto&& l : MaintenanceEvents.foreach()) {
								l(true);
							}
							postServerLogEvent("Maintenance\tEnter");
						}
					});
				}
			} else {
				socket.writeString("> Already in maintenance mode.\r\n");
			}
		} else if (BufferStartsWithLine("exitmaintenance")) {
			bool deactivated = false;
			{
				MutexLocker locker { MaintenanceMutex };
				if (MaintenanceMode) {
					MaintenanceMode = false;
					for (auto&& l : MaintenanceEvents.foreach()) {
						l(false);
					}
					postServerLogEvent("Maintenance\tExit");
					deactivated = true;
				}
			}
			if (deactivated) {
				socket.writeString("> Maintenance mode deactivated.\r\n");
			} else {
				socket.writeString("> Not in maintenance mode.\r\n");
			}
		} else if (BufferStartsWithLine("ismaintenance")) {
			if (IsMaintenanceMode()) {
				socket.writeString("> Maintenance mode active.\r\n");
			} else {
				socket.writeString("> Maintenance mode inactive.\r\n");
			}
		} else if (BufferStartsWithWord("removelevel")) {
			SapphireUUID leveluuid;
			if (SapphireUUID::fromString(&leveluuid, buffer + sizeof("removelevel"), bufcount - sizeof("removelevel"))) {
				socket.writeString("> Level removal queued.\r\n");
				QueueMaintenanceTask([=] {
					LOGI()<< "Remove level with uuid: " << leveluuid.asString();
					SapphireStorageError error = DataStorage->removeLevel(leveluuid);
					switch (error) {
						case SapphireStorageError::SUCCESS: {
							postServerLogEvent(FixedString {"Administrator\tRemoveLevel\tSUCCESS\t"}+ leveluuid.asString());
							break;
						}
						default: {
							char buffer[256];
							sprintf(buffer, "Administrator\tRemoveLevel\tFailed\t%s\tError\t%u",
									(const char*)leveluuid.asString(), (unsigned int) error);
							postServerLogEvent(buffer);
							break;
						}
					}
				});
			} else {
				socket.writeString("> Invalid UUID format.\r\n");
			}
		} else if (BufferStartsWithWord("suggestversion")) {
			unsigned int version;
			auto res = sscanf(buffer + sizeof("suggestversion"), "%u", &version);
			if (res <= 0) {
				socket.writeString("> Failed to parse suggested version.\r\n");
			} else {
				char bufferresponse[256];
				unsigned int len;
				if (version > SAPPHIRE_RELEASE_VERSION_NUMBER) {
					len = sprintf(bufferresponse, "> Suggested version greater than sever version: %u > %u.\r\n", version,
					SAPPHIRE_RELEASE_VERSION_NUMBER);
				} else {
					SetSuggestedUpgradeVersion(version);
					len = sprintf(bufferresponse, "> Suggested version updated: %u.\r\n", version);
				}
				socket.write(bufferresponse, len);
			}
		} else if (BufferStartsWithLine("logoff") || BufferStartsWithLine("bye") || BufferStartsWithLine("exit")) {
			socket.writeString("> Bye.\r\n");
			break;
		} else if (BufferStartsWithLine("pingall")) {
			socket.writeString("> Pinging users.\r\n");
			MainWorkerThread.post([&] {
				for (auto&& c : ClientConnections.objects()) {
					c.sendPingRequest(0);
				}
				sem.post();
			});
			sem.wait();
			socket.writeString("> Pinged all users.\r\n");
		}
#if RHFW_DEBUG
		else if (BufferStartsWithLine("throw")) {
			THROW();
		}
#endif /* RHFW_DEBUG */
		else {
			socket.writeString("> Unknown command: ");
			socket.write(buffer, nlindex);
			socket.writeString(".\r\n");
		}
		memmove(buffer, buffer + nlindex + 1, bufcount - nlindex - 1);
		bufcount -= nlindex + 1;
		removeWhiteSpace(buffer, bufcount);
	}
	MainWorkerThread.post([&] {
		userstatelistener = nullptr;
		loglistener = nullptr;
		ControlConnection = nullptr;
		sem.post();
	});
	sem.wait();
	return result;
}

static void startControlThread() {
	LOGI() << "Starting control thread";
	Thread t;
	t.start([] {
		LOGI() << "Start control thread";
		TCPIPv4Address addr;
		addr.setPort(SAPPHIRE_SERVER_CONTROL_PORT_NUMBER);
		TCPIPv4Socket sock(addr);
		sock.initialize();
		Semaphore sem {Semaphore::auto_init {}};

		MainWorkerThread.post([&] () {
					ControlSocket = &sock;
					sem.post();
				});
		sem.wait();
		LOGI() << "Enter control loop";
		while (true) {
			auto* res = sock.accept();
			if (res == nullptr) {
				break;
			}

			char buffer[256];
			auto addr = static_cast<const IPv4Address&>(res->getAddress());
			sprintf(buffer, "Admin connected\t%u.%u.%u.%u", addr.getAddressBytes()[0], addr.getAddressBytes()[1],
					addr.getAddressBytes()[2], addr.getAddressBytes()[3]);
			postServerLogEvent(buffer);

			bool cont = handleControlConnection(*res, sem);

			sprintf(buffer, "Admin disconnected\t%u.%u.%u.%u", addr.getAddressBytes()[0], addr.getAddressBytes()[1],
					addr.getAddressBytes()[2], addr.getAddressBytes()[3]);
			postServerLogEvent(buffer);

			delete res;
			if (!cont) {
				break;
			}
		}
		LOGI() << "Exit control loop";
		MainWorkerThread.post([&] () {
					ControlSocket = nullptr;
					sem.post();
				});
		sem.wait();
		LOGI() << "Exit control thread";
		ControlSemaphore.post();
		return 0;
	});
}

static void startPingerThread() {
	Thread t;
	t.start([] {
		LOGI() << "Start pinger thread";
		Semaphore sem {Semaphore::auto_init {}};
		while(true) {
			unsigned int sleep = 3*60 * 1000;
			while(sleep > 0) {
				sleep = Thread::sleep(sleep);
				if(sleep > 0) {
					/*was interrupted check if exit*/
					bool exitthread = false;
					MainWorkerThread.post([&] () {
								exitthread = AcceptorSocket == nullptr;
								sem.post();
							});
					sem.wait();
					if(exitthread) {
						return 0;
					}
				}
			}
			bool exitthread = false;
			MainWorkerThread.post([&] () {
						exitthread = AcceptorSocket == nullptr;
						if(!exitthread) {
							LOGI() << "Pinging all clients automatically";
							for (auto&& c : ClientConnections.objects()) {
								c.sendPingRequestOrDisconnect();
							}
						}
						sem.post();
					});
			sem.wait();
			if(exitthread) {
				return 0;
			}
		}
		LOGI() << "Exit pinger thread";
		return 0;
	});
}

#if defined(RHFW_PLATFORM_WIN32)
static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
	postServerLogEvent("Signal handler shutdown");
	MainWorkerThread.post([=] {
		LOGI() << "Destroying Acceptor socket";
		if (AcceptorSocket!= nullptr) {
			AcceptorSocket->destroy();
			AcceptorSocket = nullptr;
		}
	});
	return TRUE;
}
static void installSignalHandlers() {
	BOOL res = SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE);
	ASSERT(res) << GetLastError();
}
#elif defined(RHFW_PLATFORM_LINUX)
#include <signal.h>
static void signal_handler(int signum) {
	MainWorkerThread.post([=] {
				LOGI() << "Destroying Acceptor socket";
				if (AcceptorSocket!= nullptr) {
					AcceptorSocket->destroy();
					AcceptorSocket = nullptr;
				}
			});
}
static void signal_handler_INT(int signum) {
	postServerLogEvent("Signal handler shutdown SIGINT");
	signal_handler(signum);
}
static void signal_handler_TERM(int signum) {
	postServerLogEvent("Signal handler shutdown SIGTERM");
	signal_handler(signum);
}
static void signal_handler_HUP(int signum) {
	postServerLogEvent("Signal handler shutdown SIGHUP");
	signal_handler(signum);
}
static void installSignalHandlers() {
	signal(SIGINT, signal_handler_INT);
	signal(SIGTERM, signal_handler_TERM);
	signal(SIGHUP, signal_handler_HUP);
}
#else
static_assert(false, "Unknown platform");
#endif /* signal handlers */

int main(int argc, char* argv[]) {
	LOGTRACE();
	{
		StorageDirectoryDescriptor::InitializePlatformRootDirectory(argc, argv);

		InitSuggestedVersion();
		MainWorkerThread.start();

		StorageFileDescriptor logfilefd { StorageDirectoryDescriptor::Root() + "serverlog.log" };
		LogFile = &logfilefd;
		LogFileOutput = LogFile->createOutput();
		if (LogFileOutput != nullptr) {
			LogFileOutput->setAppend(true);
			LogFileOutput->open();
			LogFileOutput->write("\r\n", 2);
		}

		LOGI() << "Opened log file";
		postServerLogEvent("Starting server");

		MainRandomContext = Resource<RandomContext> { new ResourceBlock(new RandomContext()) };
		MainRandomContext.load();
		MainRandomer = MainRandomContext->createRandomer();
		DataStorage = new LocalSapphireDataStorage();

		LOGI() << "Initializing socket";

		TCPIPv4Address addr;
		addr.setPort(SAPPHIRE_SERVER_PORT_NUMBER);
		TCPIPv4Socket sock(addr);
		sock.initialize();

		AcceptorSocket = &sock;

		installSignalHandlers();

		startControlThread();
		startPingerThread();

		LOGI() << "Listening on address: " << addr;

		while (true) {
			auto* res = sock.accept();
			if (res == nullptr) {
				break;
			}

			char buffer[256];
			auto&& addr = static_cast<const IPv4Address&>(res->getAddress());
			sprintf(buffer, "Accepted connection\t%u.%u.%u.%u", addr.getAddressBytes()[0], addr.getAddressBytes()[1],
					addr.getAddressBytes()[2], addr.getAddressBytes()[3]);
			postServerLogEvent(buffer);

			bool accepted = MainWorkerThread.post([=] {
				ClientConnection* conn = new ClientConnection(res);
				conn->start();
			});
			ASSERT(accepted);
		}
		LOGV() << "Exit main";
		postServerLogEvent("Shutting down server");
		MainWorkerThread.post([=] {
			AcceptorSocket = nullptr;
			if (ControlSocket != nullptr) {
				LOGI() << "Destroying control socket";
				ControlSocket->destroy();
				ControlSocket = nullptr;
			}
			if (ControlConnection != nullptr) {
				LOGI() << "Disconnection control connection";
				ControlConnection->disconnect();
			}
			for (auto&& c : ClientConnections.objects()) {
				postConnectionLogEvent(c.getConnectionIdentifier(), "Shutting down");
				c.stop();
			}
			LOGI() << "Shutdown maintenance opportunity";
			MaintenanceOpportunity();
		});

		ControlSemaphore.wait();
		LOGV() << "Control thread stopped";
		MainWorkerThread.stop();

		LOGV() << "Worker thread stopped";
		delete MainRandomer;
		MainRandomContext.free();
		MainRandomContext = nullptr;
		delete DataStorage;

		writeLogEvent("Successful shutdown");
		delete LogFileOutput;
	}
	LOG_MEMORY_LEAKS();

	return 0;
}
