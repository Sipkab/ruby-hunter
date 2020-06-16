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
package bence.sipka.user.obj3d;

import java.io.IOException;
import java.io.Serializable;
import java.io.UncheckedIOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.stream.Collectors;

import bence.sipka.compiler.types.builtin.FloatType;
import bence.sipka.compiler.types.builtin.IntegerType;
import saker.build.file.path.SakerPath;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;

public class ObjectConfiguration implements Serializable {
	private static final long serialVersionUID = 1L;

	private ObjectCollection objectCollection;
	private byte[] bytes;

	Map<Integer, ObjectDescriptor> descriptors = new HashMap<>();
	List<Material> colorMaterials = new ArrayList<>();
	List<Material> textureMaterials = new ArrayList<>();

	public ObjectConfiguration(ObjectCollection objectCollection, SakerPath workingDirectoryPath,
			Map<String, Integer> assetsIdentifierMap) {
		this.objectCollection = objectCollection;
		this.colorMaterials.addAll(objectCollection.colorMaterials);
		this.textureMaterials.addAll(objectCollection.textureMaterials);
		bytes = init(workingDirectoryPath, assetsIdentifierMap);
	}

	public byte[] getBytes() {
		return bytes;
	}

	public ObjectDescriptor getDescriptor(int id) {
		return descriptors.get(id);
	}

	private byte[] init(SakerPath workingDirectoryPath, Map<String, Integer> assetsIdentifierMap) {
		byte[] data;
		List<Face> faces = objectCollection.objects.stream().flatMap(o -> o.faces.stream())
				.collect(Collectors.toList());
		Map<Material, List<Face>> matfaces = faces.stream().collect(Collectors.groupingBy(f -> f.material));

		Map<Material, List<Face>> texturedmatfaces = matfaces.entrySet().stream().filter(e -> e.getKey().hasTexture())
				.collect(Collectors.toMap(e -> e.getKey(), e -> e.getValue()));
		Map<Material, List<Face>> coloredmatfaces = matfaces.entrySet().stream().filter(e -> !e.getKey().hasTexture())
				.collect(Collectors.toMap(e -> e.getKey(), e -> e.getValue()));

		colorMaterials.addAll(coloredmatfaces.keySet());
		textureMaterials.addAll(texturedmatfaces.keySet());
		colorMaterials.sort((m1, m2) -> {
			float f1 = m1.getTransparency() == null ? 1.0f : m1.getTransparency();
			float f2 = m2.getTransparency() == null ? 1.0f : m2.getTransparency();
			return -Float.compare(f1, f2);
		});
		textureMaterials.sort((m1, m2) -> {
			float f1 = m1.getTransparency() == null ? 1.0f : m1.getTransparency();
			float f2 = m2.getTransparency() == null ? 1.0f : m2.getTransparency();
			return -Float.compare(f1, f2);
		});

		Comparator<Face> facesorter = (a, b) -> {
			if (a.obj == b.obj) {
				return a.material.getName().compareTo(b.material.getName());
			}
			return Integer.compare(a.obj.id, b.obj.id);
		};

		List<Face> coloredfaces = coloredmatfaces.values().stream().flatMap(f -> f.stream()).sorted(facesorter)
				.collect(Collectors.toList());
		List<Face> texturedfaces = texturedmatfaces.values().stream().flatMap(f -> f.stream()).sorted(facesorter)
				.collect(Collectors.toList());

		int coloredfacecount = coloredfaces.size();
		int texturedfacecount = texturedfaces.size();
		int coloredmatcount = colorMaterials.size();
		int texturedmatcount = textureMaterials.size();

		Map<ObjectData, List<Face>> coloredobjects = coloredfaces.stream().collect(Collectors.groupingBy(f -> f.obj));
		Map<ObjectData, List<Face>> texturedobjects = texturedfaces.stream().collect(Collectors.groupingBy(f -> f.obj));
		Map<ObjectData, Map<Material, List<Face>>> coloredranges = coloredobjects.entrySet().stream().collect(Collectors
				.toMap(e -> e.getKey(), e -> e.getValue().stream().collect(Collectors.groupingBy(f -> f.material))));
		Map<ObjectData, Map<Material, List<Face>>> texturedranges = texturedobjects.entrySet().stream()
				.collect(Collectors.toMap(e -> e.getKey(),
						e -> e.getValue().stream().collect(Collectors.groupingBy(f -> f.material))));
		for (Entry<ObjectData, Map<Material, List<Face>>> e : coloredranges.entrySet()) {
			ObjectDescriptor desc = descriptors.get(e.getKey().id);
			if (desc == null) {
				desc = new ObjectDescriptor();
				descriptors.put(e.getKey().id, desc);
			}
			for (Entry<Material, List<Face>> mate : e.getValue().entrySet()) {
				desc.colored++;
				desc.triangles
						.add(new ObjectDescriptor.TriangleRegion(mate.getKey(), colorMaterials.indexOf(mate.getKey()),
								coloredfaces.indexOf(mate.getValue().get(0)) * 3, mate.getValue().size() * 3));
			}
		}
		for (Entry<ObjectData, Map<Material, List<Face>>> e : texturedranges.entrySet()) {
			ObjectDescriptor desc = descriptors.get(e.getKey().id);
			if (desc == null) {
				desc = new ObjectDescriptor();
				descriptors.put(e.getKey().id, desc);
			}
			for (Entry<Material, List<Face>> mate : e.getValue().entrySet()) {
				desc.textured++;
				desc.triangles
						.add(new ObjectDescriptor.TriangleRegion(mate.getKey(), textureMaterials.indexOf(mate.getKey()),
								texturedfaces.indexOf(mate.getValue().get(0)) * 3, mate.getValue().size() * 3));
			}
		}

		try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream()) {
			IntegerType.INSTANCE.serialize(coloredfacecount, baos);
			IntegerType.INSTANCE.serialize(texturedfacecount, baos);
			IntegerType.INSTANCE.serialize(coloredmatcount, baos);
			IntegerType.INSTANCE.serialize(texturedmatcount, baos);
			for (Material mat : colorMaterials) {
				mat.getDiffuseColor().serializeXYZW(baos);
				mat.getAmbientColor().serializeXYZW(baos);
				mat.getSpecularColor().serializeXYZW(baos);
				FloatType.INSTANCE.serialize(mat.getSpecularExponent(), baos);
			}
			for (Face f : coloredfaces) {
				for (Vertex vert : f) {
					vert.pos.normalizeW().serializeXYZ(baos);
					vert.norm.normalizeLength().serializeXYZ(baos);
				}
			}

			for (Material mat : textureMaterials) {
				String assetpath = workingDirectoryPath.relativize(mat.getTexture()).toString().replace('\\', '/');
				Integer id = assetsIdentifierMap.get(assetpath);
				if (id == null) {
					throw new IllegalArgumentException("Asset not found: " + assetpath);
				}
				IntegerType.INSTANCE.serialize("0x" + Integer.toHexString(id), baos);
				mat.getAmbientColor().serializeXYZW(baos);
				mat.getSpecularColor().serializeXYZW(baos);
				FloatType.INSTANCE.serialize(mat.getSpecularExponent(), baos);
			}
			for (Face f : texturedfaces) {
				for (Vertex vert : f) {
					vert.pos.normalizeW().serializeXYZ(baos);
					vert.norm.normalizeLength().serializeXYZ(baos);
					// vert.tex.normalizeW().serializeXY(baos);
					Vector.serializeXY(baos, vert.tex.x, 1 - vert.tex.y);
				}
			}

			data = baos.toByteArray();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
		return data;
	}
}
