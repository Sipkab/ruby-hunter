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
 * ConfigurationManager.h
 *
 *  Created on: 2016. marc. 8.
 *      Author: sipka
 */

#ifndef CONFIGURATIONMANAGER_H_
#define CONFIGURATIONMANAGER_H_

#include <framework/utils/BasicGlobalListener.h>
#include <framework/utils/BasicListener.h>

//#include <gen/resconfig.h>

namespace rhfw {
namespace configuration {

class Transaction;
template<typename T>
class Property;

class ConfigurationManager;

class PropertyBase: public LinkedNode<PropertyBase> {
private:
protected:
public:
	virtual ~PropertyBase() = default;

	PropertyBase* get() override {
		return this;
	}
};

class ConfigurationChangedListener: public BasicListener<ConfigurationChangedListener> {
private:
public:
	virtual void onConfigurationChanged(ConfigurationManager& manager) {
	}
};

template<typename T>
class PropertyChangedListener: public BasicListener<PropertyChangedListener<T>> {
private:
public:
	template<typename Functor>
	class Lambda: public PropertyChangedListener<T> {
		Functor functor;
	public:
		Lambda(Functor&& f)
				: functor(util::forward<Functor>(f)) {
		}
		virtual void onPropertyChanged(Property<T>& property) override {
			functor(property);
		}
	};

	virtual void onPropertyChanged(Property<T>& property) {
	}
};

template<typename T>
class Property final: public PropertyBase {
	friend class Transaction;
private:
	bool changed = false;
protected:
	T data;
public:
	typename PropertyChangedListener<T>::Events changedListeners;

	template<typename ... Args>
	Property(Args&&... args)
			: data(util::forward<Args>(args)...) {
	}

	operator const T&() const {
		return data;
	}
	const T& operator->() const {
		return *this;
	}

	Property<T>* get() override {
		return this;
	}

	bool operator==(const T& o) const {
		return data == o;
	}
	bool operator!=(const T& o) const {
		return data != o;
	}
	bool operator<(const T& o) const {
		return data < o;
	}
	bool operator<=(const T& o) const {
		return data <= o;
	}
	bool operator>=(const T& o) const {
		return data >= o;
	}
	bool operator>(const T& o) const {
		return data > o;
	}

	bool operator==(const Property<T>& o) {
		return this == &o;
	}
	bool operator!=(const Property<T>& o) {
		return this != &o;
	}
};

class Transaction {
private:
	class ValueBase: public LinkedNode<ValueBase> {
	private:
	public:
		ValueBase* get() override {
			return this;
		}
		virtual void set() = 0;

		virtual void finish() = 0;
	};
	template<typename T>
	class Value final: public ValueBase {
	private:
		Property<T>& prop;
		T val;
	public:
		Value(Property<T>& prop, T&& val)
				: prop(prop), val(util::move(val)) {
		}
		Value<T>* get() override {
			return this;
		}
		virtual void set() override {
			if (val == prop.data) {
				return;
			}
			prop.changed = true;
			prop.data = util::move(val);
		}

		virtual void finish() override {
			if (prop.changed) {
				for (auto&& n : prop.changedListeners.nodes()) {
					static_cast<PropertyChangedListener<T>*>(n)->onPropertyChanged(prop);
				}
				prop.changed = false;
			}
		}
	};

	ConfigurationManager& manager;
	LinkedList<ValueBase> values;
public:
	Transaction(ConfigurationManager& manager)
			: manager(manager) {
	}
	Transaction(const Transaction&) = delete;
	Transaction(Transaction&&) = default;
	Transaction& operator=(const Transaction&) = delete;
	Transaction& operator=(Transaction&&) = delete;

	template<typename T>
	Transaction& set(Property<T>& prop, T value) {
		values.addToEnd(new Value<T>(prop, util::move(value)));
		return *this;
	}
	void commit();
};

typedef unsigned long long ConfigChangeMask;

class ConfigurationManager {
private:
	LinkedList<PropertyBase> properties;

	//static void setConfiguration(const ResourceConfiguration& config, ConfigChangeMask mask);
public:
	ConfigurationChangedListener::Events changedListeners;

	//static void setConfiguration(const ResourceConfiguration& config);
};
/*
 class ConfigurationChangedListener: public BasicGlobalListener<ConfigurationChangedListener> {
 private:
 friend class ConfigurationManager;

 static void call(const ResourceConfiguration& config, ConfigChangeMask mask) {
 for (auto&& l : foreach()) {
 l.onConfigurationChanged(config, mask);
 }
 }
 public:

 virtual void onConfigurationChanged(const ResourceConfiguration& config, ConfigChangeMask mask) = 0;
 };
 */
}  // namespace configuration
}  // namespace rhfw

#endif /* CONFIGURATIONMANAGER_H_ */
