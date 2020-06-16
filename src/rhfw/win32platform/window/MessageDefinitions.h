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
#ifndef WIN32MESSAGEDEFINITIONS_H_
#define WIN32MESSAGEDEFINITIONS_H_

#include <framework/utils/utility.h>

#define WM_USER_MESSAGE				(WM_USER)

#define WM_USER_RECREATE_WINDOW		(WM_USER + 1)

//lParam points to a WinMessageRunnable class
#define WM_USER_EXECUTE				(WM_USER + 2)

#define WM_USER_DESTROY_WINDOW		(WM_USER + 3)

#define WM_USER_CURSOR_VISIBILITY	(WM_USER + 4)

#define WM_USER_CURSOR_CLIPPING		(WM_USER + 5)

class WinMessageRunnable {
public:
	template<typename Lambda>
	static WinMessageRunnable* from(Lambda&& l) {
		class LambdaRun: public WinMessageRunnable {
		public:
			typename rhfw::util::remove_reference<Lambda>::type lambda;

			LambdaRun(Lambda&& l)
					: lambda(rhfw::util::forward<Lambda>(l)) {
			}

			virtual void operator()() override {
				lambda();
				delete this;
			}
		};
		return new LambdaRun(rhfw::util::forward<Lambda>(l));
	}
	virtual ~WinMessageRunnable() {
	}
	virtual void operator()() = 0;
};

#endif /* WIN32MESSAGEDEFINITIONS_H_ */
