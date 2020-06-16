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
#include <sapphire/community/nslookup.h>
#include <gen/platform.h>

#if defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_WINDOWSSTORE)
#include <ws2tcpip.h>

namespace userapp {

using namespace rhfw;

bool lookupIPv4Address(const char *domain, IPv4Address* out){
	LOGI() << "Query DNS: " << domain;
	struct addrinfo *res = nullptr;
	struct addrinfo *ptr = nullptr;
	struct addrinfo hints { };
	hints.ai_family = AF_INET;

	DWORD errcode = getaddrinfo (domain, NULL, &hints, &res);
	if (errcode != 0) {
		LOGE() << "DNS query failed: " << errcode;
		return false;
	}
	if (res == nullptr) {
		LOGE() << "DNS query failed, null results: " << domain;
		return false;
	}
	if(res->ai_family != AF_INET){
		LOGE() << "DNS query failed, non-ipv4 address: " << res->ai_family;
		freeaddrinfo(res);
		return false;
	}
	*out = IPv4Address { ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr };

	LOGI() << "DNS lookup result: " << domain << " -> " << *out;

	freeaddrinfo(res);
	return true;
}

}  // namespace userapp
#endif /* defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_WINDOWSSTORE) */

#if defined(RHFW_PLATFORM_LINUX) || defined(RHFW_PLATFORM_ANDROID) || defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_IOS)
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
namespace userapp {

using namespace rhfw;

bool lookupIPv4Address(const char *domain, IPv4Address* out) {
	LOGI() << "Query DNS: " << domain;
	struct addrinfo hints { };
	struct addrinfo *res = nullptr;
	hints.ai_family = AF_INET;

	int errcode = getaddrinfo (domain, NULL, &hints, &res);
	if (errcode != 0) {
		LOGE() << "DNS query failed: " << errcode;
		return false;
	}
	if (res == nullptr) {
		LOGE() << "DNS query failed, null results: " << domain;
		return false;
	}
	if(res->ai_family != AF_INET){
		LOGE() << "DNS query failed, non-ipv4 address: " << res->ai_family;
		freeaddrinfo(res);
		return false;
	}
	*out = IPv4Address { ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr };

	LOGI() << "DNS lookup result: " << domain << " -> " << *out;

	freeaddrinfo(res);
	return true;
}

}  // namespace userapp
#endif /* defined(RHFW_PLATFORM_LINUX) || defined(RHFW_PLATFORM_ANDROID) || defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_IOS) */

