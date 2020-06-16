#ifndef __RHFW__RANGE__ITERATOR__H__
#define __RHFW__RANGE__ITERATOR__H__
template<typename T, unsigned int START, unsigned int END>
class __range_iterator {
	static_assert(END >= START, "Invalid enumerator values");

	unsigned int value;
public:
	static const unsigned int START_VALUE = START;
	static const unsigned int END_VALUE = END;

	__range_iterator(unsigned int value = START)
			: value { value } {
	}

	bool operator ==(const __range_iterator& o) const {
		return value == o.value;
	}
	bool operator !=(const __range_iterator& o) const {
		return value != o.value;
	}
	__range_iterator& operator++() {
		++value;
		return *this;
	}
	__range_iterator& operator--() {
		--value;
		return *this;
	}

	__range_iterator operator+(const __range_iterator& o) const {
		return {value + o.value};
	}
	__range_iterator operator-(const __range_iterator& o) const {
		return {value - o.value};
	}

	__range_iterator& operator-=(const __range_iterator& o) {
		this->value -= o.value;
		return *this;
	}
	__range_iterator& operator+=(const __range_iterator& o) {
		this->value += o.value;
		return *this;
	}

	T operator*() const {
		ASSERT(value >= START && value < END) << "Iterator out of range: " << value << ", range: [" << START << " -  << "<< END << "]";
		return (T) value;
	}
};

template<typename T, unsigned int START, unsigned int END>
class __enumerator {
	static_assert(END >= START, "Invalid enumerator values");
public:
	__range_iterator <T, START, END> begin() const {
		return {START};
	}
	__range_iterator <T, START, END> end() const {
		return {END};
	}

	unsigned int count() const {
		return END - START;
	}
};

template<typename ITType, typename ... T>
class __multi_enumerator {
	class __varrange {
	public:
		unsigned int start;
		unsigned int end;
	};
	class __varrange_iterator {
		unsigned int value;
		const __varrange* range;

		void gonext() {
			do {
				++value;
				if(value == range->end) {
					++range;
					value = range->start;
				}
			}while(value >= range->end);
		}
	public:

		__varrange_iterator (const __varrange* range) : range {range} {
			value = range->start - 1;
			gonext();
		}

		bool operator ==(const __varrange_iterator& o) const {
			return value == o.value && range == o.range;
		}
		bool operator !=(const __varrange_iterator& o) const {
			return value != o.value || range != o.range;
		}
		__varrange_iterator& operator++() {
			gonext();
			return *this;
		}
		ITType operator*() const {
			return (ITType) value;
		}
	};

	template<typename ... Tail>
	class for_every {
	public:
		static void execute(__varrange* it) {
		}
	};

	template<typename Head, typename ... Tail>
	class for_every<Head, Tail...> {
	public:
		static void execute(__varrange* it) {
			it->start = Head::START_VALUE;
			it->end = Head::END_VALUE;
			for_every<Tail...>::execute(it + 1);
		}
	};

	__varrange all[sizeof...(T) + 1];
public:
	__multi_enumerator() {
		for_every<T...>::execute(all);

		//sentinel
		all[sizeof...(T)].start = 0;
		all[sizeof...(T)].end = 2;
	}

	__varrange_iterator begin() {
		return {all};
	}
	__varrange_iterator end() {
		return {all + sizeof...(T)};
	}
};

#endif
