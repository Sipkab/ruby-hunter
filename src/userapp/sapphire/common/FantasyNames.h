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

#ifndef SAPPHIRE_FANTASY_NAMES_H_
#define SAPPHIRE_FANTASY_NAMES_H_

#include <gen/fwd/types.h>
#include <framework/utils/FixedString.h>
#include <sapphire/level/SapphireUUID.h>

namespace userapp {
using namespace rhfw;

FixedString generateFantasyName(const SapphireUUID& userid);

}  // namespace userapp

#endif /* SAPPHIRE_FANTASY_NAMES_H_ */
