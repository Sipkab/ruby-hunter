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
 * ViewContainer.cpp
 *
 *  Created on: 2014.06.18.
 *      Author: sipka
 */

#include <framework/layer/LayerGroup.h>
#include <framework/xml/XmlNode.h>
#include <gen/log.h>
#include <gen/xmldecl.h>
#include <gen/types.h>

using namespace rhfw;
LINK_XML_SIMPLE(LayerGroup)

namespace rhfw {

void LayerGroup::draw() {
	for (auto& layer : childLayers.objects()) {
		layer.draw();
	}
}

Layer* LayerGroup::touch() {
	for (auto& layer : childLayers.reverse().objects()) {
		Layer* result = layer.touch();
		if (result != nullptr) {
			return result;
		}

		if (HAS_FLAG(layer.getFlags(), LayerOptions::MODAL))
			break;
	}
	return nullptr;
}

void LayerGroup::sizeChanged(const core::WindowSize& size) {
	for (auto& layer : childLayers.objects()) {
		layer.sizeChanged(size);
	}
}

void LayerGroup::loadResources(ResourceLoader& loader) {
	for (auto& layer : childLayers.objects()) {
		layer.loadResources(loader);
	}
}

void LayerGroup::setScene(Scene* scene) {
	Layer::setScene(scene);
	for (auto& layer : childLayers.objects()) {
		layer.setScene(scene);
	}
}

bool LayerGroup::addXmlChild(const xml::XmlNode& child) {
	switch (child.staticType) {
		case RXml::Elem::Layer:
			this->addLayerToTop(*static_cast<Layer*>(child));
			return true;
		default:
			return false;
	}
}

void LayerGroup::onResourcesLoaded() {
	for (auto& layer : childLayers.objects()) {
		layer.onResourcesLoaded();
	}
}
}
