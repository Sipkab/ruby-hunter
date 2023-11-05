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

#include <sapphire/common/FantasyNames.h>
#include <sapphire/sapphireconstants.h>

namespace userapp {

static const uint64 TRAIT_COUNT = 16;
static const char* Traits[] {
	"Lucky",
	"Busy",
	"Friendly",
	"Kind",
	"Brave",
	"Calm",
	"Funny",
	"Strong",
	"Fast",
	"Wise",
	"Bold",
	"Clever",
	"Humble",
	"Nice",
	"Fit",
	"Polite",
};
static_assert(sizeof(Traits)/sizeof(Traits[0]) == TRAIT_COUNT, "Traits size mismatch");

static const uint64 SUBJECT_COUNT = 16;
static const char* Subjects[] {
	"Miner",
	"Bug",
	"YamYam",
	"Robot",
	"Lorry",
	"Bomb",
	"Safe",
	"Bag",
	"Key",
	"Door",
	"Wheel",
	"Emerald",
	"Ruby",
	"Sapphire",
	"Citrine",
	"Laser",
};
static_assert(sizeof(Subjects)/sizeof(Subjects[0]) == SUBJECT_COUNT, "Subjects size mismatch");

static const uint64 SEPARATOR_COUNT = 4;
static const char* Separators[] {
	"",
	"_",
	".",
	"-",
};
static_assert(sizeof(Separators)/sizeof(Separators[0]) == SEPARATOR_COUNT, "Separators size mismatch");

static const uint64 PREFIX_COUNT = 4;
static const char* Prefixes[] {
	"",
	"A",
	"The",
	"One",
};
static_assert(sizeof(Prefixes)/sizeof(Prefixes[0]) == PREFIX_COUNT, "Prefixes size mismatch");

static const uint64 SUFFIX_COUNT = 32;
static const char* Suffixes[] {
	"",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"X",
	"Y",
	"Z",
	"#1",
	"#2",
	"#3",
	"#4",
	"#5",
	"#6",
	"#7",
	"#8",
	"#9",
	"$1",
	"$2",
	"$3",
	"$4",
	"$5",
	"$6",
	"$7",
	"$8",
	"$9",
	"_",
};
static_assert(sizeof(Suffixes)/sizeof(Suffixes[0]) == SUFFIX_COUNT, "Suffixes size mismatch");

FixedString generateFantasyName(const SapphireUUID& userid) {
	char buffer[SAPPHIRE_USERNAME_MAX_LEN + 1] = "";

	uint64 lower64Bit = userid.lower64bit();

	const char* trait = Traits[(lower64Bit >> 0) % TRAIT_COUNT];
	const char* subject = Subjects[(lower64Bit >> 4) % TRAIT_COUNT];
	const char* prefix = Prefixes[(lower64Bit >> 8) % PREFIX_COUNT];
	const char* separator = Separators[(lower64Bit >> 10) % SEPARATOR_COUNT];
	const char* prefixseparator = prefix[0] == '\0' ? "" : separator;
	const char* suffix = Suffixes[(lower64Bit >> 12) % SUFFIX_COUNT];

	snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s", prefix, prefixseparator, trait, separator, subject, suffix);

	return buffer;
}

}  // namespace userapp
