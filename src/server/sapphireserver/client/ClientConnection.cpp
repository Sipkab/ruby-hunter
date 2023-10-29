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
 * ClientConnection.cpp
 *
 *  Created on: 2016. okt. 18.
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

namespace userapp {
using namespace rhfw;

ClientConnection::ClientConnection(TCPConnection* connection)
		: connection(connection) {
	MainRandomer->read(connectionIdentifier.getData(), SapphireUUID::UUID_LENGTH);
}
ClientConnection::~ClientConnection() {
	writerWorker.stop();

	delete cipherOutputStream;
	delete normalOutputStream;
	if (registeredLevelChangedListener) {
		DataStorage->removeLevelChangedListener(levelChangedListener);
	}
	if (messagesChangedListener != nullptr) {
		DataStorage->removeMessagesChangedListener(messagesChangedListener);
	}
	if (hardwareAssociationListener != nullptr) {
		DataStorage->removeHardwareAssociationListener(hardwareAssociationListener);
	}
	for (auto&& l : hardwareProgressChangedListeners) {
		if (l->listener != nullptr) {
			DataStorage->removeHardwareProgressChangedListener(l->hardwareUUID, l->listener);
		}
	}
	hardwareProgressChangedListeners.clear();
	delete connection;
	LOGTRACE();
}

template<>
void ClientConnection::clientReadFunction<3>() {
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
			LOGI() << "Failed to read CMD " << (unsigned int) cmd;
			postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tCommand");
			return;
		}
		LOGI() << "Command: " << cmd;
		if (terminated && cmd != SapphireComm::Terminate && cmd != SapphireComm::TerminateOk) {
			//terminate commands isokay
			writeError(cmd, SapphireCommError::Terminated);
			postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tTerminated");
			break;
		}
		switch (cmd) {
			case SapphireComm::UpgradeStream: {
				uint8 servrdm[sizeof(SAPPHIRE_COMM_PV_KEY)];
				if (stream->read(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY)) != sizeof(SAPPHIRE_COMM_PV_KEY)) {
					LOGI() << "Failed to read 256 user random";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tUpgradeStream");
					return;
				}
				for (int i = 0; i < sizeof(SAPPHIRE_COMM_PV_KEY); ++i) {
					servrdm[i] ^= SAPPHIRE_COMM_PV_KEY[i];
				}
				cipher.initCipher(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY));

				stream = &upgistream;
				LOGI() << "Read stream upgraded";

				postUpgradeStream();
				break;
			}
			case SapphireComm::Login: {
				SapphireUUID userid;
				if (!stream->deserialize<SapphireUUID>(userid)) {
					LOGI() << "Failed to read userid";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tLogin");
					return;
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

				LOGI() << "Login with uuid: " << userid.asString();

				RegistrationToken token;
				FixedString username;
				SapphireDifficulty usercolor;
				SapphireStorageError loginerror = DataStorage->loginUser(userid, &token, &username, &usercolor);
				switch (loginerror) {
					case SapphireStorageError::USER_NOT_FOUND: {
						postConnectionUserLogEvent(connectionIdentifier, userid, "Login\tUSER_NOT_FOUND");
						LOGI() << "Send registration token";
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

						postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Login\tSUCCESS");
						//successful login, no response required

						if (IsMaintenanceConnectionDisabled()) {
							writeTerminate([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::Information);
								ostream.serialize<SapphireCommunityInformation>(SapphireCommunityInformation::Maintenance);
							});
						} else {
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::Login);
							});
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
							write([=](EndianOutputStream<Endianness::Big>& ostream) {
								ostream.serialize<SapphireComm>(SapphireComm::Login);
							});
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
					LOGI() << "Failed to read userid, name";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tUpdatePlayerData");
					return;
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

				LOGI() << "Uploading level...";
				Level level;
				if (!level.loadLevel(*stream)) {
					writeError(cmd, SapphireCommError::LevelReadFailed);
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tUploadLevel");
					return;
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
				LOGI() << "Received level with UUID: " << level.getInfo().uuid.asString();
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
						LOGE() << "Datastorage save level error: " << (int) storeerror;
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
					LOGI() << "Failed to read download start";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tGetLevels");
					return;
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
					LOGI() << "Failed to read query index";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tQuerySingleLevel");
					return;
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
					LOGI() << "Failed to read download leveluuid";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tDownloadLevel");
					return;
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
					default: {
						postConnectionUserLogEvent(connectionIdentifier, clientUUID,
								FixedString { "DownloadLevel\tServerError\t" } + uuid.asString());
						LOGI() << "Storage error: " << (int) storeerror;
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
					LOGI() << "Failed to read rate level uuid or rating";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tRateLevel");
					return;
				}
				LOGI() << "Rate level: " << uuid.asString() << " for: " << (unsigned int) rating;
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
							LOGI() << "Storage error: " << (int) storeerror;
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
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tQueryMessages");
					return;
				}
				unsigned int inoutstart = start;
				unsigned int outcount;
				LocalArray<SapphireDiscussionMessage, 64> messages;
				auto storeerror = DataStorage->queryMessages(messages, count > 64 ? 64 : count, &inoutstart, &outcount);
				LOGI() << "Query messages: " << start << " (" << count << ") result: " << inoutstart << " (" << outcount << ")";
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
						LOGI() << "Storage error: " << (int) storeerror;
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
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tSendMessage");
					return;
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
						LOGI() << "Storage error: " << (int) storeerror;
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
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tReportLevel");
					return;
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
					LOGI() << "Read failure";
					postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tRead failure\tCommunityNotifications");
					return;
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
				LOGI() << "Ping request";
				write([=](EndianOutputStream<Endianness::Big>& ostream) {
					ostream.serialize<SapphireComm>(SapphireComm::PingResponse);
				});
				break;
			}
			case SapphireComm::PingResponse: {
				LOGI() << "Ping response";
				write([=](EndianOutputStream<Endianness::Big>& ostream) {
					pingOrDisconnectCounter = 0;
				});
				break;
			}
			default: {
				postConnectionUserLogEvent(connectionIdentifier, clientUUID, "Abort connection\tUnknown Command");
				LOGI() << "Unknown cmd: " << cmd;
				return;
			}
		}
	}
}
template<>
void ClientConnection::clientReadFunction<2>() {
	clientReadFunction<3>();
}

void ClientConnection::readThreadFunction(TCPConnection* conn) {
	auto istream = EndianInputStream<Endianness::Big>::wrap(LoopedInputStream::wrap(*conn));
	auto&& addr = static_cast<const IPv4Address&>(conn->getAddress());

	uint32 version;
	SapphireUUID clientuuid;
	SY_COMM_CMD releasetype;
	SY_COMM_CMD platformtype;
	char buffer[sizeof(SAPPHIRE_SERVER_HELLO_STRING) - 1];
	int helloread = istream.read(buffer, sizeof(SAPPHIRE_SERVER_HELLO_STRING) - 1);
	if (helloread == sizeof(SAPPHIRE_SERVER_HELLO_STRING) - 1
			&& memcmp(buffer, SAPPHIRE_SERVER_HELLO_STRING, sizeof(SAPPHIRE_SERVER_HELLO_STRING) - 1) == 0
			&& istream.deserialize<uint32>(version) && istream.deserialize<SapphireUUID>(clientuuid)
			&& istream.deserialize<SY_COMM_CMD>(releasetype) && istream.deserialize<SY_COMM_CMD>(platformtype)) {
		clientAppVersion = version;

		LOGI() << "Client version number: " << version << " hardware id: " << clientuuid.asString();
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Connected\tVersion\t%u\tIP\t%u.%u.%u.%u\tHardware\t%s\t%s\t%s", version, addr.getAddressBytes()[0],
				addr.getAddressBytes()[1], addr.getAddressBytes()[2], addr.getAddressBytes()[3], (const char*) clientuuid.asString(),
				"", "");

		postConnectionLogEvent(connectionIdentifier, buffer);

		{
			auto eostream = EndianOutputStream<Endianness::Big>::wrap(*conn);
			auto upgeostream = EndianOutputStream<Endianness::Big>::wrap(RC4OutputStream::wrap(*conn, writeCipher));

			normalOutputStream = new decltype(eostream)(util::move(eostream));
			cipherOutputStream = new decltype(upgeostream)(util::move(upgeostream));
			outputStream = normalOutputStream;
		}
		outputStream->write(SAPPHIRE_CLIENT_HELLO_STRING, sizeof(SAPPHIRE_CLIENT_HELLO_STRING) - 1);

		Semaphore sem { Semaphore::auto_init { } };
		MainWorkerThread.post([&] {
			clientHardwareUUID = clientuuid;
			for (auto&& c : ClientConnections.objects()) {
				if(&c != this && c.clientHardwareUUID == this->clientHardwareUUID) {
					c.terminate();
					break;
				}
			}
			sem.post();
		});
		sem.wait();
		switch (version) {
			case 6:
			case 5: {
				clientReadFunction<5>();
				postConnectionLogEvent(connectionIdentifier, "Disconnected");
				break;
			}
			//no longer supported
//			case 4:
//			case 3: {
//				clientReadFunction<3>();
//				postConnectionLogEvent(connectionIdentifier, "Disconnected");
//				break;
//			}
//			case 2: {
//				//first version to include community
//				clientReadFunction<2>();
//				postConnectionLogEvent(connectionIdentifier, "Disconnected");
//				break;
//			}
			default: {
				outputStream->serialize<SapphireComm>(SapphireComm::Information);
				outputStream->serialize<SapphireCommunityInformation>(SapphireCommunityInformation::Maintenance);
				postConnectionLogEvent(connectionIdentifier, "NewerVersionDisconnected");
				LOGE() << "Unknown version number: " << version;
				goto failed;
			}
		}
	} else {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Handshake error\t%u.%u.%u.%u", addr.getAddressBytes()[0], addr.getAddressBytes()[1], addr.getAddressBytes()[2],
				addr.getAddressBytes()[3]);
		postConnectionLogEvent(connectionIdentifier, buffer);

		LOGI() << "Hello handshake message missing " << helloread;
		goto failed;
	}

	failed:

	;
}

void ClientConnection::start() {
	writerWorker.start();

	LOGI() << "Start client connection " << connection->getAddress();

	MainWorkerThread.post([=] {
		ClientConnections.addToEnd(*this);
		MaintenanceEvents += maintenanceListener;
	});

	Thread t;
	t.start([=] {
		LOGI() << "Start client thread";
		readThreadFunction(connection);

		MainWorkerThread.post([=] {
					ClientConnectionState state = getState();
					for (auto&& l : UserStateEvents.foreach()) {
						l(state, UserState::DISCONNECTED);
					}
					delete this;
					if(ClientConnections.isEmpty()) {
						MaintenanceOpportunity();
					}
				});

		LOGI() << "Exit client thread";

		return 0;
	});
}

void ClientConnection::writeError(SapphireComm cmd, SapphireCommError error) {
	LOGE() << "Writing error for: " << cmd << ": " << error;
	writeNoTerminateCheck([=](EndianOutputStream<Endianness::Big>& ostream) {
		ostream.serialize<SapphireComm>(SapphireComm::Error);
		ostream.serialize<SapphireComm>(cmd);
		ostream.serialize<SapphireCommError>(error);
	});
}

void ClientConnection::stop() {
	writerWorker.stop();
	connection->disconnect();

	delete cipherOutputStream;
	delete normalOutputStream;

	normalOutputStream = nullptr;
	cipherOutputStream = nullptr;
	outputStream = nullptr;
}

void ClientConnection::postUpgradeStream() {
	writePrivate([=](EndianOutputStream<Endianness::Big>*& ostream) {
		uint8 localrdm[sizeof(SAPPHIRE_COMM_PV_KEY)];
		if(MainRandomer->read(localrdm, sizeof(SAPPHIRE_COMM_PV_KEY)) != sizeof(SAPPHIRE_COMM_PV_KEY)) {
			LOGI() << "Failed to generate 256 local random";
			return;
		}
		ostream->serialize<SapphireComm>(SapphireComm::UpgradeStream);
		ostream->write(localrdm, sizeof(SAPPHIRE_COMM_PV_KEY));
		for (int i = 0; i < sizeof(SAPPHIRE_COMM_PV_KEY); ++i) {
			localrdm[i] ^= SAPPHIRE_COMM_PV_KEY[i];
		}
		writeCipher.initCipher(localrdm, sizeof(SAPPHIRE_COMM_PV_KEY));

		ostream = cipherOutputStream;
		LOGI() << "Write stream upgraded";
	});
}

void ClientConnection::writeUserStateChanged(const ClientConnectionState& conn, bool online) {
	if (!conn.connectionId) {
		return;
	}
	write([=](EndianOutputStream<Endianness::Big>& ostream) {
		ostream.serialize<SapphireComm>(SapphireComm::UserStateChanged);
		ostream.serialize<bool>(online);
		ostream.serialize<SapphireUUID>(conn.connectionId);
		if(online) {
			ostream.serialize<SapphireDifficulty>(conn.userDifficultyColor);
			ostream.serialize<FixedString>(conn.userName);
		}
	});
}

void ClientConnection::setMaintenanceMode(bool mode) {
	if (mode && this->clientUUID) {
		writeTerminate([=](EndianOutputStream<Endianness::Big>& ostream) {
			ostream.serialize<SapphireComm>(SapphireComm::Information);
			ostream.serialize<SapphireCommunityInformation>(SapphireCommunityInformation::Maintenance);
		});
	}
}

void ClientConnection::sendPingRequest(uint32 id) {
	if (clientAppVersion >= 5) {
		write([=](EndianOutputStream<Endianness::Big>& ostream) {
			ostream.serialize<SapphireComm>(SapphireComm::PingRequest);
			ostream.serialize<uint32>(id);
		});
	} else if (clientAppVersion > 0) {
		write([=](EndianOutputStream<Endianness::Big>& ostream) {
			ostream.serialize<SapphireComm>(SapphireComm::PingRequest);
		});
	}
}
void ClientConnection::sendPingRequestOrDisconnect() {
	write([=](EndianOutputStream<Endianness::Big>& ostream) {
		if (pingOrDisconnectCounter == 0) {
			if (clientAppVersion >= 5) {
				ostream.serialize<SapphireComm>(SapphireComm::PingRequest);
				ostream.serialize<uint32>(0);
			} else if (clientAppVersion > 0) {
				ostream.serialize<SapphireComm>(SapphireComm::PingRequest);
			}
			++pingOrDisconnectCounter;
		} else {
			MainWorkerThread.post([=] {
						stop();
					});
		}
	});
}
}  // namespace userapp

