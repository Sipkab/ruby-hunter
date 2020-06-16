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
 * CommunityConnection.cpp
 *
 *  Created on: 2016. okt. 16.
 *      Author: sipka
 */

#include <framework/utils/ContainerLinkedNode.h>
#include <framework/utils/FixedString.h>

#include <sapphire/community/CommunityConnection.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/server/SapphireLevelDetails.h>
#include <sapphire/common/RegistrationToken.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/steam_opt.h>
#include <StartConfiguration.h>

#include <sapphire/community/nslookup.h>

#include <gen/log.h>
#include <gen/fwd/types.h>

#include <time.h>

namespace userapp {
using namespace rhfw;

CommunityConnection::CommunityConnection(SapphireScene* scene)
		: scene(scene) {
}
CommunityConnection::~CommunityConnection() {
	readTask.cancel();

	ASSERT(!connection.isConnected());

	delete cipherOutputStream;
	delete normalOutputStream;
}

void CommunityConnection::connect(SapphireScene* scene) {
	if (isConnectionEstablished()) {
		return;
	}
	randomer = scene->getUUIDRandomer();

	readTask.cancel();

	delete cipherOutputStream;
	delete normalOutputStream;

	normalOutputStream = nullptr;
	cipherOutputStream = nullptr;
	outputStream = nullptr;

	//clear pending requests from writer worker
	writeWorker.reset();
	readTask.setActions([=] {
		if(!loggedHardwareUUID) {
			/*failed to connect*/
			for (auto&& l : stateEvents.foreach()) {
				l.onConnectionFailed(this);
			}
		}
		writeWorker.reset();
		connection.disconnect();
		loggedHardwareUUID = SapphireUUID {};
		userUUID = SapphireUUID {};
		levelDetails.clear();
		onlineUsers.clear();
		messages.clear();
		connectionTaskRunning = false;
		messagesRemoteStartIndex = 0;
		messagesRemoteCount = 0;
		messagesLocalStartIndex = 0;
		if(loggedIn) {
			loggedIn = false;
			for (auto&& l : stateEvents.foreach()) {
				l.onDisconnected(this);
			}
		}
	}, [=] {
		writeWorker.post([=] {
					writeWorker.signalStop();
					auto&& eostream = *outputStream;
					eostream.serialize<SapphireComm>(SapphireComm::Terminate);
				});
		writeWorker.reset();
		connection.disconnect();
		loggedHardwareUUID = SapphireUUID {};
		userUUID = SapphireUUID {};
		levelDetails.clear();
		onlineUsers.clear();
		messages.clear();
		connectionTaskRunning = false;
		messagesRemoteStartIndex = 0;
		messagesRemoteCount = 0;
		messagesLocalStartIndex = 0;
		if(loggedIn) {
			loggedIn = false;
			for (auto&& l : stateEvents.foreach()) {
				l.onDisconnected(this);
			}
		}
	});

	userUUID = scene->getCurrentUser().getUUID();
	SapphireUUID hardwareid = scene->getSettings().hardwareUUID;
	readTask.setRunnable([=] {
		readThreadFunction(readTask, hardwareid);
		connection.disconnect();
	});

	connectionTaskRunning = true;
	readTask.start();
}

TCPIPv4Address CommunityConnection::getCommunityServerAddress() const {
	auto&& sc = getStartConfiguration();
	TCPIPv4Address address;
	if (sc.serverAddress.getPort() != 0) {
		return sc.serverAddress;
	}
	//perform the lookup even if we're compiling for debug
	IPv4Address addr;
	bool lookupsuccess = lookupIPv4Address(SAPPHIRE_SERVER_HOSTNAME, &addr);

#if RHFW_DEBUG

	return TCPIPv4Address { IPv4Address { 127, 0, 0, 1 }, SAPPHIRE_SERVER_PORT_NUMBER };

#else

	if (lookupsuccess) {
		return TCPIPv4Address { IPv4Address { addr }, SAPPHIRE_SERVER_PORT_NUMBER };
	}

#endif /* RHFW_DEBUG */

	return TCPIPv4Address { IPv4Address { 0 }, SAPPHIRE_SERVER_PORT_NUMBER };
}

void CommunityConnection::readThreadFunction(AsynchronTask& task, const SapphireUUID& hardwareuuid) {
	auto address = getCommunityServerAddress();
	if (address.getNetworkAddressInt() == 0) {
		LOGW() << "No community server address.";
		return;
	}
	bool res = connection.connect(address);
	WARN(!res) << "Failed to connect to server";
	if (!res) {
		return;
	}
	{
		auto ostream = EndianOutputStream<Endianness::Big>::wrap(connection);

		ostream.write(SAPPHIRE_SERVER_HELLO_STRING, sizeof(SAPPHIRE_SERVER_HELLO_STRING) - 1);
		ostream.serialize<uint32>(SAPPHIRE_RELEASE_VERSION_NUMBER);
		ostream.serialize<SapphireUUID>(hardwareuuid);
		ostream.serialize<SY_COMM_CMD>(0);
		ostream.serialize<SY_COMM_CMD>(0);

		char buffer[1024];
		int helloread = connection.read(buffer, sizeof(SAPPHIRE_CLIENT_HELLO_STRING) - 1);
		if (helloread == sizeof(SAPPHIRE_CLIENT_HELLO_STRING) - 1
				&& memcmp(buffer, SAPPHIRE_CLIENT_HELLO_STRING, sizeof(SAPPHIRE_CLIENT_HELLO_STRING) - 1) == 0) {
			LOGI() << "Received server handshake";

			auto eostream = EndianOutputStream<Endianness::Big>::wrap(connection);
			auto upgeostream = EndianOutputStream<Endianness::Big>::wrap(RC4OutputStream::wrap(connection, writeCipher));

			normalOutputStream = new decltype(eostream)(util::move(eostream));
			cipherOutputStream = new decltype(upgeostream)(util::move(upgeostream));
			outputStream = normalOutputStream;

			loggedHardwareUUID = hardwareuuid;

			writeWorker.start();
			postLogin();

		} else {
			return;
		}
	}
	task.postTask([=] {
		for (auto&& l : stateEvents.foreach()) {
			l.onConnected(this);
		}
	});
	{
		RC4Cipher cipher;

		auto istream = LoopedInputStream::wrap(BufferedInputStream::wrap(connection));
		auto eistream = EndianInputStream<Endianness::Big>::wrap(istream);
		auto upgistream = EndianInputStream<Endianness::Big>::wrap(RC4InputStream::wrap(istream, cipher));

		EndianInputStream<Endianness::Big>* stream = &eistream;
		while (true) {
			SapphireComm cmd;
			if (!stream->deserialize<SapphireComm>(cmd)) {
				LOGI() << "Failed to read CMD";
				goto exit_loop;
			}
			LOGI() << "Command: " << cmd;
			switch (cmd) {
				case SapphireComm::Error: {
					LOGI() << "Error";
					SapphireComm errcmd;
					if (!stream->deserialize<SapphireComm>(errcmd)) {
						LOGI() << "Failed to read CMD";
						goto exit_loop;
					}
					LOGI() << "Error in command: " << errcmd;

					SapphireCommError error;
					if (!stream->deserialize<SapphireCommError>(error)) {
						LOGE() << "Failed to read error " << errcmd;
						goto exit_loop;
					} else {
						LOGI() << "Error code: " << error;
					}
					break;
				}
				case SapphireComm::UpgradeStream: {
					uint8 servrdm[sizeof(SAPPHIRE_COMM_PV_KEY)];
					if (stream->read(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY)) != sizeof(SAPPHIRE_COMM_PV_KEY)) {
						THROW() << "Failed to read 256 random";
						goto exit_loop;
					}
					for (int i = 0; i < sizeof(SAPPHIRE_COMM_PV_KEY); ++i) {
						servrdm[i] ^= SAPPHIRE_COMM_PV_KEY[i];
					}
					cipher.initCipher(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY));

					stream = &upgistream;
					break;
				}
				case SapphireComm::Login: {
					ProgressSynchId progressid;
					if (!stream->deserialize<ProgressSynchId>(progressid)) {
						LOGI() << "Failed to read ERROR";
						goto exit_loop;
					}
					LOGTRACE() << "Login " << progressid;
					task.postTask([=] {
						loggedIn = true;
						serverProgressSynchId = progressid;
						for (auto&& l : stateEvents.foreach()) {
							l.onLoggedIn(this);
						}
						checkPostLevelProgress();
					});
					break;
				}
				case SapphireComm::UploadLevel: {
					SapphireCommError err;
					SapphireUUID leveluuid;
					if (!stream->deserialize<SapphireCommError>(err)) {
						LOGI() << "Failed to read ERROR";
						goto exit_loop;
					}
					if (!stream->deserialize<SapphireUUID>(leveluuid)) {
						LOGI() << "Failed to read level UUID " << err;
						goto exit_loop;
					}
					LOGI() << "Level upload response: " << leveluuid.asString() << " " << err;

					task.postTask([=] {
						for (auto&& l : levelUploadEvents.foreach()) {
							l(err, leveluuid);
						}
					});
					break;
				}
				case SapphireComm::RegistrationToken: {
					RegistrationToken token;
					if (!stream->deserialize<RegistrationToken>(token)) {
						THROW() << "Failed to read";
						goto exit_loop;
					}
					saveRegistrationToken(token);
					writeWorker.post([=] {
						auto&& eostream = *outputStream;
						if (!eostream.serialize<SapphireComm>(SapphireComm::RegistrationTokenReceived)) {
							return false;
						}
						return true;
					});
					break;
				}
				case SapphireComm::GetLevels: {
					SapphireCommError err;
					uint32 start;
					if (!stream->deserialize<SapphireCommError>(err) || !stream->deserialize<uint32>(start)) {
						LOGI() << "Failed to read ERROR";
						goto exit_loop;
					}
					LOGI() << "Get level err: " << err << " start: " << start;
					switch (err) {
						case SapphireCommError::NoError: {
							uint32 count;
							if (!stream->deserialize<uint32>(count)) {
								LOGI() << "Failed to read received count";
								goto exit_loop;
							}
							LOGI() << "Receiving info about " << count << " levels";
							if (count == 0) {
								LOGI() << "No more level info to receive";
								task.postTask([=] {
									for (auto&& l : levelsQueriedEvents.foreach()) {
										l(start, nullptr);
									}
								});
							} else {
								for (unsigned int i = 0; i < count; ++i) {
									SapphireLevelDetails detail;
									if (!stream->deserialize<SapphireLevelDetails>(detail)) {
										LOGI() << "Failed to read level details";
										goto exit_loop;
									}
									LOGTRACE() << "Post query level: " << detail.uuid.asString();
									task.postTask([=] {
										LOGI() << "Level: " << detail.title << " with id: " << detail.uuid.asString();
										auto* real = new SapphireLevelDetails(util::move(detail));
										levelDetails.add(real);

										for (auto&& l : levelsQueriedEvents.foreach()) {
											l(start + i, real);
										}
									});
								}
							}
							break;
						}
						case SapphireCommError::ValueOutOfBounds: {
							task.postTask([=] {
								for (auto&& l : levelsQueriedEvents.foreach()) {
									l(start, nullptr);
								}
							});
							break;
						}
						default: {
							break;
						}
					}
					break;
				}
				case SapphireComm::DownloadLevel: {
					SapphireCommError err;
					if (!stream->deserialize<SapphireCommError>(err)) {
						LOGI() << "Failed to read ERROR";
						goto exit_loop;
					}
					LOGI() << "Download level err: " << err;
					switch (err) {
						case SapphireCommError::NoError: {
							Level level;
							if (!level.loadLevel(*stream)) {
								LOGI() << "Failed to load downloaded level";
								goto exit_loop;
							}
							task.postTask([=] {
								for (auto&& l : levelDownloadEvents.pointers()) {
									if(l != nullptr) {
										(*l)(level.getInfo().uuid, SapphireCommError::NoError, &level);
									}
								}
							});
							break;
						}
						default: {
							SapphireUUID uuid;
							if (!stream->deserialize<SapphireUUID>(uuid)) {
								LOGI() << "Failed to read level UUID " << err;
								goto exit_loop;
							}
							LOGE() << "Failed to download level " << uuid.asString();
							task.postTask([=] {
								for (auto&& l : levelDownloadEvents.pointers()) {
									if(l != nullptr) {
										(*l)(uuid, err, nullptr);
									}
								}
							});
							break;
						}
					}
					break;
				}
				case SapphireComm::RateLevel: {
					SapphireUUID uuid;
					SapphireCommError error;
					if (!stream->deserialize<SapphireCommError>(error) || !stream->deserialize<SapphireUUID>(uuid)) {
						LOGI() << "Failed to read rate level uuid or rating";
						goto exit_loop;
					}
					LOGI() << "Rate level response: " << uuid.asString() << ": " << error;
					break;
				}
				case SapphireComm::SendMessage: {
					SapphireCommError error;
					uint32 userid;
					if (!stream->deserialize<SapphireCommError>(error) || !stream->deserialize<uint32>(userid)) {
						LOGI() << "Read failure";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							task.postTask([=] {
								for (auto&& l : sendMessageEvents.foreach()) {
									l(userid, true);
								}
							});
							break;
						}
						default: {
							task.postTask([=] {
								for (auto&& l : sendMessageEvents.foreach()) {
									l(userid, false);
								}
							});
							break;
						}
					}
					break;
				}
				case SapphireComm::QueryMessages: {
					SapphireCommError error;
					uint32 requeststart;
					uint32 requestcount;
					if (!stream->deserialize<SapphireCommError>(error) || !stream->deserialize<uint32>(requeststart)
							|| !stream->deserialize<uint32>(requestcount)) {
						LOGI() << "Read failure";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							uint32 start;
							uint32 count;
							if (!stream->deserialize<uint32>(start) || !stream->deserialize<uint32>(count)) {
								LOGI() << "Read failure";
								goto exit_loop;
							}
							ASSERT(start >= requeststart);
							ASSERT(count <= requestcount);
							if (start < requeststart || count > requestcount) {
								LOGI() << "Data range failure";
								goto exit_loop;
							}
							if (count == 0) {
								break;
							}
							task.postTask([=] {
								if(messages.size() == 0) {
									messagesLocalStartIndex = start;
									messages.reserveInEnd(count);
								} else {
									if(start < messagesLocalStartIndex) {
										messages.reserveInFront(messagesLocalStartIndex - start);
										messagesLocalStartIndex = start;
										if(messages.size() < count) {
											messages.reserveInEnd(count - messages.size());
										}
									} else if(start > messagesLocalStartIndex) {
										messages.reserveInEnd(start + count - messagesLocalStartIndex - messages.size());
									} else { //start == messagesLocalStartIndex
										if(messages.size() < count) {
											messages.reserveInEnd(count - messages.size());
										}
									}
								}
							});
							for (unsigned int i = 0; i < count; ++i) {
								SapphireDiscussionMessage msg;
								if (!stream->deserialize<SapphireDiscussionMessage>(msg)) {
									goto exit_loop;
								}
								task.postTask([=] {
									auto* real = new SapphireDiscussionMessage(util::move(msg));
									unsigned int index = start + i - messagesLocalStartIndex;
									messages.set(index, real);
									for (auto&& l : discussionMessageArrivedEvents.foreach()) {
										l(index);
									}
								});
							}
							task.postTask([=] {
								if(requeststart != start) {
									unsigned int diff = requeststart - start;
									requestMessages(start, diff);
									if(start + count < requeststart + requestcount) {
										requestMessages(start + count, requeststart + requestcount - (start + count));
									}
								} else {
									if(count < requestcount) {
										requestMessages(requeststart + count, requestcount - count);
									}
								}
							});
							break;
						}
						case SapphireCommError::ValueOutOfBounds: {
							uint32 start;
							uint32 count;
							if (!stream->deserialize<uint32>(start) || !stream->deserialize<uint32>(count)) {
								LOGI() << "Read failure";
								goto exit_loop;
							}
							task.postTask([=] {
								if(start!= this->messagesRemoteStartIndex || count != this->messagesRemoteCount) {
									this->messagesRemoteStartIndex = start;
									this->messagesRemoteCount = count;
									for (auto&& l : discussionMessagesChangedEvents.foreach()) {
										l(this->messagesRemoteStartIndex, this->messagesRemoteCount);
									}
								}
							});
							break;
						}
						default: {
							break;
						}
					}
					break;
				}
				case SapphireComm::DiscussionMessagesChanged: {
					uint32 startindex;
					uint32 count;
					if (!stream->deserialize<uint32>(startindex) || !stream->deserialize<uint32>(count)) {
						LOGI() << "Read failure";
						goto exit_loop;
					}
					task.postTask([=] {
						if(startindex != this->messagesRemoteStartIndex || count != this->messagesRemoteCount) {
							this->messagesRemoteStartIndex = startindex;
							this->messagesRemoteCount = count;
							for (auto&& l : discussionMessagesChangedEvents.foreach()) {
								l(this->messagesRemoteStartIndex, this->messagesRemoteCount);
							}
						}
					});
					break;
				}
				case SapphireComm::UserStateChanged: {
					bool online;
					SapphireUUID connid;
					if (!stream->deserialize<bool>(online) || !stream->deserialize<SapphireUUID>(connid)) {
						goto exit_loop;
					}
					if (online) {
						//someone connected
						SapphireDifficulty diffstate;
						FixedString name;
						if (!stream->deserialize<SapphireDifficulty>(diffstate)
								|| !stream->deserialize<SafeFixedString<SAPPHIRE_USERNAME_MAX_LEN>>(name)) {
							LOGI() << "Read failure";
							goto exit_loop;
						}
						LOGI() << "User became online: " << name << " with difficulty state: " << diffstate;
						task.postTask(
								[=] {
									for (auto&& p : onlineUsers.pointers()) {
										if(p->getConnectionId() == connid) {
											p->getUserName() = util::move(name);
											p->getDifficultyLevel() = diffstate;
											return;
										}
									}
									ContainerLinkedNode<OnlineUser>* usernode = new ContainerLinkedNode<OnlineUser>(connid, util::move(name), diffstate);
									onlineUsers.addToEnd(*usernode);
									for (auto&& l : userStateChangedEvents.foreach()) {
										l(*usernode, true);
									}
								});
					} else {
						//someone disconnected
						LOGI() << "User became offline with id: " << connid.asString();
						task.postTask([=] {
							for (auto&& n : onlineUsers.nodes()) {
								if(n->get()->getConnectionId() == connid) {
									LOGI() << "User disconnected: " << n->get()->getUserName();
									n->removeLinkFromList();
									for (auto&& l : userStateChangedEvents.foreach()) {
										l(*n->get(), false);
									}
									delete n;
									break;
								}
							}
						});
					}
					break;
				}
				case SapphireComm::LevelDetailsChanged: {
					uint32 index;
					if (!stream->deserialize<uint32>(index)) {
						goto exit_loop;
					}
					task.postTask([=] {
						if(index <= levelDetails.size()) {
							//<= to get info about newly uploaded levels
							writeWorker.post([=] {
										auto&& eostream = *outputStream;
										if (!eostream.serialize<SapphireComm>(SapphireComm::QuerySingleLevel) ||
												!eostream.serialize<uint32>(index)) {
											return false;
										}
										return true;
									});
						}
					});
					break;
				}
				case SapphireComm::LevelRemoved: {
					uint32 index;
					if (!stream->deserialize<uint32>(index)) {
						goto exit_loop;
					}
					task.postTask([=] {
						if(index <= levelDetails.size()) {
							for (auto&& l : levelRemovedEvents.foreach()) {
								l(levelDetails.get(index));
							}
							delete levelDetails.remove(index);
						}
					});
					break;
				}
				case SapphireComm::QuerySingleLevel: {
					SapphireCommError error;
					uint32 index;
					if (!stream->deserialize<SapphireCommError>(error) || !stream->deserialize<uint32>(index)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							SapphireLevelDetails details;
							if (!stream->deserialize<SapphireLevelDetails>(details)) {
								LOGI() << "Failed to read";
								goto exit_loop;
							}
							task.postTask([=] {
								if(index < levelDetails.size()) {
									levelDetails[index] = details;
									for (auto&& l : levelsQueriedEvents.foreach()) {
										l(index, &levelDetails[index]);
									}
								} else if(index == levelDetails.size()) {
									levelDetails.add(new SapphireLevelDetails(util::move(details)));
									for (auto&& l : levelsQueriedEvents.foreach()) {
										l(index, &levelDetails[index]);
									}
								}
							});
							break;
						}
						default: {
							break;
						}
					}
					break;
				}
				case SapphireComm::Information: {
					SapphireCommunityInformation info;
					if (!stream->deserialize<SapphireCommunityInformation>(info)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					LOGI() << "Community information: " << info;
					task.postTask([=] {
						for (auto&& l : communityInformationEvents.foreach()) {
							l(info);
						}
					});
					break;
				}
				case SapphireComm::LevelProgress: {
					SapphireCommError error;
					if (!stream->deserialize<SapphireCommError>(error)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							task.postTask([=] {
								++serverProgressSynchId;
								checkPostLevelProgress();
							});
							break;
						}
						case SapphireCommError::InvalidOperation: {
							//invalid SapphireLevelCommProgress was sent
							task.postTask([=] {
								++serverProgressSynchId;
								checkPostLevelProgress();
							});
							break;
						}
						case SapphireCommError::LevelReadFailed: {
							LOGE() << error;
							//send the next one anyway
							task.postTask([=] {
								++serverProgressSynchId;
								checkPostLevelProgress();
							});
							break;
						}
						case SapphireCommError::ValueOutOfBounds: {
							ProgressSynchId id;
							if (!stream->deserialize<ProgressSynchId>(id)) {
								LOGI() << "Failed to read";
								goto exit_loop;
							}
							LOGI() << error << " " << id;
							task.postTask([=] {
								serverProgressSynchId = id;
								checkPostLevelProgress();
							});
							break;
						}
						default: {
							THROW() << error;
							//handle error
							break;
						}
					}
					break;
				}
				case SapphireComm::LinkAccRequest: {
					uint32 identifier;
					if (!stream->deserialize<uint32>(identifier)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					task.postTask([=] {
						LOGI() << "Received link id: " << identifier;
						this->linkIdentifier = identifier;
						for (auto&& l : linkIdentifierRequestEvents.foreach()) {
							l(identifier);
						}
					});
					break;
				}
				case SapphireComm::LinkResult: {
					SapphireCommError error;
					if (!stream->deserialize<SapphireCommError>(error)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					task.postTask([=] {
						this->linkIdentifier = 0;
						for (auto&& l : linkResultEvents.foreach()) {
							l(error);
						}
					});
					break;
				}
				case SapphireComm::LevelProgressRemoteChanged: {
					int32 index;
					ProgressSynchId synch;
					SapphireUUID level;
					SapphireLevelCommProgress progress;
					if (!stream->deserialize<int32>(index) || !stream->deserialize<ProgressSynchId>(synch)
							|| !stream->deserialize<SapphireUUID>(level) || !stream->deserialize<SapphireLevelCommProgress>(progress)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					task.postTask([=] {
						LOGI() << "Remote progress changed: " << level.asString() << " " << progress;
						auto error = scene->setRemoteLevelProgress(level, sapphireCommProgressToState(progress));
						/*returned error doesnt matter, just send the response*/
						writeWorker.post([=] {
									auto&& eostream = *outputStream;
									eostream.serialize<SapphireComm>(SapphireComm::LevelProgressRemoteChanged);
									eostream.serialize<int32>(index);
									eostream.serialize<ProgressSynchId>(synch);
								});
					});
					break;
				}
				case SapphireComm::LinkCancel: {
					//TODO listeners?
					break;
				}
				case SapphireComm::GetStatistics: {
					SapphireUUID leveluuid;
					SapphireCommError error;
					if (!stream->deserialize<SapphireUUID>(leveluuid) || !stream->deserialize<SapphireCommError>(error)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							LevelStatistics stats;
							unsigned int playcount;
							if (!LevelStatistics::deserialize(*stream, &stats, &playcount)) {
								LOGI() << "Failed to read";
								goto exit_loop;
							}
							task.postTask([=] {
								for (auto&& l : statisticsDownloadEvents.foreach()) {
									l(SapphireCommError::NoError, leveluuid, &stats, playcount);
								}
							});
							break;
						}
						case SapphireCommError::NoStatsYet: {
							task.postTask([=] {
								for (auto&& l : statisticsDownloadEvents.foreach()) {
									l(SapphireCommError::NoStatsYet, leveluuid, nullptr, 0);
								}
							});
							break;
						}
						default: {
							LOGI() << "GetStatistics error " << error << " for " << leveluuid.asString();
							task.postTask([=] {
								for (auto&& l : statisticsDownloadEvents.foreach()) {
									l(SapphireCommError::ServerError, leveluuid, nullptr, 0);
								}
							});
							break;
						}
					}
					break;
				}
				case SapphireComm::GetLeaderboard: {
					SapphireUUID leveluuid;
					SapphireLeaderboards leaderboard;
					SapphireCommError error;
					if (!stream->deserialize<SapphireUUID>(leveluuid) || !stream->deserialize<SapphireLeaderboards>(leaderboard)
							|| !stream->deserialize<SapphireCommError>(error)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							uint32 count;
							int32 userindex;
							uint32 userscore;
							int32 userposition;
							PlayerDemoId userdemoid;
							uint32 totalcount;
							if (!stream->deserialize<uint32>(count) || !stream->deserialize<int32>(userindex)
									|| !stream->deserialize<uint32>(userscore) || !stream->deserialize<int32>(userposition)
									|| !stream->deserialize<PlayerDemoId>(userdemoid) || !stream->deserialize<uint32>(totalcount)) {
								LOGI() << "Failed to read";
								goto exit_loop;
							}
							ArrayList<LeaderboardEntry> entries;
							for (unsigned int i = 0; i < count; ++i) {
								LeaderboardEntry* e = new LeaderboardEntry();
								if (!stream->deserialize<uint32>(e->score) || !stream->deserialize<PlayerDemoId>(e->demoId)
										|| !stream->deserialize<SafeFixedString<SAPPHIRE_USERNAME_MAX_LEN>>(e->userName)) {
									delete e;
									LOGI() << "Failed to read";
									goto exit_loop;
								}
								entries.add(e);
							}
							task.postTask(
									[=] {
										for (auto&& l : leaderboardDownloadedEvents.foreach()) {
											l(leveluuid, leaderboard, error, &entries, userindex, userscore, userposition, userdemoid, totalcount);
										}
									});
							break;
						}
						case SapphireCommError::NoStatsYet: {
							task.postTask([=] {
								for (auto&& l : leaderboardDownloadedEvents.foreach()) {
									l(leveluuid, leaderboard, error, nullptr, -1, 0, -1, 0, 0);
								}
							});
							break;
						}
						default: {
							LOGE() << error;
							task.postTask([=] {
								for (auto&& l : leaderboardDownloadedEvents.foreach()) {
									l(leveluuid, leaderboard, SapphireCommError::NoStatsYet, nullptr, -1, 0, -1, 0, 0);
								}
							});
							break;
						}
					}
					break;
				}
				case SapphireComm::GetPlayerDemo: {
					SapphireUUID leveluuid;
					PlayerDemoId demoid;
					SapphireCommError error;
					if (!stream->deserialize<SapphireUUID>(leveluuid) || !stream->deserialize<PlayerDemoId>(demoid)
							|| !stream->deserialize<SapphireCommError>(error)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					switch (error) {
						case SapphireCommError::NoError: {
							uint32 randomseed;
							FixedString steps;
							if (!stream->deserialize<uint32>(randomseed)
									|| !stream->deserialize<SafeFixedString<SAPPHIRE_DEMO_MAX_LEN>>(steps)) {
								LOGI() << "Failed to read";
								goto exit_loop;
							}
							task.postTask([=] {
								for (auto&& l : playerDemoDownloadedEvents.foreach()) {
									l(leveluuid, demoid, error, steps, randomseed);
								}
							});
							break;
						}
						default: {
							task.postTask([=] {
								for (auto&& l : playerDemoDownloadedEvents.foreach()) {
									l(leveluuid, demoid, error, nullptr, 0);
								}
							});
							break;
						}
					}
					break;
				}
				case SapphireComm::Terminate: {
					writeWorker.post([=] {
						writeWorker.signalStop();
						auto&& eostream = *outputStream;
						eostream.serialize<SapphireComm>(SapphireComm::TerminateOk);
					});
					writeWorker.stop();
					goto exit_loop;
				}
				case SapphireComm::TerminateOk: {
					goto exit_loop;
				}
				case SapphireComm::PingRequest: {
					LOGI() << "Ping request";
					uint32 extra;
					if (!stream->deserialize<uint32>(extra)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					postPingResponse(extra);
					break;
				}
				case SapphireComm::PingResponse: {
					LOGI() << "Ping response";
					uint32 extra;
					if (!stream->deserialize<uint32>(extra)) {
						LOGI() << "Failed to read";
						goto exit_loop;
					}
					break;
				}
				default: {
					THROW() << "Unknown cmd: " << cmd;
					goto exit_loop;
				}
			}
		}
	}
	exit_loop:

	;
}

void CommunityConnection::postUpgradeStream() {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::UpgradeStream)) {
			return false;
		}
		uint8 servrdm[sizeof(SAPPHIRE_COMM_PV_KEY)];
		randomer->read(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY));
		if(!eostream.write(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY)) ) {
			return false;
		}

		for (int i = 0; i < sizeof(SAPPHIRE_COMM_PV_KEY); ++i) {
			servrdm[i] ^= SAPPHIRE_COMM_PV_KEY[i];
		}
		writeCipher.initCipher(servrdm, sizeof(SAPPHIRE_COMM_PV_KEY));
		outputStream = cipherOutputStream;
		return true;
	});
}

void CommunityConnection::postLogin() {
	postUpgradeStream();
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::Login)) {
			return false;
		}
		if (!eostream.serialize<SapphireUUID>(userUUID)) {
			return false;
		}
		return true;
	});
}
void CommunityConnection::postUpdatePlayerData(const FixedString& name, SapphireDifficulty diffcolor) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::UpdatePlayerData)) {
			return false;
		}
		if (!eostream.serialize<FixedString>(name)) {
			return false;
		}
		if (!eostream.serialize<SapphireDifficulty>(diffcolor)) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::postUploadLevel(const Level& level) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::UploadLevel)) {
			return false;
		}
		level.saveLevel(eostream, true);
		return true;
	});
}

void CommunityConnection::uploadLevel(const Level& level) {
	postUploadLevel(level);
}

void CommunityConnection::requestLevels() {
	uint32 detailscount = levelDetails.size();
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::GetLevels)) {
			return false;
		}
		if (!eostream.serialize<uint32>(detailscount)) {
			return false;
		}
		return true;
	});
}
void CommunityConnection::downloadLevel(const SapphireUUID& leveluuid) {
	ASSERT(scene->canDownloadLevels());
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::DownloadLevel)) {
			return false;
		}
		if (!eostream.serialize<SapphireUUID>(leveluuid)) {
			return false;
		}
		return true;
	});
}
void CommunityConnection::downloadLevel(const SapphireLevelDetails* details) {
	downloadLevel(details->uuid);
}

void CommunityConnection::rateLevel(const SapphireUUID& leveluuid, unsigned int rating) {
	ASSERT(rating >= 1 && rating <= 5) << rating;
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::RateLevel)) {
			return false;
		}
		if (!eostream.serialize<SapphireUUID>(leveluuid)) {
			return false;
		}
		if (!eostream.serialize<uint8>((uint8) rating)) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::sendMessage(const char* message, uint32 userid) {
	FixedString msg { message };
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::SendMessage)
				|| !eostream.serialize<FixedString>(msg)
				|| !eostream.serialize<uint32>(userid) ) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::requestCommunityNotifications(bool needs) {
	if (!needs) {
		onlineUsers.clear();
		messages.clear();
		messagesLocalStartIndex = 0;
		messagesRemoteCount = 0;
		messagesRemoteStartIndex = 0;
	}
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::CommunityNotifications)) {
			return false;
		}
		if (!eostream.serialize<bool>(needs)) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::addStateListener(LinkedNode<StateListener>& listener) {
	stateEvents += listener;
	if (isConnectionEstablished()) {
		listener.get()->onConnected(this);
		if (isLoggedIn()) {
			listener.get()->onLoggedIn(this);
		}
	}
}

void CommunityConnection::addStateListenerAndConnect(SapphireScene* scene, LinkedNode<StateListener>& listener) {
	stateEvents += listener;
	if (isConnectionEstablished()) {
		listener.get()->onConnected(this);
		if (isLoggedIn()) {
			listener.get()->onLoggedIn(this);
		}
	} else {
		connect(scene);
	}
}

void CommunityConnection::requestMessages(unsigned int start, unsigned int count) {
	if (messages.size() > 0) {
		if (start >= messagesLocalStartIndex) {
			unsigned int endindex = messagesLocalStartIndex + messages.size();
			if (start <= endindex) {
				unsigned int diff = endindex - start;
				if (diff >= count) {
					return;
				}
				start += diff;
				count -= diff;
			} else {
				//request the gap too
				requestMessages(endindex, start - endindex);
			}
		} else {
			if (start + count < messagesLocalStartIndex) {
				//request the gap too
				requestMessages(start + count, messagesLocalStartIndex - (start + count));
			}
		}
	}
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::QueryMessages)) {
			return false;
		}
		if (!eostream.serialize<uint32>(start) || !eostream.serialize<uint32>(count)) {
			return false;
		}
		return true;
	});
}

bool CommunityConnection::saveRegistrationToken(const RegistrationToken& regtoken) {
	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(scene->getRegistrationTokenFile().openOutputStream());
	return ostream.serialize<RegistrationToken>(regtoken);
}


bool CommunityConnection::reportLevel(const SapphireUUID& leveluuid, const FixedString& reason) {
	return writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::ReportLevel)
				|| !eostream.serialize<SapphireUUID>(leveluuid)
				|| !eostream.serialize<FixedString>(reason)) {
			return false;
		}
		return true;
	});
}

bool CommunityConnection::showCommunityInformationDialog(SapphireCommunityInformation info, SapphireUILayer* parent) {
	switch (info) {
		case SapphireCommunityInformation::NewVersionRequired: {
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
			{
				if (auto* apps = SteamApps()) {
					apps->MarkContentCorrupt(false);
				}
			}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
			DialogLayer* dialog = new DialogLayer(parent);
			dialog->setTitle("New version");
			dialog->addDialogItem(new TextDialogItem("A newer version of " SAPPHIRE_GAME_NAME " is available! "
			"Please download it to continue using the Community Hub."));
			dialog->addDialogItem(new EmptyDialogItem(0.5f));
			dialog->addDialogItem(new CommandDialogItem("Okay", [=] {
				dialog->dismiss();
			}));
			dialog->show(parent->getScene(), true);

			break;
		}
		case SapphireCommunityInformation::Maintenance: {
			DialogLayer* dialog = new DialogLayer(parent);
			dialog->setTitle("Maintenance");
			dialog->addDialogItem(new TextDialogItem("Community Hub is under maintenance. Please check back later."));
			dialog->addDialogItem(new EmptyDialogItem(0.5f));
			dialog->addDialogItem(new CommandDialogItem("Okay", [=] {
				dialog->dismiss();
			}));
			dialog->show(parent->getScene(), true);

			break;
		}
		default: {
			return false;
		}
	}
	return true;
}

void CommunityConnection::postPingRequest(uint32 extra) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		eostream.serialize<SapphireComm>(SapphireComm::PingRequest);
		eostream.serialize<uint32>(extra);
	});
}
void CommunityConnection::postPingResponse(uint32 extra) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		eostream.serialize<SapphireComm>(SapphireComm::PingResponse);
		eostream.serialize<uint32>(extra);
	});
}

void CommunityConnection::sendLinkIdentifier(uint32 identifier) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		eostream.serialize<SapphireComm>(SapphireComm::LinkAccIdentifier);
		eostream.serialize<uint32>(identifier);
	});
}

void CommunityConnection::notifyLevelProgress(ProgressSynchId progressid, const SapphireUUID& leveluuid, const FixedString& steps,
		uint32 randomseed, SapphireLevelCommProgress progress) {
	if (isLoggedIn()) {
		if (serverProgressSynchId == progressid) {
			postLevelProgress(progressid, leveluuid, steps, randomseed, progress);
		}
	}
}
void CommunityConnection::postLevelProgress(ProgressSynchId progressid, const SapphireUUID& leveluuid, const FixedString& steps,
		uint32 randomseed, SapphireLevelCommProgress progress) {
	writeWorker.post([=] {
		LOGI() << "Write level progress " << progressid;

		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::LevelProgress)
				|| !eostream.serialize<SapphireLevelCommProgress>(progress)
				|| !eostream.serialize<ProgressSynchId>(progressid)
				|| !eostream.serialize<SapphireUUID>(leveluuid)
				|| !eostream.serialize<FixedString>(steps)
				|| !eostream.serialize<uint32>(randomseed)) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::checkPostLevelProgress() {
	if (serverProgressSynchId > scene->getProgressSynchId()) {
		scene->updateServerProgressSynchId(serverProgressSynchId);
	} else if (serverProgressSynchId < scene->getProgressSynchId()) {
		SapphireLevelCommProgress progress;
		SapphireUUID uuid;
		FixedString steps;
		uint32 random;
		bool res = scene->getLevelProgress(serverProgressSynchId, &uuid, &steps, &random, &progress);
		WARN(res) << "Failed to get level progress: " << serverProgressSynchId;
		if (res) {
			postLevelProgress(serverProgressSynchId, uuid, steps, random, progress);
		}
	}
}

void CommunityConnection::sendLinkIdentifierRequest() {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::LinkAccRequest) ) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::sendLinkCancel() {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::LinkCancel) ) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::sendLinkRemoteIdentifier(uint32 identifier) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::LinkAccIdentifier)
				|| !eostream.serialize<uint32>(identifier)) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::sendGetStatistics(const SapphireUUID& level) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::GetStatistics)
				|| !eostream.serialize<SapphireUUID>(level)) {
			return false;
		}
		return true;
	});
}

void CommunityConnection::sendGetLeaderboards(const SapphireUUID& level, SapphireLeaderboards leaderboard) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::GetLeaderboard)
				|| !eostream.serialize<SapphireUUID>(level)
				|| !eostream.serialize<SapphireLeaderboards>(leaderboard)) {
			return false;
		}
		return true;
	});
}
void CommunityConnection::sendGetDemo(const SapphireUUID& leveluuid, PlayerDemoId demoid) {
	writeWorker.post([=] {
		auto&& eostream = *outputStream;
		if (!eostream.serialize<SapphireComm>(SapphireComm::GetPlayerDemo)
				|| !eostream.serialize<SapphireUUID>(leveluuid)
				|| !eostream.serialize<PlayerDemoId>(demoid)) {
			return false;
		}
		return true;
	});
}

} // namespace userapp

