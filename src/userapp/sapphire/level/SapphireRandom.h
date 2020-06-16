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
 * SapphireRandom.h
 *
 *  Created on: 2016. jan. 17.
 *      Author: sipka
 */

#ifndef SAPPHIRERANDOM_H_
#define SAPPHIRERANDOM_H_

#include <gen/configuration.h>
#include <gen/log.h>

namespace userapp {
using namespace rhfw;
class SapphireRandom {
private:
	unsigned int seed = 0;

	unsigned int next() {
		//0042CD74 random generalasa

		//Address   Hex dump            Command							//Comments
		//00438E40  /$  8B81 C8000000   MOV EAX,DWORD PTR DS:[ECX+0C8]	;sapphire.00438E40(guessed Arg1) //EAX = seed
		//00438E46  |.  33D2            XOR EDX,EDX						//EDX = 0
		//00438E48  |.  69C0 05840808   IMUL EAX,EAX,8088405			//EAX *= 0x8088405
		//00438E4E  |.  40              INC EAX							//++EAX
		//00438E4F  |.  8981 C8000000   MOV DWORD PTR DS:[ECX+0C8],EAX	//seed = EAX
		//00438E55  |.  C1E8 10         SHR EAX,10						//EAX >>= 0x10
		//00438E58  |.  F77424 04       DIV DWORD PTR SS:[ARG.1]		//modulo, eredmenye EDX, arg1-ben kapnank a modulo-t
		//00438E5C  |.  8BC2            MOV EAX,EDX						//EAX a visszateresi ertek, EAX = EDX
		//00438E5E  \.  C2 0400         RETN 4							//return
		this->seed = seed * 0x8088405 + 1;

		return seed >> 0x10;
	}
public:

	explicit SapphireRandom(unsigned int seed = 0)
			: seed { seed } {
	}

	unsigned int getSeed() const {
		return seed;
	}

	unsigned int next(unsigned int mod) {
		unsigned int result = next() % mod;
		//LOGI("Random %% %u = %u, seed: %X", mod, result, this->seed);
		return result;
	}

	bool nextBool() {
		return next(2) != 0;
	}

	float nextFloat() {
		return next(0xFFFF) / (float) 0xFFFF;
	}

	void setSeed(unsigned int seed) {
		this->seed = seed;
	}
};

}

#endif /* SAPPHIRERANDOM_H_ */
