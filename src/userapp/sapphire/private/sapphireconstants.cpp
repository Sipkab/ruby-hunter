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
 * sapphireconstants.cpp
 *
 *  Created on: 2016. okt. 16.
 *      Author: sipka
 */

#include <sapphire/sapphireconstants.h>

namespace userapp {

/*
 * A constant key that is shared with the community server of sapphire.
 * It is somewhat used to encrypt the communication, but not to provide security
 * as it is present in the released binaries of the game.
 *
 * The actual key is not uploaded to version control but only used when compiling release
 * versions of the game.
 *
 * In theory, you could go ahead and find this key in the binaries, and initiate connections
 * to the community server, doing some funky stuff, but please don't.
 * The server doesn't contain anything apart from some game levels, leaderboards, and statistics.
 * The server itself is strictly isolated from other services so there's barely any reason to hack into it.
 * There's nothing to gain when attempting to hack the server but only to ruin the joy of others.
 * I'll gladly give you a tour about the server config if you're interested.
 */
extern const rhfw::uint8 SAPPHIRE_COMM_PV_KEY[256] = { 0 };

}  // namespace userapp
