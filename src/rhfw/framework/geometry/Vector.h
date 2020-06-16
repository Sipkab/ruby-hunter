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
 * Vector.h
 *
 *  Created on: 2015 febr. 11
 *      Author: sipka
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include <gen/configuration.h>
#include <gen/serialize.h>
#include <framework/utils/utility.h>
#include <gen/log.h>

#include <math.h>

namespace rhfw {

namespace vector_internals {

template<unsigned int Index, unsigned int ... IDims>
struct indexer;
template<unsigned int Index, unsigned int Head, unsigned int ... Tail>
struct indexer<Index, Head, Tail...> : public indexer<Index - 1, Tail...> {
};
template<unsigned int Head, unsigned int ... Tail>
struct indexer<0, Head, Tail...> {
	static const unsigned int value = Head;
};

template<typename Operation, unsigned int Index, unsigned int End>
struct Foreach {
	template<typename ... Args>
	inline static void execute(Args&&... args) {
		Operation::template execute<Index>(util::forward<Args>(args)...);
		Foreach<Operation, Index + 1, End>::execute(util::forward<Args>(args)...);
	}
};
template<typename Operation, unsigned int End>
struct Foreach<Operation, End, End> {
	template<typename ... Args>
	inline static void execute(Args&&... args) {
	}
};
template<typename Operation, unsigned int Index, unsigned int End>
struct BreakingForeach {
	template<typename ... Args>
	inline static bool execute(Args&&... args) {
		bool res = Operation::template execute<Index>(util::forward<Args>(args)...);
		if (!res)
			return res;
		return BreakingForeach<Operation, Index + 1, End>::execute(util::forward<Args>(args)...);
	}
};
template<typename Operation, unsigned int End>
struct BreakingForeach<Operation, End, End> {
	template<typename ... Args>
	inline static bool execute(Args&&... args) {
		return true;
	}
};
}  // namespace vector_internals

using namespace vector_internals;

template<unsigned int Dimension, typename Type>
class vector_data_container;
template<typename Type, unsigned int DataDims, unsigned int ... Dims>
class vector_ref_container;
template<typename Container>
class VectorBase;

template<typename Type, unsigned int DataDims, unsigned int ... Dims>
class vector_ref_container {
public:
	static const unsigned int DIMENSION = sizeof...(Dims);
	typedef Type component_type;
private:
	typedef vector_ref_container<component_type, DataDims, Dims...> self_type;

	vector_data_container<DataDims, component_type>& data;
public:
	static const bool IS_DATA_CONTAINER = false;

	template<unsigned int Index>
	component_type& component() {
		static_assert(Index < DIMENSION,"Invalid index");
		return data.template component<indexer<Index, Dims...>::value>();
	}
	template<unsigned int Index>
	const component_type& component() const {
		static_assert(Index < DIMENSION,"Invalid index");
		return data.template component<indexer<Index, Dims...>::value>();
	}

	vector_ref_container(vector_data_container<DataDims, Type>& data)
			: data(data) {
	}
	vector_ref_container(const self_type& o)
			: data(o.data) {
	}
	template<typename VType, unsigned int VDataDims, unsigned int ... VDims>
	self_type& operator=(const vector_ref_container<VType, VDataDims, VDims...>&) = delete;
};
template<typename Type, unsigned int DataDims, unsigned int ... Dims>
class vector_ref_container<const Type, DataDims, Dims...> {
public:
	static const unsigned int DIMENSION = sizeof...(Dims);
	typedef const Type component_type;
private:
	typedef vector_ref_container<component_type, DataDims, Dims...> self_type;

	const vector_data_container<DataDims, Type>& data;
public:
	static const bool IS_DATA_CONTAINER = false;

	template<unsigned int Index>
	component_type& component() {
		static_assert(Index < DIMENSION,"Invalid index");
		return data.template component<indexer<Index, Dims...>::value>();
	}
	template<unsigned int Index>
	const component_type& component() const {
		static_assert(Index < DIMENSION,"Invalid index");
		return data.template component<indexer<Index, Dims...>::value>();
	}

	vector_ref_container(const vector_data_container<DataDims, Type>& data)
			: data(data) {
	}
	vector_ref_container(const self_type& o)
			: data(o.data) {
	}
	template<typename VType, unsigned int VDataDims, unsigned int ... VDims>
	self_type& operator=(const vector_ref_container<VType, VDataDims, VDims...>&) = delete;
};

template<unsigned int Dimension, typename Type>
class vector_data_container {
public:
	static const unsigned int DIMENSION = Dimension;
	typedef Type component_type;
private:
	typedef vector_data_container<Dimension, Type> self_type;
public:
	static const bool IS_DATA_CONTAINER = true;

	template<typename CType>
	struct Setter {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() = (CType) util::forward<V2>(odata).template component<Index>();
		}
	};

	template<typename T>
	struct DimGetter {
		template<typename U, unsigned int Value = U::DIMENSION> static constexpr unsigned int Test(int*) {
			return Value;
		}
		template<typename U> static constexpr unsigned int Test(void*) {
			return 1;
		}

		static const unsigned int value = Test<typename util::remove_reference<T>::type>(static_cast<int*>(nullptr));
	};

	template<typename ArgType, unsigned int Index>
	struct SetExecutor {
		struct Setter {
			template<unsigned int SetIndex>
			inline static void execute(self_type& data, const ArgType& v) {
				static_assert(SetIndex - Index < ArgType::DIMENSION, "Invalid index");
				data.template component<SetIndex>() = (Type) v.template component<SetIndex - Index>();
			}
		};

		template<typename U = ArgType, typename util::enable_if<(DimGetter<U>::value > 1)>::type* = nullptr>
		inline static void execute(self_type& data, const ArgType& v) {
			Foreach<Setter, Index, Index + DimGetter<U>::value>::execute(data, v);
		}
		template<typename U = ArgType, typename util::enable_if<(DimGetter<U>::value == 1)>::type* = nullptr>
		inline static void execute(self_type& data, const ArgType& v) {
			data.template component<Index>() = (Type) v;
		}
	};

	template<unsigned int Index, typename ... Args>
	struct MultiSetter;

	template<unsigned int Index, typename Head, typename ... Tail>
	struct MultiSetter<Index, Head, Tail...> {
		inline static void execute(self_type& data, Head&& h, Tail&&... t) {
			typedef SetExecutor<typename util::remove_reference<typename util::remove_const<Head>::type>::type, Index> settertype;
			settertype::execute(data, util::forward<Head>(h));
			MultiSetter<Index + DimGetter<Head>::value, Tail...>::execute(data, util::forward<Tail>(t)...);
		}
	};
	template<unsigned int Index>
	struct MultiSetter<Index> {
		//for example for Vec3 { Vec2, float} or Vec3 { float, float, float}, or Vec3{ float, Vec2} etc...
		static_assert(Index == DIMENSION || Index == 0, "Either specify 0 arguments at constructor, or pass at least DIMENSION component arguments");
		inline static void execute(self_type& data) {
		}
	};

	template<unsigned int Count, typename ... Args>
	struct DimCounter;

	template<unsigned int Count, typename Head, typename ... Tail>
	struct DimCounter<Count, Head, Tail...> : public DimCounter<Count + DimGetter<Head>::value, Tail...> {
	};
	template<unsigned int Count>
	struct DimCounter<Count> {
		static const unsigned int value = Count;
	};

	Type data[Dimension];
public:
	template<unsigned int Index>
	Type& component() {
		static_assert(Index < Dimension,"Invalid index");
		return data[Index];
	}
	template<unsigned int Index>
	const Type& component() const {
		static_assert(Index < Dimension,"Invalid index");
		return data[Index];
	}

	//TODO we should use char arrays as data container, and initialize with placement new, and manually destruct

	//members default initialized
	//we cant have enable_if here, or MSVC will break :(
	template<typename ... Args>
	vector_data_container(Args&& ... args) {
		MultiSetter<0, Args...>::execute(*this, util::forward<Args>(args)...);
	}

	vector_data_container(const self_type&) = default;
	vector_data_container(self_type&&) = default;
	self_type& operator=(const self_type&) = default;
	self_type& operator=(self_type&&) = default;

	explicit operator Type*() {
		return data;
	}
	explicit operator const Type*() const {
		return data;
	}

	Type& operator[](unsigned int index) {
		ASSERT(index < Dimension) << index;
		return data[index];
	}
	const Type& operator[](unsigned int index) const {
		ASSERT(index < Dimension) << index;
		return data[index];
	}
};

template<typename Container>
class VectorBase: public Container {
private:
	typedef VectorBase<Container> self_type;
	using self_component_type = typename Container::component_type;

	template<unsigned int Dim>
	using data_type = VectorBase<vector_data_container<Dim, self_component_type>>;

	template<unsigned int ... Dims>
	using ref_type = VectorBase<vector_ref_container<self_component_type, Container::DIMENSION, Dims...>>;
	template<unsigned int ... Dims>
	using const_ref_type = VectorBase<vector_ref_container<const self_component_type, Container::DIMENSION, Dims...>>;

	using self_data_type = data_type<Container::DIMENSION>;

	template<unsigned int ... Vals>
	struct MaxGetter;
	template<unsigned int Current, unsigned int Head, unsigned int ... Tail>
	struct MaxGetter<Current, Head, Tail...> {
		static const unsigned int value = Current > MaxGetter<Head, Tail...>::value ? Current : MaxGetter<Head, Tail...>::value;
	};
	template<unsigned int Current>
	struct MaxGetter<Current> {
		static const unsigned int value = Current;
	};

	struct EqualityTester {
		template<unsigned int Index, typename V1, typename V2>
		inline static bool execute(bool& res, V1&& data, V2&& odata) {
			res = util::forward<V1>(data).template component<Index>() == util::forward<V2>(odata).template component<Index>();
			return res;
		}
	};

	struct Setter {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() = util::forward<V2>(odata).template component<Index>();
		}
	};
	struct SetMult {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() *= util::forward<V2>(odata).template component<Index>();
		}
	};
	struct SetDiv {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() /= util::forward<V2>(odata).template component<Index>();
		}
	};
	struct SetAdd {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() += util::forward<V2>(odata).template component<Index>();
		}
	};
	struct SetSub {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() -= util::forward<V2>(odata).template component<Index>();
		}
	};

	struct SingleSetMult {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() *= util::forward<V2>(odata);
		}
	};
	struct SingleSetDiv {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() /= util::forward<V2>(odata);
		}
	};
	struct SingleSetAdd {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() += util::forward<V2>(odata);
		}
	};
	struct SingleSetSub {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() -= util::forward<V2>(odata);
		}
	};

	struct Summarize {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data) += util::forward<V2>(odata).template component<Index>();
		}
	};
	struct UnaryMinus {
		template<unsigned int Index, typename V1, typename V2>
		inline static void execute(V1&& data, V2&& odata) {
			util::forward<V1>(data).template component<Index>() = -util::forward<V2>(odata).template component<Index>();
		}
	};
public:
	template<typename ... Args>
	VectorBase(Args&& ... args)
			: Container(util::forward<Args>(args)...) {
	}
	template<typename Args>
	VectorBase& operator=(Args&& args) {
		Container::operator=(util::forward<Args>(args));
		return *this;
	}

	template<unsigned int ... Comps>
	ref_type<Comps...> get() {
		static_assert(MaxGetter<Comps...>::value < Container::DIMENSION, "Dimension is out of bounds");
		return ref_type<Comps...> { *this };
	}
	template<unsigned int ... Comps>
	const_ref_type<Comps...> get() const {
		static_assert(MaxGetter<Comps...>::value < Container::DIMENSION, "Dimension is out of bounds");
		return const_ref_type<Comps...> { *this };
	}

	//assignment functions
	//ref to ref assignment is implemented by converting the right operant to data
	template<typename VContainer, typename util::enable_if<VContainer::IS_DATA_CONTAINER || Container::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator=(const VectorBase<VContainer>& o) {
		Foreach<Setter, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	template<typename VType, unsigned int RefDim, unsigned int ... Dims, typename SelfContainer = Container, typename util::enable_if<
			!SelfContainer::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator=(const vector_ref_container<VType, RefDim, Dims...>& o) {
		Foreach<Setter, 0, Container::DIMENSION>::execute(*this,
				vector_data_container<vector_ref_container<VType, RefDim, Dims...>::DIMENSION, typename util::remove_const<VType>::type> { o });
		return *this;
	}

	template<typename VContainer, typename util::enable_if<VContainer::IS_DATA_CONTAINER || Container::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator*=(const VectorBase<VContainer>& o) {
		Foreach<SetMult, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	template<typename VType, unsigned int RefDim, unsigned int ... Dims, typename SelfContainer = Container, typename util::enable_if<
			!SelfContainer::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator*=(const vector_ref_container<VType, RefDim, Dims...>& o) {
		Foreach<SetMult, 0, Container::DIMENSION>::execute(*this,
				vector_data_container<vector_ref_container<VType, RefDim, Dims...>::DIMENSION, typename util::remove_const<VType>::type> { o });
		return *this;
	}
	template<typename VContainer, typename util::enable_if<VContainer::IS_DATA_CONTAINER || Container::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator/=(const VectorBase<VContainer>& o) {
		Foreach<SetDiv, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	template<typename VType, unsigned int RefDim, unsigned int ... Dims, typename SelfContainer = Container, typename util::enable_if<
			!SelfContainer::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator/=(const vector_ref_container<VType, RefDim, Dims...>& o) {
		Foreach<SetDiv, 0, Container::DIMENSION>::execute(*this,
				vector_data_container<vector_ref_container<VType, RefDim, Dims...>::DIMENSION, typename util::remove_const<VType>::type> { o });
		return *this;
	}
	template<typename VContainer, typename util::enable_if<VContainer::IS_DATA_CONTAINER || Container::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator-=(const VectorBase<VContainer>& o) {
		Foreach<SetSub, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	template<typename VType, unsigned int RefDim, unsigned int ... Dims, typename SelfContainer = Container, typename util::enable_if<
			!SelfContainer::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator-=(const vector_ref_container<VType, RefDim, Dims...>& o) {
		Foreach<SetSub, 0, Container::DIMENSION>::execute(*this,
				vector_data_container<vector_ref_container<VType, RefDim, Dims...>::DIMENSION, typename util::remove_const<VType>::type> { o });
		return *this;
	}
	template<typename VContainer, typename util::enable_if<VContainer::IS_DATA_CONTAINER || Container::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator+=(const VectorBase<VContainer>& o) {
		Foreach<SetAdd, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	template<typename VType, unsigned int RefDim, unsigned int ... Dims, typename SelfContainer = Container, typename util::enable_if<
			!SelfContainer::IS_DATA_CONTAINER>::type* = nullptr>
	self_type& operator+=(const vector_ref_container<VType, RefDim, Dims...>& o) {
		Foreach<SetAdd, 0, Container::DIMENSION>::execute(*this,
				vector_data_container<vector_ref_container<VType, RefDim, Dims...>::DIMENSION, typename util::remove_const<VType>::type> { o });
		return *this;
	}

	//TODO convert these to template<typename MultType>
	self_type& operator*=(const self_component_type& o) {
		Foreach<SingleSetMult, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	self_type& operator/=(const self_component_type& o) {
		Foreach<SingleSetDiv, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	self_type& operator-=(const self_component_type& o) {
		Foreach<SingleSetSub, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}
	self_type& operator+=(const self_component_type& o) {
		Foreach<SingleSetAdd, 0, Container::DIMENSION>::execute(*this, o);
		return *this;
	}

	//do type promotion of types doesnt match
	template<typename VContainer, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION, decltype(self_component_type {}* (typename VContainer::component_type {}))>>>
	ResultType operator*(const VectorBase<VContainer>& o) const {
		return ResultType(*this) *= o;
	}
	template<typename VContainer, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION, decltype(self_component_type {}/ (typename VContainer::component_type {}))>>>
	ResultType operator/(const VectorBase<VContainer>& o) const {
		return ResultType(*this) /= o;
	}
	template<typename VContainer, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION, decltype(self_component_type {}- (typename VContainer::component_type {}))>>>
	ResultType operator-(const VectorBase<VContainer>& o) const {
		return ResultType(*this) -= o;
	}
	template<typename VContainer, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION, decltype(self_component_type {}+ (typename VContainer::component_type {}))>>>
	ResultType operator+(const VectorBase<VContainer>& o) const {
		return ResultType(*this) += o;
	}

	//TODO do not use default constructors in decltype(), do with pointers instead
	template<typename MultType, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION,
					decltype(self_component_type {}* (typename util::remove_reference<MultType>::type {}))>>>
	ResultType operator*(MultType&& o) const {
		return ResultType(*this) *= util::forward<MultType>(o);
	}
	template<typename MultType, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION,
					decltype(self_component_type {}/ (typename util::remove_reference<MultType>::type {}))>>>
	ResultType operator/(MultType&& o) const {
		return ResultType(*this) /= util::forward<MultType>(o);
	}
	template<typename MultType, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION,
					decltype(self_component_type {}- (typename util::remove_reference<MultType>::type {}))>>>
	ResultType operator-(MultType&& o) const {
		return ResultType(*this) -= util::forward<MultType>(o);
	}
	template<typename MultType, typename ResultType = VectorBase<
			vector_data_container<Container::DIMENSION,
					decltype(self_component_type {}+ (typename util::remove_reference<MultType>::type {}))>>>
	ResultType operator+(MultType&& o) const {
		return ResultType(*this) += util::forward<MultType>(o);
	}

	self_component_type sum() const {
		self_component_type result { };
		Foreach<Summarize, 0, Container::DIMENSION>::execute(result, *this);
		return util::move(result);
	}
	self_component_type length() const {
		return (self_component_type) sqrt((*this * *this).sum());
	}

	template<typename VContainer>
	self_component_type distance(const VectorBase<VContainer>& o) const {
		return (*this - o).length();
	}

	self_type& normalize() {
		self_component_type len = length();
		return *this /= len;
	}
	self_data_type normalized() const {
		return util::move(self_type(*this).normalize());
	}

	self_data_type operator-() const {
		self_data_type result;
		Foreach<UnaryMinus, 0, Container::DIMENSION>::execute(result, *this);
		return result;
	}

	template<typename VContainer>
	self_component_type dot(const VectorBase<VContainer>& o) const {
		return (*this * o).sum();
	}

	template<typename VContainer>
	bool operator==(const VectorBase<VContainer>& o) const {
		bool res;
		BreakingForeach<EqualityTester, 0, Container::DIMENSION>::execute(res, *this, o);
		return res;
	}
	template<typename VContainer>
	bool operator!=(const VectorBase<VContainer>& o) const {
		return !(*this == o);
	}

#define DECLARE_VECTOR_SUBFUNCTION(name, ...) \
	ref_type<__VA_ARGS__> name () {\
		return this->template get<__VA_ARGS__>();\
	}\
	const_ref_type<__VA_ARGS__> name () const {\
		return this->template get<__VA_ARGS__>();\
	}
#define DECLARE_SINGLE_SUBFUNC(name, index) \
	self_component_type& name () { return this->template component<index>(); }\
	const self_component_type& name () const { return this->template component<index>(); }

	DECLARE_SINGLE_SUBFUNC(x, 0)
	DECLARE_SINGLE_SUBFUNC(y, 1)
	DECLARE_SINGLE_SUBFUNC(z, 2)
	DECLARE_SINGLE_SUBFUNC(w, 3)

	DECLARE_SINGLE_SUBFUNC(r, 0)
	DECLARE_SINGLE_SUBFUNC(g, 1)
	DECLARE_SINGLE_SUBFUNC(b, 2)
	DECLARE_SINGLE_SUBFUNC(a, 3)

	DECLARE_SINGLE_SUBFUNC(width, 0)
	DECLARE_SINGLE_SUBFUNC(height, 1)
	DECLARE_SINGLE_SUBFUNC(depth, 2)

//2D functions
	DECLARE_VECTOR_SUBFUNCTION(xx, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(xy, 0, 1);
	DECLARE_VECTOR_SUBFUNCTION(xz, 0, 2);
	DECLARE_VECTOR_SUBFUNCTION(xw, 0, 3);

	DECLARE_VECTOR_SUBFUNCTION(yx, 1, 0);
	DECLARE_VECTOR_SUBFUNCTION(yy, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(yz, 1, 2);
	DECLARE_VECTOR_SUBFUNCTION(yw, 1, 3);

	DECLARE_VECTOR_SUBFUNCTION(zx, 2, 0);
	DECLARE_VECTOR_SUBFUNCTION(zy, 2, 1);
	DECLARE_VECTOR_SUBFUNCTION(zz, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(zw, 2, 3);

	DECLARE_VECTOR_SUBFUNCTION(wx, 3, 0);
	DECLARE_VECTOR_SUBFUNCTION(wy, 3, 1);
	DECLARE_VECTOR_SUBFUNCTION(wz, 3, 2);
	DECLARE_VECTOR_SUBFUNCTION(ww, 3, 3);

//3D functions
	DECLARE_VECTOR_SUBFUNCTION(rgb, 0, 1, 2);
	DECLARE_VECTOR_SUBFUNCTION(rrr, 0, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(ggg, 1, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(bbb, 2, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(aaa, 3, 3, 3);

	DECLARE_VECTOR_SUBFUNCTION(xxx, 0, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(xxy, 0, 0, 1);
	DECLARE_VECTOR_SUBFUNCTION(xxz, 0, 0, 2);
	DECLARE_VECTOR_SUBFUNCTION(xxw, 0, 0, 3);
	DECLARE_VECTOR_SUBFUNCTION(xyx, 0, 1, 0);
	DECLARE_VECTOR_SUBFUNCTION(xyy, 0, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(xyz, 0, 1, 2);
	DECLARE_VECTOR_SUBFUNCTION(xyw, 0, 1, 3);
	DECLARE_VECTOR_SUBFUNCTION(xzx, 0, 2, 0);
	DECLARE_VECTOR_SUBFUNCTION(xzy, 0, 2, 1);
	DECLARE_VECTOR_SUBFUNCTION(xzz, 0, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(xzw, 0, 2, 3);
	DECLARE_VECTOR_SUBFUNCTION(xwx, 0, 3, 0);
	DECLARE_VECTOR_SUBFUNCTION(xwy, 0, 3, 1);
	DECLARE_VECTOR_SUBFUNCTION(xwz, 0, 3, 2);
	DECLARE_VECTOR_SUBFUNCTION(xww, 0, 3, 3);

	DECLARE_VECTOR_SUBFUNCTION(yxx, 1, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(yxy, 1, 0, 1);
	DECLARE_VECTOR_SUBFUNCTION(yxz, 1, 0, 2);
	DECLARE_VECTOR_SUBFUNCTION(yxw, 1, 0, 3);
	DECLARE_VECTOR_SUBFUNCTION(yyx, 1, 1, 0);
	DECLARE_VECTOR_SUBFUNCTION(yyy, 1, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(yyz, 1, 1, 2);
	DECLARE_VECTOR_SUBFUNCTION(yyw, 1, 1, 3);
	DECLARE_VECTOR_SUBFUNCTION(yzx, 1, 2, 0);
	DECLARE_VECTOR_SUBFUNCTION(yzy, 1, 2, 1);
	DECLARE_VECTOR_SUBFUNCTION(yzz, 1, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(yzw, 1, 2, 3);
	DECLARE_VECTOR_SUBFUNCTION(ywx, 1, 3, 0);
	DECLARE_VECTOR_SUBFUNCTION(ywy, 1, 3, 1);
	DECLARE_VECTOR_SUBFUNCTION(ywz, 1, 3, 2);
	DECLARE_VECTOR_SUBFUNCTION(yww, 1, 3, 3);

	DECLARE_VECTOR_SUBFUNCTION(zxx, 2, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(zxy, 2, 0, 1);
	DECLARE_VECTOR_SUBFUNCTION(zxz, 2, 0, 2);
	DECLARE_VECTOR_SUBFUNCTION(zxw, 2, 0, 3);
	DECLARE_VECTOR_SUBFUNCTION(zyx, 2, 1, 0);
	DECLARE_VECTOR_SUBFUNCTION(zyy, 2, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(zyz, 2, 1, 2);
	DECLARE_VECTOR_SUBFUNCTION(zyw, 2, 1, 3);
	DECLARE_VECTOR_SUBFUNCTION(zzx, 2, 2, 0);
	DECLARE_VECTOR_SUBFUNCTION(zzy, 2, 2, 1);
	DECLARE_VECTOR_SUBFUNCTION(zzz, 2, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(zzw, 2, 2, 3);
	DECLARE_VECTOR_SUBFUNCTION(zwx, 2, 3, 0);
	DECLARE_VECTOR_SUBFUNCTION(zwy, 2, 3, 1);
	DECLARE_VECTOR_SUBFUNCTION(zwz, 2, 3, 2);
	DECLARE_VECTOR_SUBFUNCTION(zww, 2, 3, 3);

	DECLARE_VECTOR_SUBFUNCTION(wxx, 3, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(wxy, 3, 0, 1);
	DECLARE_VECTOR_SUBFUNCTION(wxz, 3, 0, 2);
	DECLARE_VECTOR_SUBFUNCTION(wxw, 3, 0, 3);
	DECLARE_VECTOR_SUBFUNCTION(wyx, 3, 1, 0);
	DECLARE_VECTOR_SUBFUNCTION(wyy, 3, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(wyz, 3, 1, 2);
	DECLARE_VECTOR_SUBFUNCTION(wyw, 3, 1, 3);
	DECLARE_VECTOR_SUBFUNCTION(wzx, 3, 2, 0);
	DECLARE_VECTOR_SUBFUNCTION(wzy, 3, 2, 1);
	DECLARE_VECTOR_SUBFUNCTION(wzz, 3, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(wzw, 3, 2, 3);
	DECLARE_VECTOR_SUBFUNCTION(wwx, 3, 3, 0);
	DECLARE_VECTOR_SUBFUNCTION(wwy, 3, 3, 1);
	DECLARE_VECTOR_SUBFUNCTION(wwz, 3, 3, 2);
	DECLARE_VECTOR_SUBFUNCTION(www, 3, 3, 3);

//4D functions
	DECLARE_VECTOR_SUBFUNCTION(rrrr, 0, 0, 0, 0);
	DECLARE_VECTOR_SUBFUNCTION(gggg, 1, 1, 1, 1);
	DECLARE_VECTOR_SUBFUNCTION(bbbb, 2, 2, 2, 2);
	DECLARE_VECTOR_SUBFUNCTION(aaaa, 3, 3, 3, 3);
	DECLARE_VECTOR_SUBFUNCTION(rgba, 0, 1, 2, 3);
	DECLARE_VECTOR_SUBFUNCTION(argb, 3, 0, 1, 2);

#undef DECLARE_VECTOR_SUBFUNCTION
#undef DECLARE_SINGLE_SUBFUNC
};

#define UINT_TO_COLOR_RGBA(uint) (float)((uint) >> 24)/255.0f, (float)(((uint) >> 16)&0xFF)/255.0f, (float)(((uint)>>8)&0xFF)/255.0f, (float)((uint)&0xFF)/255.0f
#define UINT_TO_COLOR_ARGB(uint) (float)(((uint) >> 16)&0xFF)/255.0f, (float)(((uint)>>8)&0xFF)/255.0f, (float)((uint)&0xFF)/255.0f, (float)((uint) >> 24)/255.0f

template<unsigned int Dimension, typename Type>
using Vector = VectorBase<vector_data_container<Dimension, Type>>;
template<typename Type>
using Vector2 = Vector<2, Type>;
template<typename Type>
using Vector3 = Vector<3, Type>;
template<typename Type>
using Vector4 = Vector<4, Type>;

using Vector2F = Vector2<float>;
using Vector3F = Vector3<float>;
using Vector4F = Vector4<float>;

using Vector2UI = Vector2<unsigned int>;
using Vector3UI = Vector3<unsigned int>;
using Vector4UI = Vector4<unsigned int>;

using Vector2I = Vector2<int>;
using Vector3I = Vector3<int>;
using Vector4I = Vector4<int>;

using Size2F = Vector2F;
using Size3F = Vector3F;

using Size2UI = Vector2UI;
using Size3UI = Vector3UI;

using Size2I = Vector2I;
using Size3I = Vector3I;

typedef Vector2F Position2F;
typedef Vector3F Position3F;

typedef Vector4F Color;

template<typename Container>
class SerializeIsEndianIndependent<VectorBase<Container>> : public SerializeIsEndianIndependent<typename Container::component_type> {
};

template<typename Container, Endianness ENDIAN>
class SerializeExecutor<VectorBase<Container>, ENDIAN> {
	using self_component_type = typename Container::component_type;

	template<typename InStream>
	struct Reader {
		template<unsigned int Index>
		inline static bool execute(InStream& is, VectorBase<Container>& outdata) {
			return SerializeExecutor<self_component_type, ENDIAN>::deserialize(is, outdata.template component<Index>());
		}
	};

	template<typename OutStream>
	struct Writer {
		template<unsigned int Index>
		inline static bool execute(OutStream& os, const VectorBase<Container>& data) {
			return SerializeExecutor<self_component_type, ENDIAN>::serialize(os, data.template component<Index>());
		}
	};
public:
	template<typename InStream>
	static bool deserialize(InStream& is, VectorBase<Container>& outdata) {
		return BreakingForeach<Reader<InStream>, 0, Container::DIMENSION>::execute(is, outdata);
	}
	template<typename InStream>
	static bool deserialize(InStream& is, VectorBase<Container> && outdata) {
		return deserialize(is, outdata);
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const VectorBase<Container>& data) {
		return BreakingForeach<Writer<OutStream>, 0, Container::DIMENSION>::execute(os, data);
	}
};

#if LOGGING_ENABLED

template<typename Container>
class __internal_tostring_t<VectorBase<Container>> {
	struct Appender {
		template<unsigned int Index>
		inline static void execute(_tostring_type& result, const VectorBase<Container>& value) {
			if (Index != 0) {
				result = result + ", ";
			}
			result = result + TOSTRING(value.template component<Index>());
		}
	};
public:
	static _tostring_type tostring(const VectorBase<Container>& value) {
		_tostring_type result = "(";
		Foreach<Appender, 0, Container::DIMENSION>::execute(result, value);
		return result + ")";
	}
};

#endif /* RHFW_DEBUG */

} // namespace rhfw

#endif /* VECTOR_H_ */
