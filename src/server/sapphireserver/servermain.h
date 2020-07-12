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
 * servermain.h
 *
 *  Created on: 2016. okt. 18.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_SERVERMAIN_H_
#define TEST_SAPPHIRE_SERVER_SERVERMAIN_H_

#include <framework/utils/utility.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/FixedString.h>
#include <framework/threading/Mutex.h>
#include <framework/threading/Semaphore.h>
#include <framework/random/RandomContext.h>
#include <framework/random/Randomer.h>
#include <framework/resource/Resource.h>

#include <sapphire/community/SapphireUser.h>
#include <sapphire/server/WorkerThread.h>
#include <sapphire/sapphireconstants.h>

namespace userapp {
using namespace rhfw;

class SapphireDataStorage;
class ClientConnection;
class ClientConnectionState;

enum class UserState {
	CONNECTED,
	AUTHORIZED,
	DISCONNECTED,
};
using UserStateListener = SimpleListener<void(const ClientConnectionState&, UserState)>;
using LogEventListener = SimpleListener<void(const FixedString&)>;
using MaintenanceListener = SimpleListener<void(bool)>;
bool IsMaintenanceMode();
bool IsMaintenanceQueued();
bool IsMaintenanceConnectionDisabled();
void MaintenanceOpportunity();
unsigned int GetSuggestedUpgradeVersion();

extern SapphireDataStorage* DataStorage;
extern LinkedList<ClientConnection, false> ClientConnections;

extern UserStateListener::Events UserStateEvents;
extern LogEventListener::Events LogEvents;
extern MaintenanceListener::Events MaintenanceEvents;

void postLogEvent(const FixedString& log);
void postConnectionLogEvent(const SapphireUUID& connectionid, const FixedString& data);
void postConnectionUserLogEvent(const SapphireUUID& connectionid, const SapphireUUID& uuid, const FixedString& data);

extern WorkerThread MainWorkerThread;

extern Resource<RandomContext> MainRandomContext;
extern Randomer* MainRandomer;

}  // namespace userapp

#endif /* TEST_SAPPHIRE_SERVER_SERVERMAIN_H_ */
