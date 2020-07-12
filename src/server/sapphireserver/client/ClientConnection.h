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
 * ClientConnection.h
 *
 *  Created on: 2016. okt. 18.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_CLIENT_CLIENTCONNECTION_H_
#define TEST_SAPPHIRE_SERVER_CLIENT_CLIENTCONNECTION_H_

#include <framework/utils/LinkedNode.h>
#include <framework/threading/Semaphore.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>
#include <framework/io/stream/LoopedInputStream.h>
#include <framework/io/stream/OutputStream.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/BufferedInputStream.h>
#include <framework/threading/Mutex.h>
#include <framework/utils/ArrayList.h>

#include <sapphire/community/SapphireUser.h>
#include <sapphire/server/WorkerThread.h>
#include <sapphireserver/storage/SapphireDataStorage.h>
#include <util/RC4Cipher.h>
#include <util/RC4Stream.h>
#include <sapphire/common/RegistrationToken.h>

#include <sapphire/sapphireconstants.h>
#include <sapphireserver/servermain.h>

namespace userapp {
using namespace rhfw;

class ClientConnectionState {
public:
	FixedString userName;
	SapphireDifficulty userDifficultyColor;
	SapphireUUID connectionId;
};

class ClientConnection: public LinkedNode<ClientConnection> {
	class HardwareListener {
	public:
		SapphireUUID hardwareUUID;
		ProgressSynchId nextProgressId = 0;
		SapphireDataStorage::HardwareProgressChangedListener::Listener listener;
	};
	TCPConnection* connection;
	Mutex destroyMutex { Mutex::auto_init { } };

	WorkerThread writerWorker;

	RC4Cipher writeCipher;

	EndianOutputStream<Endianness::Big>* normalOutputStream = nullptr;
	EndianOutputStream<Endianness::Big>* cipherOutputStream = nullptr;
	EndianOutputStream<Endianness::Big>* outputStream = nullptr;

	bool registeredLevelChangedListener = false;
	SapphireDataStorage::LevelChangedListener::Listener levelChangedListener;
	SapphireDataStorage::MessagesChangedListener::Listener messagesChangedListener;
	SapphireDataStorage::HardwareAssociationListener::Listener hardwareAssociationListener;
	ArrayList<HardwareListener> hardwareProgressChangedListeners;
	UserStateListener::Listener userStateListener;

	HardwareListener* getHardwareListener(const SapphireUUID& hardware);
	bool requiresCommunityNotifications = false;

	/**
	 * The UUID the user provides at the start of connection
	 */
	SapphireUUID clientHardwareUUID;
	SapphireUUID clientUUID;
	unsigned int clientAppVersion = 0;
	RegistrationToken registrationToken;
	FixedString userName;
	SapphireDifficulty userDifficultyColor = SapphireDifficulty::Tutorial;

	RegistrationToken pendingRegistrationToken;
	SapphireUUID pendingClientId;

	SapphireUUID connectionIdentifier;

	//index is after the last queried level

	bool terminated = false;

	//uint32 linkIdentifier = 0;
	//uint32 enteredLinkIdentifier = 0;

	MaintenanceListener::Listener maintenanceListener = MaintenanceListener::make_listener([=](bool maintenance) {
		setMaintenanceMode(maintenance);
	});

	unsigned int pingOrDisconnectCounter = 0;

	template<unsigned int Version>
	void clientReadFunction();
	void readThreadFunction(TCPConnection* conn);

	template<typename Writer>
	void writePrivate(Writer&& writer) {
		writerWorker.post([=] {
			writer(outputStream);
		});
	}
	void postUpgradeStream();

	void writeError(SapphireComm cmd, SapphireCommError error);

	template<typename Writer>
	void writeTerminate(Writer&& writer) {
		write([=] (EndianOutputStream<Endianness::Big>& ostream) {
			terminated = true;
			writer(ostream);
			ostream.serialize<SapphireComm>(SapphireComm::Terminate);
		});
	}

	void terminate() {
		write([=] (EndianOutputStream<Endianness::Big>& ostream) {
			terminated = true;
			ostream.serialize<SapphireComm>(SapphireComm::Terminate);
		});
	}

	template<unsigned int Version>
	bool sendLoginResponse();

	void queryAndSendProgressFromRemoteHardware(HardwareListener& listener);
	void sendRemoteHardwareProgress(HardwareListener& listener, ProgressSynchId synchid, const SapphireUUID& hardware,
			const SapphireUUID& level, SapphireLevelProgress progress);

	void installHardwareProgressListener(const SapphireDataStorage::AssociatedHardware* h);
	void removeHardwareProgressListener(const SapphireDataStorage::AssociatedHardware* h);
public:
	ClientConnection(TCPConnection* connection);
	~ClientConnection();

	void start();
	void stop();

	template<typename Writer>
	void write(Writer&& writer) {
		writerWorker.post([=] () mutable {
			if (terminated) {
				return;
			}
			util::forward<Writer>(writer)(*outputStream);
		});
	}
	template<typename Writer>
	void writeNoTerminateCheck(Writer&& writer) {
		writerWorker.post([=] () mutable {
			util::forward<Writer>(writer)(*outputStream);
		});
	}

	virtual ClientConnection* get() override {
		return this;
	}

	void setMaintenanceMode(bool mode);

	const FixedString& getUserName() const {
		return userName;
	}

	const SapphireUUID& getConnectionIdentifier() const {
		return connectionIdentifier;
	}

	ClientConnectionState getState() const {
		ClientConnectionState state;
		state.connectionId = connectionIdentifier;
		state.userDifficultyColor = userDifficultyColor;
		state.userName = userName;
		return state;
	}

	void writeUserStateChanged(const ClientConnectionState& connection, bool online);

	unsigned int getClientAppVersion() const {
		return clientAppVersion;
	}

	void sendPingRequest(uint32 id);
	void sendPingRequestOrDisconnect();
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SERVER_CLIENT_CLIENTCONNECTION_H_ */
