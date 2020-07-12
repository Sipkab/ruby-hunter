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

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.UncheckedIOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;
import java.util.stream.Collectors;

import bence.sipka.compiler.types.builtin.FloatType;
import bence.sipka.compiler.types.builtin.IntegerType;
import saker.build.file.path.SakerPath;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;

public class ObjectConfiguration implements Externalizable {
	private static final long serialVersionUID = 1L;

	private ObjectCollection objectCollection;

	NavigableMap<Integer, ObjectDescriptor> descriptors = new TreeMap<>();
	List<Material> colorMaterials = new ArrayList<>();
	List<Material> textureMaterials = new ArrayList<>();

	/**
	 * For {@link Externalizable}.
	 */
	public ObjectConfiguration() {
	}

	public ObjectConfiguration(ObjectCollection objectCollection) {
		this.objectCollection = objectCollection;
		this.colorMaterials.addAll(objectCollection.colorMaterials);
		this.textureMaterials.addAll(objectCollection.textureMaterials);
	}

	public ByteArrayRegion getBytes(SakerPath workingDirectoryPath, Map<String, Integer> assetsIdentifierMap) {
		return init(workingDirectoryPath, assetsIdentifierMap);
	}

	public ObjectDescriptor getDescriptor(int id) {
		return descriptors.get(id);
	}

	private static class ObjectFace {
		public final ObjectData obj;
		public final Face face;

		public ObjectFace(ObjectData obj, Face face) {
			this.obj = obj;
			this.face = face;
		}

	}

	private ByteArrayRegion init(SakerPath workingDirectoryPath, Map<String, Integer> assetsIdentifierMap) {
		ByteArrayRegion data;
		List<ObjectFace> faces = objectCollection.objects.stream()
				.flatMap(o -> o.faces.stream().map(f -> new ObjectFace(o, f))).collect(Collectors.toList());
		Map<Material, List<ObjectFace>> matfaces = faces.stream().collect(Collectors.groupingBy(f -> f.face.material));

		Map<Material, List<ObjectFace>> texturedmatfaces = matfaces.entrySet().stream()
				.filter(e -> e.getKey().hasTexture()).collect(Collectors.toMap(e -> e.getKey(), e -> e.getValue()));
		Map<Material, List<ObjectFace>> coloredmatfaces = matfaces.entrySet().stream()
				.filter(e -> !e.getKey().hasTexture()).collect(Collectors.toMap(e -> e.getKey(), e -> e.getValue()));

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

		Comparator<ObjectFace> facesorter = (a, b) -> {
			int cmp = Integer.compare(a.obj.id, b.obj.id);
			if (cmp != 0) {
				return cmp;
			}
			return a.face.material.getName().compareTo(b.face.material.getName());
		};

		List<ObjectFace> coloredfaces = coloredmatfaces.values().stream().flatMap(f -> f.stream()).sorted(facesorter)
				.collect(Collectors.toList());
		List<ObjectFace> texturedfaces = texturedmatfaces.values().stream().flatMap(f -> f.stream()).sorted(facesorter)
				.collect(Collectors.toList());

		int coloredfacecount = coloredfaces.size();
		int texturedfacecount = texturedfaces.size();
		int coloredmatcount = colorMaterials.size();
		int texturedmatcount = textureMaterials.size();

		Map<ObjectData, List<ObjectFace>> coloredobjects = coloredfaces.stream()
				.collect(Collectors.groupingBy(f -> f.obj));
		Map<ObjectData, List<ObjectFace>> texturedobjects = texturedfaces.stream()
				.collect(Collectors.groupingBy(f -> f.obj));
		Map<ObjectData, Map<Material, List<ObjectFace>>> coloredranges = coloredobjects.entrySet().stream()
				.collect(Collectors.toMap(e -> e.getKey(),
						e -> e.getValue().stream().collect(Collectors.groupingBy(f -> f.face.material))));
		Map<ObjectData, Map<Material, List<ObjectFace>>> texturedranges = texturedobjects.entrySet().stream()
				.collect(Collectors.toMap(e -> e.getKey(),
						e -> e.getValue().stream().collect(Collectors.groupingBy(f -> f.face.material))));
		for (Entry<ObjectData, Map<Material, List<ObjectFace>>> e : coloredranges.entrySet()) {
			ObjectDescriptor desc = descriptors.get(e.getKey().id);
			if (desc == null) {
				desc = new ObjectDescriptor();
				descriptors.put(e.getKey().id, desc);
			}
			for (Entry<Material, List<ObjectFace>> mate : e.getValue().entrySet()) {
				desc.colored++;
				desc.triangles
						.add(new ObjectDescriptor.TriangleRegion(mate.getKey(), colorMaterials.indexOf(mate.getKey()),
								coloredfaces.indexOf(mate.getValue().get(0)) * 3, mate.getValue().size() * 3));
			}
		}
		for (Entry<ObjectData, Map<Material, List<ObjectFace>>> e : texturedranges.entrySet()) {
			ObjectDescriptor desc = descriptors.get(e.getKey().id);
			if (desc == null) {
				desc = new ObjectDescriptor();
				descriptors.put(e.getKey().id, desc);
			}
			for (Entry<Material, List<ObjectFace>> mate : e.getValue().entrySet()) {
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
			for (ObjectFace f : coloredfaces) {
				for (Iterator<Vertex> it = f.face.vertexIterator(f.obj); it.hasNext();) {
					Vertex vert = it.next();
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
			for (ObjectFace f : texturedfaces) {
				for (Iterator<Vertex> it = f.face.vertexIterator(f.obj); it.hasNext();) {
					Vertex vert = it.next();
					vert.pos.normalizeW().serializeXYZ(baos);
					vert.norm.normalizeLength().serializeXYZ(baos);
					// vert.tex.normalizeW().serializeXY(baos);
					Vector.serializeXY(baos, vert.tex.x, 1 - vert.tex.y);
				}
			}

			data = baos.toByteArrayRegion();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
		return data;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(objectCollection);
		SerialUtils.writeExternalMap(out, descriptors);
		SerialUtils.writeExternalCollection(out, colorMaterials);
		SerialUtils.writeExternalCollection(out, textureMaterials);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		objectCollection = SerialUtils.readExternalObject(in);
		descriptors = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		colorMaterials = SerialUtils.readExternalImmutableList(in);
		textureMaterials = SerialUtils.readExternalImmutableList(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((colorMaterials == null) ? 0 : colorMaterials.hashCode());
		result = prime * result + ((descriptors == null) ? 0 : descriptors.hashCode());
		result = prime * result + ((objectCollection == null) ? 0 : objectCollection.hashCode());
		result = prime * result + ((textureMaterials == null) ? 0 : textureMaterials.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		ObjectConfiguration other = (ObjectConfiguration) obj;
		if (colorMaterials == null) {
			if (other.colorMaterials != null)
				return false;
		} else if (!colorMaterials.equals(other.colorMaterials))
			return false;
		if (descriptors == null) {
			if (other.descriptors != null)
				return false;
		} else if (!descriptors.equals(other.descriptors))
			return false;
		if (objectCollection == null) {
			if (other.objectCollection != null)
				return false;
		} else if (!objectCollection.equals(other.objectCollection))
			return false;
		if (textureMaterials == null) {
			if (other.textureMaterials != null)
				return false;
		} else if (!textureMaterials.equals(other.textureMaterials))
			return false;
		return true;
	}
}
