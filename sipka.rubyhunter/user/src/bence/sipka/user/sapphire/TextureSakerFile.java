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
package bence.sipka.user.sapphire;

import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Collection;

public class TextureSakerFile extends ImageSakerFile {
	private Collection<Element> elements = new ArrayList<>();
	private int dim;

	public TextureSakerFile(String name, int dim) {
		super(name, new BufferedImage(
				dim * (SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION
						+ SapphireTextureConverterWorkerTaskFactory.TILE_PADDING * 2),
				dim * (SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION
						+ SapphireTextureConverterWorkerTaskFactory.TILE_PADDING * 2),
				BufferedImage.TYPE_INT_ARGB));
		this.dim = dim;
	}

	public boolean canAdd() {
		return elements.size() < dim * dim;
	}

	public void add(Element elem) {
		BufferedImage img = getImage();
		int idx = elements.size();
		elements.add(elem);
		int x = (idx % dim)
				* (SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION
						+ SapphireTextureConverterWorkerTaskFactory.TILE_PADDING * 2)
				+ SapphireTextureConverterWorkerTaskFactory.TILE_PADDING;
		int y = (idx / dim)
				* (SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION
						+ SapphireTextureConverterWorkerTaskFactory.TILE_PADDING * 2)
				+ SapphireTextureConverterWorkerTaskFactory.TILE_PADDING;
		elem.resultPos = new Rectangle2D.Float(//
				x / (float) img.getWidth(), //
				y / (float) img.getHeight(), //
				(float) SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION / img.getWidth(), //
				(float) SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION / img.getHeight()//
		);

		for (int i = 0; i < SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION; i++) {
			for (int j = 0; j < SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION; j++) {
				img.setRGB(x + i, y + j, elem.image.getRGB(elem.imagePos.x + i, elem.imagePos.y + j));
			}
		}
		int tl = elem.image.getRGB(elem.imagePos.x, elem.imagePos.y);
		int bl = elem.image.getRGB(elem.imagePos.x,
				elem.imagePos.y + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION - 1);
		int tr = elem.image.getRGB(elem.imagePos.x + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION - 1,
				elem.imagePos.y);
		int br = elem.image.getRGB(elem.imagePos.x + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION - 1,
				elem.imagePos.y + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION - 1);

		for (int i = 0; i < SapphireTextureConverterWorkerTaskFactory.TILE_PADDING; i++) {
			for (int j = 0; j < SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION; j++) {
				img.setRGB(x - 1 - i, y + j, elem.image.getRGB(elem.imagePos.x + 0, elem.imagePos.y + j));
				img.setRGB(x + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION + i, y + j,
						elem.image.getRGB(
								elem.imagePos.x + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION - 1,
								elem.imagePos.y + j));

				img.setRGB(x + j, y - 1 - i, elem.image.getRGB(elem.imagePos.x + j, elem.imagePos.y + 0));
				img.setRGB(x + j, y + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION + i,
						elem.image.getRGB(elem.imagePos.x + j,
								elem.imagePos.y + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION - 1));
			}
			for (int j = 0; j < SapphireTextureConverterWorkerTaskFactory.TILE_PADDING; j++) {
				img.setRGB(x - 1 - i, y - 1 - j, tl);
				img.setRGB(x - 1 - i, y + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION + j, bl);
				img.setRGB(x + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION + i, y - 1 - j, tr);
				img.setRGB(x + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION + i,
						y + SapphireTextureConverterWorkerTaskFactory.TILE_DIMENSION + j, br);
			}
		}

	}

}