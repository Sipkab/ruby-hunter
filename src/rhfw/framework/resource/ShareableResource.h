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
 * ShareableResource.h
 *
 *  Created on: 2015 nov. 22
 *      Author: sipka
 */

#ifndef SHAREABLERESOURCE_H_
#define SHAREABLERESOURCE_H_

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class ShareableResource {
protected:
	template<typename, typename >
	friend class ResourceBase;

	virtual bool load() = 0;
	virtual void free() = 0;
	virtual bool reload() {
		LOGW()<<"Reload not implemented";
		free();
		return load();
	}
public:
	ShareableResource() = default;
	ShareableResource(ShareableResource&& o) = default;
	ShareableResource& operator=(ShareableResource&& o) {
		ASSERT(this != &o) << "Self move assignment";
		return *this;
	}
	ShareableResource(const ShareableResource&) = default;
	ShareableResource& operator=(const ShareableResource&) = default;
	virtual ~ShareableResource() = default;
};

}
 // namespace rhfw

#endif /* SHAREABLERESOURCE_H_ */
