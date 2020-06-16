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
 * MonotonicTime.h
 *
 *  Created on: 2016. febr. 28.
 *      Author: sipka
 */

#ifndef MONOTONICTIME_H_
#define MONOTONICTIME_H_

#include <framework/utils/BasicGlobalListener.h>

#include <gen/configuration.h>

namespace rhfw {
class platform_bridge;
namespace core {

template<int EXP_COUNT>
class basic_time {
private:
	typedef basic_time<EXP_COUNT> self_type;

	template<int NUM, int TOEXP>
	class exponential {
		static_assert(TOEXP > 0, "exponential is less than zero");
	public:
		static constexpr int value = NUM * exponential<NUM, TOEXP - 1>::value;
	};
	template<int NUM>
	class exponential<NUM, 0> {
	public:
		static constexpr int value = 1;
	};

	template<int FROM_EXPONENTIAL, int CAST_EXPONENTIAL, bool divide>
	class castor;

	template<int FROM_EXPONENTIAL, int CAST_EXPONENTIAL>
	class castor<FROM_EXPONENTIAL, CAST_EXPONENTIAL, true> {
	public:
		static basic_time<CAST_EXPONENTIAL> get(const basic_time<FROM_EXPONENTIAL>& v) {
			return basic_time<CAST_EXPONENTIAL> { (v.value) / exponential<10, CAST_EXPONENTIAL - FROM_EXPONENTIAL>::value };
		}
	};
	template<int FROM_EXPONENTIAL, int CAST_EXPONENTIAL>
	class castor<FROM_EXPONENTIAL, CAST_EXPONENTIAL, false> {
	public:
		static basic_time<CAST_EXPONENTIAL> get(const basic_time<FROM_EXPONENTIAL>& v) {
			return basic_time<CAST_EXPONENTIAL> { (v.value) * exponential<10, FROM_EXPONENTIAL - CAST_EXPONENTIAL>::value };
		}
	};
	template<int EXP, bool divide>
	class castor<EXP, EXP, divide> {
	public:
		static const basic_time<EXP>& get(const basic_time<EXP>& v) {
			return v;
		}
	};

	template<int FEXP>
	friend class basic_time;

	long long value;
public:
	explicit basic_time(long long value = 0)
			: value { value } {
	}
	basic_time(const self_type& o) = default;
	self_type& operator=(const self_type& o) = default;

	template<int SRC_EXP>
	basic_time(const basic_time<SRC_EXP>& o)
			: value { (long long) castor<SRC_EXP, EXP_COUNT, (EXP_COUNT > SRC_EXP)>::get(o) } {
	}

	explicit operator long long() const {
		return value;
	}
	explicit operator unsigned long long() const {
		return (unsigned long long) value;
	}
	explicit operator long() const {
		return (long) value;
	}
	explicit operator unsigned long() const {
		return (unsigned long) value;
	}
	explicit operator int() const {
		return (int) value;
	}
	explicit operator unsigned int() const {
		return (unsigned int) value;
	}

	/*template<int CAST_EXP>
	 operator basic_time<CAST_EXP>() const {
	 return castor<CAST_EXP, (CAST_EXP > EXP_COUNT)>::get(*this);
	 }*/

	bool operator==(const self_type& t) const {
		return value == t.value;
	}
	bool operator!=(const self_type& t) const {
		return value != t.value;
	}
	bool operator>=(const self_type& t) const {
		return value >= t.value;
	}
	bool operator<=(const self_type& t) const {
		return value <= t.value;
	}
	bool operator<(const self_type& t) const {
		return value < t.value;
	}
	bool operator>(const self_type& t) const {
		return value > t.value;
	}

	self_type operator+(const self_type& t) const {
		return self_type { value + t.value };
	}
	self_type operator-(const self_type& t) const {
		return self_type { value - t.value };
	}

	double operator/(const self_type& t) const {
		return (double) value / t.value;
	}

	self_type operator*(double v) const {
		return self_type { (long long) (value * v) };
	}
	self_type operator/(double v) const {
		return self_type { (long long) (value / v) };
	}

	self_type& operator +=(const self_type& t) {
		this->value += t.value;
		return *this;
	}
	self_type& operator -=(const self_type& t) {
		this->value -= t.value;
		return *this;
	}
};

typedef basic_time<0> time_seconds;
typedef basic_time<-3> time_millis;
typedef basic_time<-6> time_micros;

class TimeListener: public LinkedNode<TimeListener> {
private:
	TimeListener* get() override {
		return this;
	}
public:
	using LinkedNode<TimeListener>::LinkedNode;
	virtual void onTimeChanged(const time_millis& time, const time_millis& previous) {
	}

	bool isSubscribed() const {
		return this->isInList();
	}

	void unsubscribe() {
		this->removeLinkFromList();
	}
};

class MonotonicTime {
private:
	friend class GlobalMonotonicTimeListener;

	static time_micros current;
	static time_micros previous;

	MonotonicTime() {
	}
public:
	static time_micros getCurrent() {
		return current;
	}
	static time_micros getPrevious() {
		return previous;
	}

};

class GlobalMonotonicTimeListener: public BasicGlobalListener<TimeListener> {
private:
	friend class ::rhfw::platform_bridge;

	static void setCurrent(time_micros time) {
		MonotonicTime::previous = MonotonicTime::current;
		MonotonicTime::current = time;

		for (auto&& l : pointers()) {
			if (l != nullptr) {
				l->onTimeChanged(time, MonotonicTime::previous);
			}
		}
	}
public:
};

} // namespace core
} // namespace rhfw
#endif /* MONOTONICTIME_H_ */
