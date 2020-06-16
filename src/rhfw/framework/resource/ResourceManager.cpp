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
 * ResourceManager.cpp
 *
 *  Created on: 2015 ?pr. 4
 *      Author: sipka
 */

#include <framework/resource/ResourceManager.h>

#include <framework/io/files/FileInput.h>
#include <framework/io/files/FileDescriptor.h>
#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/utils/StaticInitializer.h>

#include <gen/log.h>

#include <gen/assets.h>
#include <gen/xmlcompile.h>

static_assert(sizeof(unsigned long long) == 8, "long long not 8 bytes");

namespace rhfw {

ResourceDictionary<ResIds::QUALIFIED_RESOURCE_COUNT> ResourceManager::resourceDictionary;
ConditionalArray<Resource<ShareableResource>, ResIds::RESOURCE_COUNT> ResourceManager::sharedStaticResources;

void ResourceManager::initStatic() {
	LOGTRACE();
	//setConfiguration(ResourceConfiguration::current);
}

//void ResourceManager::setConfiguration(const ResourceConfiguration& a_config) {
/*ASSERT(ResourceConfiguration::current.version >= a_config.version) <<  "Resource configuration version error: " <<
 ResourceConfiguration::current.version << " - " << a_config.version;

 ConfigChangeMask changedmask =
 ResourceConfiguration::current.version == a_config.version ?
 a_config.changedMask : a_config.getChangeMask(ResourceConfiguration::current);

 WARN(changedmask == 0x0) <<  "Setting configuration with no changes";
 WARN(a_config.getChangeMask(ResourceConfiguration::current) == 0x0) <<  "Setting configuration with no changes";
 if (changedmask != 0x0)
 setConfiguration(a_config, changedmask);*/
//}
//void ResourceManager::setConfiguration(const ResourceConfiguration& config, ConfigChangeMask changedmask) {
//	LOGI() << "Configuration changed, mask: " << changedmask;
/*if (ResIds::QUALIFIED_RESOURCE_COUNT > 0) {
 LOGV() << "Reading configuration dictionary";
 //read resource dictionary
 AssetFileDescriptor fd { RAssets::RESOURCE_DICTIONARY };
 FileInput* in = fd.createInput();
 in->open();
 for (unsigned int i = 0; i < ResIds::RESOURCE_COUNT; ++i) {
 const ResId resid = (ResId) in->readIntBigEndian();
 const unsigned int assetcount = in->readIntBigEndian();

 StaticResourceEntry& entry = resourceDictionary[(unsigned int) resid];
 Resource<ShareableResource>& resptr = sharedStaticResources[(unsigned int) resid];
 const RAssetFile originalAssetId = entry.assetid;
 ConfigChangeMask bestFit = 0x0;
 for (unsigned int j = 0; j < assetcount; ++j) {
 const RAssetFile assetid = (RAssetFile) in->readIntBigEndian();
 const unsigned int qualifiercount = in->readIntBigEndian();
 //LOGV("Resource configuration %s, asset %s", TOSTRING(resid), TOSTRING(assetid));
 //TODO ha van relevant changed, akkor INVALID_ASSET_IDENTIFIER-re allitas
 ConfigChangeMask readmask = 0;

 bool validqual = true;

 for (unsigned int k = 0; k < qualifiercount; ++k) {
 const signed char valtype = in->readByte();
 const unsigned char index = in->readByte();
 readmask |= 1ull << (63 - index);*/
/*
 unsigned int readint = 0;
 const char* readptr = nullptr;
 switch (valtype) {
 case RXmlCompile::Types::INTEGER: {
 readint = in->readIntBigEndian();
 break;
 }
 case RXmlCompile::Types::FLOAT: {
 readint = in->readIntBigEndian();
 break;
 }
 case RXmlCompile::Types::STRING: {
 unsigned int ptrgot;
 readint = in->readIntBigEndian();
 readptr = in->get(readint, &ptrgot);
 ASSERT(ptrgot == readint) <<  "failed to read enough chars";
 break;
 }
 case RXmlCompile::Types::BOOLEAN: {
 readint = in->readByte();
 break;
 }
 default: {
 THROW() << "Invalid attribute type: " << valtype;
 break;
 }
 }

 validqual = validqual && config.matchesAtIndex(index, {readint, readptr});*/
/*}

 if (validqual && readmask >= bestFit) { //equal for initial condition. btw only one can fit with one readmask
 entry.assetid = assetid;
 bestFit = readmask;
 }

 }
 if (entry.assetid != originalAssetId && resptr != nullptr) {
 resptr.reloadIfLoaded();
 }
 }
 in->close();
 delete in;
 }*/
//ResourceConfiguration::current = config;
//++ResourceConfiguration::current.version;
//}
void ResourceManager::destroyStatic() {
	LOGTRACE();
	/*for (unsigned int i = 0; i < ResIds::QUALIFIED_RESOURCE_COUNT; ++i) {
	 delete sharedStaticResources[i];
	 sharedStaticResources[i] = nullptr;
	 resourceDictionary[i].assetid = RAssetFile::INVALID_ASSET_IDENTIFIER;
	 }
	 for (unsigned int i = ResIds::QUALIFIED_RESOURCE_COUNT; i < ResIds::RESOURCE_COUNT; ++i) {
	 delete sharedStaticResources[i];
	 sharedStaticResources[i] = nullptr;
	 }*/
}

namespace resources {
/*
 typedef StaticInitializer<ResourceConfiguration, ResourceManager> StaticResourcesInitializer;
 void module_initialize() {
 StaticResourcesInitializer::initStatic();
 }
 void module_terminate() {
 StaticResourcesInitializer::destroyStatic();
 }
 */
}
}
