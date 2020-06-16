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
 * ConfigurationManager.cpp
 *
 *  Created on: 2016. marc. 8.
 *      Author: sipka
 */

#include <framework/configuration/ConfigurationManager.h>
#include <gen/log.h>

namespace rhfw {
namespace configuration {

//void ConfigurationManager::setConfiguration(const ResourceConfiguration& config, ConfigChangeMask mask) {
//ConfigurationChangedListener::call(config, mask);
//}

//void ConfigurationManager::setConfiguration(const ResourceConfiguration& a_config) {
//TODO 
/*	ASSERT(ResourceConfiguration::current.version >= a_config.version, "Resource configuration version error: %u - %u",
 ResourceConfiguration::current.version, a_config.version);

 ConfigChangeMask changedmask =
 ResourceConfiguration::current.version == a_config.version ?
 a_config.changedMask : a_config.getChangeMask(ResourceConfiguration::current);

 WARN(changedmask == 0x0, "Setting configuration with no changes");
 WARN(a_config.getChangeMask(ResourceConfiguration::current) == 0x0, "Setting configuration with no changes");
 if (changedmask != 0x0)
 setConfiguration(a_config, changedmask);
 */
//}
void Transaction::commit() {
	for (auto&& n : values.nodes()) {
		static_cast<ValueBase*>(n)->set();
	}
	for (auto&& n : manager.changedListeners.nodes()) {
		static_cast<ConfigurationChangedListener*>(n)->onConfigurationChanged(manager);
	}
	values.clear([&](LinkedNode<ValueBase>* n) {
		static_cast<ValueBase*>(n)->set();
	});
}

}  // namespace configuration
}  // namespace rhfw

