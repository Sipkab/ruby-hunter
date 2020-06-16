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
 * ResourceInputSource.h
 *
 *  Created on: 2016. marc. 22.
 *      Author: sipka
 */

#ifndef RESOURCEINPUTSOURCE_H_
#define RESOURCEINPUTSOURCE_H_

#include <framework/utils/InputSource.h>
#include <framework/io/files/AssetFileDescriptor.h>

#include <gen/resources.h>

namespace rhfw {

class ResourceInputSource: public InputSource {
private:
	ResId res;
	AssetFileDescriptor asset;

	virtual void closeData(InputSource::Data& data) override;
public:
	ResourceInputSource(ResId res)
			: res { res } {
	}
	virtual InputSource::Data getData() override;
};

}  // namespace rhfw

#endif /* RESOURCEINPUTSOURCE_H_ */
