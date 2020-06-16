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
 * ResourceManager.h
 *
 *  Created on: 2015 ?pr. 4
 *      Author: sipka
 */

#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

#include <framework/resource/Resource.h>
#include <framework/resource/ShareableResource.h>
#include <framework/utils/ConditionalArray.h>
#include <gen/resources.h>
#include <gen/configuration.h>
#include <gen/log.h>
//#include <gen/resconfig.h>
#include <gen/assets.h>
#include <gen/types.h>

namespace rhfw {

class StaticResourceEntry {
public:
	RAssetFile assetid = RAssetFile::INVALID_ASSET_IDENTIFIER;
};

template<int ResCount>
class ResourceDictionary {
	StaticResourceEntry resourceDictionary[ResCount];
public:
	operator StaticResourceEntry*() {
		return resourceDictionary;
	}
};

template<>
class ResourceDictionary<0> {
public:
	operator StaticResourceEntry*() {
		return nullptr;
	}
};
//TODO ami csak sima asset file, annak ne foglaljunk shared static resource memoriat
class ResourceManager {
private:
	typedef unsigned long long ConfigChangeMask;

	static ConditionalArray<Resource<ShareableResource>, ResIds::RESOURCE_COUNT> sharedStaticResources;

	static ResourceDictionary<ResIds::QUALIFIED_RESOURCE_COUNT> resourceDictionary;

	ResourceManager() = delete;
	~ResourceManager() = delete;

	//static void setConfiguration(const ResourceConfiguration& config, ConfigChangeMask mask);
public:

	static void initStatic();
	static void destroyStatic();

	//static void setConfiguration(const ResourceConfiguration& config);

	static RAssetFile idToFile(ResId id) {
		return (RAssetFile) id;
		ASSERT((unsigned int ) id < ResIds::RESOURCE_COUNT) << "Resource id is invalid: " << id;
		if ((unsigned int) id >= (unsigned int) ResIds::QUALIFIED_RESOURCE_COUNT) {
			return (RAssetFile) id;
		}
		ASSERT(resourceDictionary[(unsigned int )id].assetid != RAssetFile::INVALID_ASSET_IDENTIFIER) <<
		"resource is not defined in dictionary with configuration: " << id;
		return resourceDictionary[(unsigned int) id].assetid;
	}
	/*
	 template<typename T>
	 static Resource<T> getSharedResource(ResId id) {
	 auto& result = sharedStaticResources[id];
	 if (result == nullptr) {
	 result = new T { id };
	 sharedStaticResources[id].setPointer(result);
	 }
	 return sharedStaticResources[id];
	 }
	 */
};
}

#endif /* RESOURCEMANAGER_H_ */
