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
 * LayerGroup.h
 *
 *  Created on: 2014.06.18.
 *      Author: sipka
 */

#ifndef LAYERGROUP_H_
#define LAYERGROUP_H_

#include <framework/layer/Layer.h>
#include <framework/utils/LinkedList.h>
#include <gen/configuration.h>
namespace rhfw {

namespace xml {
class XmlNode;
}  // namespace xml

class LayerGroup: public Layer {
protected:
	LinkedList<Layer> childLayers;
public:
	LayerGroup() {
	}

	virtual void draw() override;

	virtual void setScene(Scene* scene) override;

	virtual void sizeChanged(const core::WindowSize& size) override;

	/**
	 * Adds a Layer to the LayerGroup, removes from previous parent.
	 * The Layer will be added to the front (on top) in drawing order.
	 * Takes ownership of the Layer
	 */
	void addLayerToTop(LinkedNode<Layer>& v) {
		v.removeLinkFromList();
		childLayers.addToEnd(v);
	}

	void addLayerToBottom(LinkedNode<Layer>& v) {
		v.removeLinkFromList();
		childLayers.addToStart(v);
	}

	virtual ~LayerGroup() {
		//LinkedList will delete children
	}

	bool addXmlChild(const xml::XmlNode& child);

	virtual Layer* touch() override;

	virtual void loadResources(ResourceLoader& loader) override;

	virtual void onResourcesLoaded() override;

	void clearChildren() {
		childLayers.clear();
	}

};

} // namespace rhfw

#endif /* LAYERGROUP_H_ */
