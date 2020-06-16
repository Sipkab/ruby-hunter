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
 * connection_v5.cpp
 *
 *  Created on: 2017. jul. 13.
 *      Author: sipka
 */

#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>

#include <sapphireserver/client/ClientConnection.h>

#include <sapphire/sapphireconstants.h>
#include <sapphireserver/servermain.h>
#include <sapphireserver/storage/SapphireDataStorage.h>
#include <sapphire/level/Level.h>
#include <sapphire/server/SapphireLevelDetails.h>
#include <sapphire/community/SapphireDiscussionMessage.h>
#include <sapphire/common/RegistrationToken.h>

#include <gen/log.h>
#include <gen/types.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <sapphireserver/client/connection_common.h>

#define CONNECTION_VERSION 5

namespace userapp {
using namespace rhfw;

void ClientConnection::installHardwareProgressListener(const SapphireDataStorage::AssociatedHardware* h) {
	SapphireUUID huuid = h->hardwareUUID;
	ProgressSynchId synchid = h->synchronizedProgressCount;
	writerWorker.post([=] {
		LOGI()<< "Install hardware progress listener: "<< huuid.asString();
		HardwareListener* listener = getHardwareListener(huuid);
		if (listener->listener != nullptr) {
			LOGI() << "Hardware progress changed listener already installed " << huuid.asString();
			return;
		}
		listener->nextProgressId = synchid;
		listener->listener = SapphireDataStorage::HardwareProgressChangedListener::make_listener(
				[=](ProgressSynchId synchid, const SapphireUUID& leveluuid, SapphireLevelProgress progress) {
					writerWorker.post([=] {
								sendRemoteHardwareProgress(*listener, synchid, huuid, leveluuid, progress);
							});
				});
		auto lerror = DataStorage->addHardwareProgressChangedListener(huuid, listener->listener);
		ASSERT(lerror == SapphireStorageError::SUCCESS);

		queryAndSendProgressFromRemoteHardware(*listener);
	});
}
void ClientConnection::removeHardwareProgressListener(const SapphireDataStorage::AssociatedHardware* h) {
	SapphireUUID huuid = h->hardwareUUID;
	writerWorker.post([=] {
		LOGI()<< "Remove hardware progress listener: "<< huuid.asString();
		HardwareListener* listener = getHardwareListener(huuid);
		if (listener->listener == nullptr) {
			LOGI() << "Hardware progress changed listener already removed " << huuid.asString();
			return;
		}
		auto lerror = DataStorage->removeHardwareProgressChangedListener(huuid, listener->listener);
		ASSERT(lerror == SapphireStorageError::SUCCESS);
		listener->listener = nullptr;
	});
}
ClientConnection::HardwareListener* ClientConnection::getHardwareListener(const SapphireUUID& hardware) {
	for (auto&& h : hardwareProgressChangedListeners) {
		if (h->hardwareUUID == hardware) {
			return h;
		}
	}
	auto* result = new HardwareListener();
	result->hardwareUUID = hardware;
	hardwareProgressChangedListeners.add(result);
	return result;
}

template<>
bool ClientConnection::sendLoginResponse<CONNECTION_VERSION>() {
	ProgressSynchId outprogressid;
	auto error = DataStorage->getProgressId(clientHardwareUUID, &outprogressid);
	if (error == SapphireStorageError::SUCCESS) {
		hardwareAssociationListener = SapphireDataStorage::HardwareAssociationListener::make_listener(
				[=](const SapphireDataStorage::AssociatedHardware& h1, const SapphireDataStorage::AssociatedHardware& h2, bool alive) {
					const SapphireDataStorage::AssociatedHardware* otherh;
					if(h1.hardwareUUID == clientHardwareUUID) {
						otherh = &h2;
					} else if(h2.hardwareUUID == clientHardwareUUID) {
						otherh = &h1;
					} else {
						return;
					}
					LOGI() << "HardwareAssociationListener " << h1.hardwareUUID.asString() << " - " << h2.hardwareUUID.asString();
					if(alive) {
						/* register listeners, and start sending*/
						installHardwareProgressListener(otherh);
					} else {
						/* remove listener*/
						removeHardwareProgressListener(otherh);
					}
				});
		auto haerror = DataStorage->addHardwareAssociationListener(hardwareAssociationListener);
		ASSERT(haerror == SapphireStorageError::SUCCESS);

		ArrayList<SapphireDataStorage::AssociatedHardware> hardwares;
		auto error = DataStorage->queryAssociatedHardwares(clientHardwareUUID, hardwares);
		switch (error) {
			case SapphireStorageError::SUCCESS: {
				for (auto&& h : hardwares) {
					if (h->hardwareUUID == clientHardwareUUID) {
						continue;
					}
					installHardwareProgressListener(h);
				}
				break;
			}
			default: {
				return false;
			}
		}
		write([=](EndianOutputStream<Endianness::Big>& ostream) {
			LOGTRACE() << "Login response with progress id: " << outprogressid;
			ostream.serialize<SapphireComm>(SapphireComm::Login);
			ostream.serialize<ProgressSynchId>(outprogressid);
		});

		return true;
	}
	return false;
}

template<>
void ClientConnection::clientReadFunction<CONNECTION_VERSION>() {
	Semaphore sem { Semaphore::auto_init { } };
	MainWorkerThread.post([=] {
		ClientConnectionState state = getState();
		for (auto&& l : UserStateEvents.foreach()) {
			l(state, UserState::CONNECTED);
		}
	});
	RC4Cipher cipher;

	auto istream = LoopedInputStream::wrap(BufferedInputStream::wrap(*connection));
	auto eistream = EndianInputStream<Endianness::Big>::wrap(istream);
	auto upgistream = EndianInputStream<Endianness::Big>::wrap(RC4InputStream::wrap(istream, cipher));

	EndianInputStream<Endianness::Big>* stream = &eistream;
	while (true) {
		SapphireComm cmd = (SapphireComm) 0;
		if (!stream->deserialize<SapphireComm>(cmd)) {
			LOGI()<< "Failed to read CMD " << (unsigned int) cmd;
			postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tCommand");
			goto exit_loop;
		}
		LOGI()<< "Command: " << cmd;
		if (terminated && cmd != SapphireComm::Terminate && cmd != SapphireComm::TerminateOk) {
			//terminate commands is okay
			writeError(cmd, SapphireCommError::Terminated);
			postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tTerminated");
			break;
		}
		switch (cmd) {
			case SapphireComm::UpgradeStream: {
				uint8 servrdm[sizeof(SAPPHIRE_COMM_PV_KEY)];
				if (stream->read(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY)) != sizeof(SAPPHIRE_COMM_PV_KEY)) {
					LOGI()<< "Failed to read 256 user random";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tUpgradeStream");
					goto exit_loop;
				}
				for (int i = 0; i < sizeof(SAPPHIRE_COMM_PV_KEY); ++i) {
					servrdm[i] ^= SAPPHIRE_COMM_PV_KEY[i];
				}
				cipher.initCipher(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY));

				stream = &upgistream;
				LOGI()<< "Read stream upgraded";

				postUpgradeStream();
				break;
			}
			case SapphireComm::Login: {
				SapphireUUID userid;
				if (!stream->deserialize<SapphireUUID>(userid)) {
					LOGI()<< "Failed to read userid";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tLogin");
					goto exit_loop;
				}
				if (IsMaintenanceConnectionDisabled()) {
					writeTerminate([=](EndianOutputStream<Endianness::Big>& ostream) {
						ostream.serialize<SapphireComm>(SapphireComm::Information);
						ostream.serialize<SapphireCommunityInformation>(SapphireCommunityInformation::Maintenance);
					});
					break;
				}
				if (!userid) {
					writeError(cmd, SapphireCommError::InvalidUserId);
					break;
				}

				LOGI()<< "Login with uuid: " << userid.asString();

				RegistrationToken token;
				FixedString username;
				SapphireDifficulty usercolor;
				SapphireStorageError loginerror = DataStorage->loginUser(userid, &token, &username, &usercolor);
				switch (loginerror) {
					case SapphireStorageError::USER_NOT_FOUND: {
						postConnectionUserLogEvent(connectionIdentifier, userid, "Login\tUSER_NOT_FOUND");
						LOGI()<< "Send registration token";
						if (MainRandomer->read(pendingRegistrationToken.data, 256) == 256) {
							pendingClientId = userid;
							postUpgradeStream();
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::RegistrationToken);
								ostream.serialize<RegistrationToken>(token);
							});
						} else {
							writeError(cmd, SapphireCommError::ServerError);
						}
						break;
					}
					case SapphireStorageError::SUCCESS: {
						this->clientUUID = userid;
						this->registrationToken = token;

						MainWorkerThread.post([=] {
							this->userName = util::move(username);
							this->userDifficultyColor = usercolor;
							if(this->userName != nullptr) {
								ClientConnectionState state = getState();
								for (auto&& l : UserStateEvents.foreach()) {
									l(state, UserState::AUTHORIZED);
								}
							}
						});

						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Login\tSUCCESS");
						//successful login, no response required

						if (IsMaintenanceConnectionDisabled()) {
							writeTerminate([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::Information);
								ostream.serialize<SapphireCommunityInformation>(SapphireCommunityInformation::Maintenance);
							});
						} else {
							if (!sendLoginResponse<CONNECTION_VERSION>()) {
								writeError(cmd, SapphireCommError::ServerError);
							}
						}
						break;
					}
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Login\tServerError");
						writeError(cmd, SapphireCommError::ServerError);
						break;
					}
				}

				break;
			}
			case SapphireComm::RegistrationTokenReceived: {
				if (!pendingClientId) {
					writeError(cmd, SapphireCommError::InvalidOperation);
					break;
				}
				if (!pendingRegistrationToken) {
					writeError(cmd, SapphireCommError::InvalidOperation);
					break;
				}
				SapphireStorageError registererror = DataStorage->registerUser(pendingClientId, pendingRegistrationToken);
				switch (registererror) {
					case SapphireStorageError::SUCCESS: {
						postConnectionUserLogEvent(connectionIdentifier, pendingClientId, "RegistrationTokenReceived\tSUCCESS");

						clientUUID = pendingClientId;
						registrationToken = pendingRegistrationToken;
						pendingClientId = SapphireUUID { };
						pendingRegistrationToken = RegistrationToken { };

						if (IsMaintenanceConnectionDisabled()) {
							writeTerminate([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::Information);
								ostream.serialize<SapphireCommunityInformation>(SapphireCommunityInformation::Maintenance);
							});
						} else {
							if (!sendLoginResponse<CONNECTION_VERSION>()) {
								writeError(cmd, SapphireCommError::ServerError);
							}
						}
						break;
					}
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "RegistrationTokenReceived\tServerError");
						writeError(cmd, SapphireCommError::ServerError);
						break;
					}
				}
				break;
			}
			case SapphireComm::UpdatePlayerData: {
				CHECK_CLIENT_ID();

				FixedString name;
				SapphireDifficulty diffcolor;
				if (!stream->deserialize<SafeFixedString<SAPPHIRE_USERNAME_MAX_LEN>>(name)
						|| !stream->deserialize<SapphireDifficulty>(diffcolor)) {
					LOGI()<< "Failed to read userid, name";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tUpdatePlayerData");
					goto exit_loop;
				}
				if (!ValidateName(name)) {
					writeError(cmd, SapphireCommError::InvalidName);
					break;
				}
				DataStorage->updateUserInfo(clientUUID, name, diffcolor);
				postConnectionUserLogEvent(connectionIdentifier, clientUUID, FixedString { "UpdatePlayerData\t" } + name);
				MainWorkerThread.post([=] {
					this->userName = util::move(name);
					this->userDifficultyColor = diffcolor;
					ClientConnectionState state = getState();
					for (auto&& l : UserStateEvents.foreach()) {
						l(state, UserState::AUTHORIZED);
					}
				});

				if (clientAppVersion < GetSuggestedUpgradeVersion()) {
					write([=](EndianOutputStream<Endianness::Big>& ostream) {
						ostream.serialize<SapphireComm>(SapphireComm::Information);
						ostream.serialize<SapphireCommunityInformation>(SapphireCommunityInformation::NewVersionRequired);
					});
				}
				break;
			}
			case SapphireComm::UploadLevel: {
				CHECK_CLIENT_ID();

				LOGI()<< "Uploading level...";
				Level level;
				if (!level.loadLevel(*stream)) {
					writeError(cmd, SapphireCommError::LevelReadFailed);
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tUploadLevel");
					goto exit_loop;
				}
				if (level.getInfo().author.getUserName().length() == 0) {
					ClientConnectionState state;
					MainWorkerThread.post([&] {
						state = getState();
						sem.post();
					});
					sem.wait();
					level.getInfo().author.getUserName() = state.userName;
				}
				level.getInfo().nonModifyAbleFlag = true;
				LOGI()<< "Received level with UUID: " << level.getInfo().uuid.asString();
				auto storeerror = DataStorage->saveLevel(level, this->clientUUID);
				switch (storeerror) {
					case SapphireStorageError::SUCCESS: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "UploadLevel\tSUCCESS\t" } + level.getInfo().uuid.asString());
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::UploadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							ostream.serialize<SapphireUUID>(level.getInfo().uuid);
						});
						break;
					}
					case SapphireStorageError::LEVEL_SAVE_SUCCESS_DEMO_REMOVED: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, FixedString {
								"UploadLevel\tLEVEL_SAVE_SUCCESS_DEMO_REMOVED\t" } + level.getInfo().uuid.asString());
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::UploadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::LevelDemoRemoved);
							ostream.serialize<SapphireUUID>(level.getInfo().uuid);
						});
						break;
					}
					case SapphireStorageError::LEVEL_ALREADY_EXISTS: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "UploadLevel\tLEVEL_ALREADY_EXISTS\t" } + level.getInfo().uuid.asString());
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::UploadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::LevelAlreadyExists);
							ostream.serialize<SapphireUUID>(level.getInfo().uuid);
						});
						break;
					}
					case SapphireStorageError::DEMO_INCORRECT: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "UploadLevel\tDEMO_INCORRECT\t" } + level.getInfo().uuid.asString());
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::UploadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::LevelInvalidDemo);
							ostream.serialize<SapphireUUID>(level.getInfo().uuid);
						});
						break;
					}
					case SapphireStorageError::NO_DEMO: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "UploadLevel\tNO_DEMO\t" } + level.getInfo().uuid.asString());
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::UploadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::LevelNoDemo);
							ostream.serialize<SapphireUUID>(level.getInfo().uuid);
						});
						break;
					}
						//TODO
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "UploadLevel\tServerError\t" } + level.getInfo().uuid.asString());
						LOGE()<< "Datastorage save level error: " << (int) storeerror;
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::UploadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<SapphireUUID>(level.getInfo().uuid);
						});
						break;
					}
				}
				break;
			}
			case SapphireComm::GetLevels: {
				CHECK_CLIENT_ID();

				uint32 start;
				if (!stream->deserialize<uint32>(start)) {
					LOGI()<< "Failed to read download start";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tGetLevels");
					goto exit_loop;
				}
				LocalArray<SapphireLevelDetails, 64> details;

				unsigned int outcount;
				auto storeerror = DataStorage->queryLevels(details, 64, start, &outcount, clientUUID);
				switch (storeerror) {
					case SapphireStorageError::SUCCESS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::GetLevels);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							ostream.serialize<uint32>(start);
							ostream.serialize<uint32>(outcount);
							for (unsigned int i = 0; i < outcount; ++i) {
								ostream.serialize<SapphireLevelDetails>(details[i]);
							}
						});
						break;
					}
					case SapphireStorageError::OUT_OF_BOUNDS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::GetLevels);
							ostream.serialize<SapphireCommError>(SapphireCommError::ValueOutOfBounds);
							ostream.serialize<uint32>(start);
						});
						break;
					}
					default: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::GetLevels);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<uint32>(start);
						});
						break;
					}
				}
				if (!registeredLevelChangedListener) {
					levelChangedListener = SapphireDataStorage::LevelChangedListener::make_listener(
							[=](unsigned int index, LevelChangeInfo info) {
								switch (info) {
									case LevelChangeInfo::REMOVED: {
										write([=](EndianOutputStream<Endianness::Big>& ostream) {
													ostream.serialize<SapphireComm>(SapphireComm::LevelRemoved);
													ostream.serialize<uint32>(index);
												});
										break;
									}
									case LevelChangeInfo::ADDED:
									case LevelChangeInfo::RATING_CHANGED: {
										write([=](EndianOutputStream<Endianness::Big>& ostream) {
													ostream.serialize<SapphireComm>(SapphireComm::LevelDetailsChanged);
													ostream.serialize<uint32>(index);
												});
										break;
									}
									default: {
										break;
									}
								}
							});
					auto storeerror = DataStorage->addLevelChangedListener(levelChangedListener);
					if (storeerror == SapphireStorageError::SUCCESS) {
						registeredLevelChangedListener = true;
					} else {
						levelChangedListener = nullptr;
					}
				}
				break;
			}
			case SapphireComm::QuerySingleLevel: {
				CHECK_CLIENT_ID();

				uint32 index;
				if (!stream->deserialize<uint32>(index)) {
					LOGI()<< "Failed to read query index";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tQuerySingleLevel");
					goto exit_loop;
				}
				SapphireLevelDetails details;
				unsigned int outcount;
				auto storeerror = DataStorage->queryLevels(&details, 1, index, &outcount, clientUUID);
				if (outcount == 1) {
					switch (storeerror) {
						case SapphireStorageError::SUCCESS: {
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::QuerySingleLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
								ostream.serialize<uint32>(index);
								ostream.serialize<SapphireLevelDetails>(details);
							});
							break;
						}
						default: {
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::QuerySingleLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
								ostream.serialize<uint32>(index);
							});
							break;
						}
					}
				} else {
					write([=](EndianOutputStream<Endianness::Big>& ostream) {
						ostream.serialize<SapphireComm>(SapphireComm::QuerySingleLevel);
						ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
						ostream.serialize<uint32>(index);
					});
				}
				break;
			}
			case SapphireComm::DownloadLevel: {
				CHECK_CLIENT_ID();

				SapphireUUID uuid;
				if (!stream->deserialize<SapphireUUID>(uuid)) {
					LOGI()<< "Failed to read download leveluuid";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tDownloadLevel");
					goto exit_loop;
				}
				bool builtinlevel;
				Level outlevel;
				auto storeerror = DataStorage->getLevel(uuid, &outlevel, &builtinlevel);
				switch (storeerror) {
					case SapphireStorageError::SUCCESS: {
						if (builtinlevel) {
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::DownloadLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::NotFound);
								ostream.serialize<SapphireUUID>(uuid);
							});
						} else if (clientAppVersion < outlevel.getLevelVersion()) {
							postConnectionUserLogEvent(connectionIdentifier, clientUUID,
									FixedString { "DownloadLevel\tNEWVERSION\t" } + outlevel.getInfo().uuid.asString());
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::DownloadLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::NewerVersion);
								ostream.serialize<SapphireUUID>(uuid);
							});
						} else {
							postConnectionUserLogEvent(connectionIdentifier, clientUUID,
									FixedString { "DownloadLevel\tSUCCESS\t" } + outlevel.getInfo().uuid.asString());
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::DownloadLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
								outlevel.saveLevel(ostream, true);
							});
						}
						break;
					}
					case SapphireStorageError::LEVEL_NOT_FOUND: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "DownloadLevel\tLevelNotFound\t" } + uuid.asString());
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::DownloadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::NotFound);
							ostream.serialize<SapphireUUID>(uuid);
						});
						break;
					}
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "DownloadLevel\tServerError\t" } + uuid.asString());
						LOGI()<< "Storage error: " << (int) storeerror;
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::DownloadLevel);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<SapphireUUID>(uuid);
						});
						break;
					}
				}

				break;
			}
			case SapphireComm::RateLevel: {
				CHECK_CLIENT_ID();

				SapphireUUID uuid;
				uint8 rating;
				if (!stream->deserialize<SapphireUUID>(uuid) || !stream->deserialize<uint8>(rating)) {
					LOGI()<< "Failed to read rate level uuid or rating";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tRateLevel");
					goto exit_loop;
				}
				LOGI()<< "Rate level: " << uuid.asString() << " for: " << (unsigned int) rating;
				if (rating < 1 || rating > 5) {
					postConnectionUserLogEvent(connectionIdentifier, clientUUID,
							FixedString { "RateLevel\tInvalidRating\t" } + uuid.asString());
					write([=](EndianOutputStream<Endianness::Big>& ostream) {
						ostream.serialize<SapphireComm>(SapphireComm::RateLevel);
						ostream.serialize<SapphireCommError>(SapphireCommError::InvalidRating);
						ostream.serialize<SapphireUUID>(uuid);
					});
				} else {
					auto storeerror = DataStorage->rateLevel(clientUUID, uuid, rating);
					switch (storeerror) {
						case SapphireStorageError::SUCCESS: {
							char r[] { '\t', (char) ('0' + (char) rating), 0 };
							postConnectionUserLogEvent(connectionIdentifier, clientUUID,
									FixedString { "RateLevel\tSUCCESS\t" } + uuid.asString() + r);
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::RateLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
								ostream.serialize<SapphireUUID>(uuid);
							});
							break;
						}
						case SapphireStorageError::INVALID_RATING: {
							char r[] { '\t', (char) ('0' + (char) rating), 0 };
							postConnectionUserLogEvent(connectionIdentifier, clientUUID,
									FixedString { "RateLevel\tINVALID_RATING\t" } + uuid.asString() + r);
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::RateLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::InvalidRating);
								ostream.serialize<SapphireUUID>(uuid);
							});
							break;
						}
						default: {
							postConnectionUserLogEvent(connectionIdentifier, clientUUID,
									FixedString { "RateLevel\tServerError\t" } + uuid.asString());
							LOGI()<< "Storage error: " << (int) storeerror;
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::RateLevel);
								ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
								ostream.serialize<SapphireUUID>(uuid);
							});
							break;
						}
					}
				}
				break;
			}
			case SapphireComm::QueryMessages: {
				CHECK_CLIENT_ID();

				uint32 start;
				uint32 count;
				if (!stream->deserialize<uint32>(start) || !stream->deserialize<uint32>(count)) {
					LOGI()<< "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tQueryMessages");
					goto exit_loop;
				}
				unsigned int inoutstart = start;
				unsigned int outcount;
				LocalArray<SapphireDiscussionMessage, 64> messages;
				auto storeerror = DataStorage->queryMessages(messages, count > 64 ? 64 : count, &inoutstart, &outcount);
				LOGI()<< "Query messages: " << start << " (" << count << ") result: " << inoutstart << " (" << outcount << ")";
				switch (storeerror) {
					case SapphireStorageError::OUT_OF_BOUNDS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::QueryMessages);
							ostream.serialize<SapphireCommError>(SapphireCommError::ValueOutOfBounds);
							ostream.serialize<uint32>(start);
							ostream.serialize<uint32>(count);
							ostream.serialize<uint32>(inoutstart);
							ostream.serialize<uint32>(outcount);
						});
						break;
					}
					case SapphireStorageError::SUCCESS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::QueryMessages);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							ostream.serialize<uint32>(start);
							ostream.serialize<uint32>(count);
							ostream.serialize<uint32>(inoutstart);
							ostream.serialize<uint32>(outcount);
							for (unsigned int i = 0; i < outcount; ++i) {
								ostream.serialize<SapphireDiscussionMessage>(messages[i]);
							}
						});
						break;
					}
					default: {
						LOGI()<< "Storage error: " << (int) storeerror;
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
									ostream.serialize<SapphireComm>(SapphireComm::QueryMessages);
									ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
									ostream.serialize<uint32>(start);
									ostream.serialize<uint32>(count);
								});
						break;
					}
				}
				break;
			}
			case SapphireComm::SendMessage: {
				CHECK_CLIENT_ID();

				FixedString message;
				uint32 userid;
				if (!stream->deserialize<SafeFixedString<SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN>>(message)
						|| !stream->deserialize<uint32>(userid)) {
					LOGI()<< "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tSendMessage");
					goto exit_loop;
				}
				if (!ValidateMessage(message)) {
					write([=](EndianOutputStream<Endianness::Big>& ostream) {
						ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
						ostream.serialize<SapphireCommError>(SapphireCommError::InvalidOperation);
						ostream.serialize<uint32>(userid);
					});
					break;
				}
				//TODO check for spam
				ClientConnectionState state;
				MainWorkerThread.post([&] {
					state = getState();
					sem.post();
				});
				sem.wait();
				if (state.userName.length() == 0) {
					write([=](EndianOutputStream<Endianness::Big>& ostream) {
						ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
						ostream.serialize<SapphireCommError>(SapphireCommError::InvalidName);
						ostream.serialize<uint32>(userid);
					});
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, FixedString { "SendMessage\tInvalidName\t" } + message);
					break;
				}
				auto storeerror = DataStorage->appendMessage(clientUUID, message);
				switch (storeerror) {
					case SapphireStorageError::SUCCESS: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, FixedString { "SendMessage\tSUCCESS\t" } + message);
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							ostream.serialize<uint32>(userid);
						});
						break;
					}
					case SapphireStorageError::NULLPOINTER: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "SendMessage\tNULLPOINTER\t" } + message);
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<uint32>(userid);
						});
						break;
					}
					case SapphireStorageError::INVALID_USER_UUID: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "SendMessage\tINVALID_USER_UUID\t" } + message);
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<uint32>(userid);
						});
						break;
					}
					case SapphireStorageError::OUT_OF_BOUNDS: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "SendMessage\tOUT_OF_BOUNDS\t" } + message);
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<uint32>(userid);
						});
						break;
					}
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "SendMessage\tServerError\t" } + message);
						LOGI()<< "Storage error: " << (int) storeerror;
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::SendMessage);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
							ostream.serialize<uint32>(userid);
						});
						break;
					}
				}
				break;
			}
			case SapphireComm::ReportLevel: {
				SapphireUUID leveluuid;
				FixedString reason;
				if (!stream->deserialize<SapphireUUID>(leveluuid)
						|| !stream->deserialize<SafeFixedString<SAPPHIRE_REPORT_REASON_MAX_LEN>>(reason)) {
					LOGI()<< "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tReportLevel");
					goto exit_loop;
				}
				if (!ValidateMessage(reason)) {
					writeError(cmd, SapphireCommError::InvalidOperation);
					break;
				}
				postConnectionUserLogEvent(connectionIdentifier, clientUUID,
						FixedString { "ReportLevel\t" } + leveluuid.asString() + "\t" + reason);
				break;
			}
			case SapphireComm::CommunityNotifications: {
				CHECK_CLIENT_ID();

				bool need;
				if (!stream->deserialize<bool>(need)) {
					LOGI()<< "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tCommunityNotifications");
					goto exit_loop;
				}
				if (need != requiresCommunityNotifications) {
					requiresCommunityNotifications = need;
					if (need) {
						messagesChangedListener = SapphireDataStorage::MessagesChangedListener::make_listener(
								[=](unsigned int startindex, unsigned int count) {
									write([=](EndianOutputStream<Endianness::Big>& ostream) {
												ostream.serialize<SapphireComm>(SapphireComm::DiscussionMessagesChanged);
												ostream.serialize<uint32>(startindex);
												ostream.serialize<uint32>(count);
											});
								});
						DataStorage->addMessagesChangedListener(messagesChangedListener);
						MainWorkerThread.post(
								[=] {
									userStateListener = UserStateListener::make_listener([=](const ClientConnectionState& conn, UserState state) {
												if(conn.userName.length() > 0 && (state == UserState::AUTHORIZED || state == UserState::DISCONNECTED)) {
													writeUserStateChanged(conn, state == UserState::AUTHORIZED);
												}
											});
									UserStateEvents += userStateListener;

									for (auto&& c : ClientConnections.objects()) {
										ClientConnectionState state = c.getState();
										if(state.userName.length() > 0) {
											writeUserStateChanged(state, true);
										}
									}
								});
					} else {
						if (messagesChangedListener != nullptr) {
							DataStorage->removeMessagesChangedListener(messagesChangedListener);
							messagesChangedListener = nullptr;
						}
						MainWorkerThread.post([=] {
							userStateListener = nullptr;
						});
					}
				}
				break;
			}
			case SapphireComm::LevelProgress: {
				CHECK_CLIENT_ID();

				SapphireLevelCommProgress progress;
				ProgressSynchId progressid;
				SapphireUUID leveluuid;
				FixedString steps;
				uint32 randomseed;
				if (!stream->deserialize<SapphireLevelCommProgress>(progress) || !stream->deserialize<uint64>(progressid)
						|| !stream->deserialize<SapphireUUID>(leveluuid)
						|| !stream->deserialize<SafeFixedString<SAPPHIRE_DEMO_MAX_LEN>>(steps)
						|| !stream->deserialize<uint32>(randomseed)) {
					LOGI()<< "Failed to read";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tLevelProgress");
					goto exit_loop;
				}
				LOGI()<< cmd << " - " << leveluuid.asString() << " " << progress;
				auto appenderror = DataStorage->appendLevelStatistics(leveluuid, clientUUID, steps, randomseed);
				//ignore append error as the level might not exist on server but only on client
				auto progresserror = DataStorage->setLevelProgress(&progressid, clientHardwareUUID, leveluuid,
						sapphireLevelCommProgressToSapphireLevelProgress(progress));
				switch (progresserror) {
					case SapphireStorageError::SUCCESS: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tSUCCESS");
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
						});
						break;
					}
					case SapphireStorageError::USER_NOT_FOUND: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tUSER_NOT_FOUND");
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireCommError>(SapphireCommError::InvalidUserId);
						});
						break;
					}
					case SapphireStorageError::PROGRESS_UNCHANGED: {
						//success, but progress didnt change
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tPROGRESS_UNCHANGED");
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
						});
						break;
					}
					case SapphireStorageError::LEVEL_NOT_FOUND: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tLEVEL_NOT_FOUND");
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireCommError>(SapphireCommError::LevelReadFailed);
						});
						break;
					}
					case SapphireStorageError::INVALID_PROGRESSID: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tINVALID_PROGRESSID");
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireCommError>(SapphireCommError::ValueOutOfBounds);
							ostream.serialize<ProgressSynchId>(progressid);
						});
						break;
					}
					case SapphireStorageError::LEVEL_NOT_SUCCESSFULLY_FINISHED: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tLEVEL_NOT_SUCCESSFULLY_FINISHED");
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireCommError>(SapphireCommError::LevelInvalidDemo);
						});
						break;
					}
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "LevelProgress\tServerError");
						writeError(cmd, SapphireCommError::ServerError);
						break;
					}
				}

				break;
			}
			case SapphireComm::GetStatistics: {
				CHECK_CLIENT_ID();

				SapphireUUID leveluuid;
				if (!stream->deserialize<SapphireUUID>(leveluuid)) {
					LOGI()<< "Failed to read";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tGetStatistics");
					goto exit_loop;
				}
				LevelStatistics outstats;
				unsigned int outplaycount;
				auto error = DataStorage->getLevelStatistics(leveluuid, &outstats, &outplaycount);
				switch (error) {
					case SapphireStorageError::SUCCESS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							outstats.serialize<1>(ostream, outplaycount);
						});
						break;
					}
					case SapphireStorageError::STATS_NOT_FOUND: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoStatsYet);
						});
						break;
					}
					default: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
						});
						break;
					}
				}
				break;
			}
//			case SapphireComm::LinkAccRequest: {
//				CHECK_CLIENT_ID();
//
//				uint8 randoms[SAPPHIRE_LINK_NUMBER_LENGTH];
//				if (MainRandomer->read(randoms, SAPPHIRE_LINK_NUMBER_LENGTH) != SAPPHIRE_LINK_NUMBER_LENGTH) {
//					writeError(cmd, SapphireCommError::ServerError);
//				} else {
//					uint32 identifier = 0;
//					for (unsigned int i = 0; i < SAPPHIRE_LINK_NUMBER_LENGTH; ++i) {
//						// every number is between 1-9
//						identifier = identifier * 10 + (randoms[i] % 9) + 1;
//					}
//					MainWorkerThread.post([=] {
//						linkIdentifier = identifier;
//					});
//					ASSERT(identifier != 0);
//					write([=](EndianOutputStream<Endianness::Big>& ostream) {
//						ostream.serialize<SapphireComm>(SapphireComm::LinkAccRequest);
//						ostream.serialize<uint32>(identifier);
//					});
//				}
//				break;
//			}
//			case SapphireComm::LinkAccIdentifier: {
//				CHECK_CLIENT_ID();
//
//				uint32 identifier;
//				if (!stream->deserialize<uint32>(identifier)) {
//					LOGI()<< "Read failure";
//					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tLinkAccIdentifier");
//					goto exit_loop;
//				}
//				if (identifier == 0) {
//					writeError(cmd, SapphireCommError::ValueOutOfBounds);
//				}
//				MainWorkerThread.post([=] {
//					if(enteredLinkIdentifier == identifier) {
//						return;
//					}
//					enteredLinkIdentifier = identifier;
//					for (auto&& c : ClientConnections.objects()) {
//						if(c.enteredLinkIdentifier == linkIdentifier && enteredLinkIdentifier == c.linkIdentifier) {
//							this->linkIdentifier = 0;
//							this->enteredLinkIdentifier = 0;
//							c.linkIdentifier = 0;
//							c.enteredLinkIdentifier = 0;
//
//							if(c.clientHardwareUUID == this->clientHardwareUUID) {
//								auto&& writer = [=](EndianOutputStream<Endianness::Big>& ostream) {
//									ostream.serialize<SapphireComm>(SapphireComm::LinkResult);
//									ostream.serialize<SapphireCommError>(SapphireCommError::SameHardware);
//								};
//								write(writer);
//								c.write(writer);
//								break;
//							}
//
//							auto error = DataStorage->createHardwareAssociation(clientHardwareUUID, c.clientHardwareUUID);
//							switch (error) {
//								case SapphireStorageError::SUCCESS: {
//									/*progresses will start to send via listeners*/
//									auto&& writer = [=](EndianOutputStream<Endianness::Big>& ostream) {
//										ostream.serialize<SapphireComm>(SapphireComm::LinkResult);
//										ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
//									};
//									write(writer);
//									c.write(writer);
//									break;
//								}
//								case SapphireStorageError::HARDWARE_ALREADY_ASSOCIATED: {
//									auto&& writer = [=](EndianOutputStream<Endianness::Big>& ostream) {
//										ostream.serialize<SapphireComm>(SapphireComm::LinkResult);
//										ostream.serialize<SapphireCommError>(SapphireCommError::AlreadyLinked);
//									};
//									write(writer);
//									c.write(writer);
//									break;
//								}
//								default: {
//									LOGE() << "Create hardware association error: " << (uint32) error;
//									auto&& writer = [=](EndianOutputStream<Endianness::Big>& ostream) {
//										ostream.serialize<SapphireComm>(SapphireComm::LinkResult);
//										ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
//									};
//									write(writer);
//									c.write(writer);
//									break;
//								}
//							}
//							break;
//						}
//					}
//				});
//				break;
//			}
			case SapphireComm::GetLeaderboard: {
				CHECK_CLIENT_ID();

				SapphireUUID leveluuid;
				SapphireLeaderboards leaderboard;
				if (!stream->deserialize<SapphireUUID>(leveluuid) || !stream->deserialize<SapphireLeaderboards>(leaderboard)) {
					LOGI()<< "Failed to read";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tGetLeaderboard");
					goto exit_loop;
				}
				ArrayList<SapphireDataStorage::LeaderboardEntry> outentries;
				int outuserindex;
				uint32 outuserscore;
				int32 outuserposition;
				PlayerDemoId outuserdemoid;
				uint32 outtotalcount;
				auto error = DataStorage->getLeaderboard(leveluuid, clientUUID, leaderboard, 50, &outentries, &outuserindex, &outuserscore,
						&outuserposition, &outuserdemoid, &outtotalcount);

				switch (error) {
					case SapphireStorageError::SUCCESS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) mutable {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireLeaderboards>(leaderboard);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							ostream.serialize<uint32>(outentries.size());
							ostream.serialize<int32>(outuserindex);
							ostream.serialize<uint32>(outuserscore);
							ostream.serialize<int32>(outuserposition);
							ostream.serialize<PlayerDemoId>(outuserdemoid);
							ostream.serialize<uint32>(outtotalcount);
							for (auto&& e : outentries) {
								ostream.serialize<uint32>(e->score);
								ostream.serialize<PlayerDemoId>(e->demoId);
								ostream.serialize<FixedString>(e->userName);
							}
						});
						break;
					}
					case SapphireStorageError::LEADERBOARD_NOT_FOUND: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireLeaderboards>(leaderboard);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoStatsYet);
						});
						break;
					}
					default: {
						break;
					}
				}
				break;
			}
			case SapphireComm::GetPlayerDemo: {
				SapphireUUID leveluuid;
				PlayerDemoId demoid;
				if (!stream->deserialize<SapphireUUID>(leveluuid) || !stream->deserialize<PlayerDemoId>(demoid)) {
					LOGI()<< "Failed to read";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tGetPlayerDemo");
					goto exit_loop;
				}
				FixedString steps;
				uint32 randomseed;
				auto error = DataStorage->getPlayerDemo(leveluuid, demoid, &steps, &randomseed);
				switch (error) {
					case SapphireStorageError::SUCCESS: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<PlayerDemoId>(demoid);
							ostream.serialize<SapphireCommError>(SapphireCommError::NoError);
							ostream.serialize<uint32>(randomseed);
							ostream.serialize<FixedString>(steps);
						});
						break;
					}
					case SapphireStorageError::DEMO_NOT_FOUND: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireCommError>(SapphireCommError::NotFound);
						});
						break;
					}
					default: {
						write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(cmd);
							ostream.serialize<SapphireUUID>(leveluuid);
							ostream.serialize<SapphireCommError>(SapphireCommError::ServerError);
						});
						break;
					}
				}
				break;
			}
//			case SapphireComm::LinkCancel: {
//				CHECK_CLIENT_ID();
//
//				MainWorkerThread.post([=] {
//					linkIdentifier = 0;
//					enteredLinkIdentifier = 0;
//					write([=](EndianOutputStream<Endianness::Big>& ostream) {
//								ostream.serialize<SapphireComm>(SapphireComm::LinkCancel);
//							});
//				});
//				break;
//			}
			case SapphireComm::Terminate: {
				writeNoTerminateCheck([=](EndianOutputStream<Endianness::Big>& ostream) {
					ostream.serialize<SapphireComm>(SapphireComm::TerminateOk);
				});
				break;
			}
			case SapphireComm::TerminateOk: {
				MainWorkerThread.post([=] {
					this->stop();
				});
				break;
			}
			case SapphireComm::PingRequest: {
				LOGI()<< "Ping request";
				uint32 extra;
				if (!stream->deserialize<uint32>(extra)) {
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tPingRequest");
					goto exit_loop;
				}
				write([=](EndianOutputStream<Endianness::Big>& ostream) {
							ostream.serialize<SapphireComm>(SapphireComm::PingResponse);
							ostream.serialize<uint32>(extra);
						});
				break;
			}
			case SapphireComm::PingResponse: {
				LOGI() << "Ping response";
				uint32 extra;
				if (!stream->deserialize<uint32>(extra)) {
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tPingResponse");
					goto exit_loop;
				}
				write([=](EndianOutputStream<Endianness::Big>& ostream) {
							pingOrDisconnectCounter = 0;
						});
				break;
			}
			case SapphireComm::LevelProgressRemoteChanged: {
				int32 index;
				ProgressSynchId synchid;
				if (!stream->deserialize<int32>(index) || !stream->deserialize<ProgressSynchId>(synchid)) {
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID,
							"Abort connection\tRead failure\tLevelProgressRemoteChanged");
					goto exit_loop;
				}
				writerWorker.post(
						[=] () mutable {
							if (index < 0 || index >= hardwareProgressChangedListeners.size()) {
								LOGI() << "LevelProgressRemoteChanged index out of bounds " << index;
								return;
							}
							auto* listener = hardwareProgressChangedListeners.get(index);
							if (listener->listener == nullptr) {
								/* already unsubscribed */
								return;
							}
							if (listener->nextProgressId != synchid) {
								LOGI() << "LevelProgressRemoteChanged progress id out of bounds " << listener->nextProgressId << " - " << synchid;
								return;
							}
							auto incerror = DataStorage->increaseAssociatedHardwareProgressId(clientHardwareUUID, listener->hardwareUUID, &synchid);
							switch (incerror) {
								case SapphireStorageError::SUCCESS: {
									listener->nextProgressId++;
									queryAndSendProgressFromRemoteHardware(*listener);
									break;
								}
								default: {
									THROW() << (uint32)incerror << " " << index << " " << synchid;
									break;
								}
							}

						});
				break;
			}
			default: {
				postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tUnknown Command");
				LOGI() << "Unknown cmd: " << cmd;
				goto exit_loop;
			}
		}
	}

	exit_loop:

	writerWorker.stop();
	for (auto&& l : hardwareProgressChangedListeners) {
		if (l->listener != nullptr) {
			DataStorage->removeHardwareProgressChangedListener(l->hardwareUUID, l->listener);
		}
	}
	hardwareProgressChangedListeners.clear();
}

void ClientConnection::queryAndSendProgressFromRemoteHardware(HardwareListener& listener) {
	if (terminated) {
		return;
	}
	SapphireUUID outlevel;
	SapphireLevelProgress outprogress;
	ProgressSynchId queryid = listener.nextProgressId;
	auto error = DataStorage->queryLevelProgress(&queryid, listener.hardwareUUID, &outlevel, &outprogress);
	switch (error) {
		case SapphireStorageError::SUCCESS: {
			sendRemoteHardwareProgress(listener, queryid, listener.hardwareUUID, outlevel, outprogress);
			break;
		}
		case SapphireStorageError::OUT_OF_BOUNDS: {
			listener.nextProgressId = queryid;
			break;
		}
		default: {
			break;
		}
	}
}
void ClientConnection::sendRemoteHardwareProgress(HardwareListener& listener, ProgressSynchId synchid, const SapphireUUID& hardware,
		const SapphireUUID& level, SapphireLevelProgress progress) {
	if (terminated || listener.nextProgressId < synchid) {
		return;
	}
	ASSERT(listener.nextProgressId == synchid);
	EndianOutputStream<Endianness::Big>& ostream = *outputStream;
	ostream.serialize<SapphireComm>(SapphireComm::LevelProgressRemoteChanged);
	ostream.serialize<int32>(hardwareProgressChangedListeners.indexOf(&listener));
	ostream.serialize<ProgressSynchId>(synchid);
	ostream.serialize<SapphireUUID>(level);
	ostream.serialize<SapphireLevelCommProgress>(sapphireLevelProgressToComm(progress));
}

} //namespace userapp
