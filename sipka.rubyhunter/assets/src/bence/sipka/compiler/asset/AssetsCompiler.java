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
//package bence.sipka.compiler.asset;
//
//import java.util.ArrayList;
//import java.util.Collection;
//import java.util.Comparator;
//import java.util.Map;
//import java.util.Optional;
//import java.util.TreeMap;
//
//import bence.sipka.compiler.CCompilerOptions;
//import bence.sipka.compiler.source.TemplatedSource;
//import bence.sipka.compiler.source.TemplatedSourceModularFile;
//import bence.sipka.compiler.types.TypeDeclaration;
//import bence.sipka.repository.FrameworkModule;
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularOutput;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.file.StrongDelegateModularFile;
//import modular.core.file.path.WildcardPath;
//import modular.core.module.ModuleOperation;
//
//@ModuleURI(AssetsCompiler.MODULE_URI)
//public class AssetsCompiler extends FrameworkModule {
//	public static final String MODULE_URI = "bence.sipka.compiler.assets";
//
//	@Operation("parse")
//	public class ParseOperation extends ModuleOperation {
//
//		// input for optional specifications
//		@ModularData
//		private Map<String, Integer> assetsIdentifierMap = new TreeMap<>();
//
//		@ModularData
//		private Collection<WildcardPath> ApplicationAssets = new ArrayList<>();
//
//		@ModularData
//		private Map<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
//
//		@ModularData
//		private Collection<CCompilerOptions> CGlobalCompilerOptions = new ArrayList<>();
//
//		@ModularOutput
//		private String ApplicationAssetsDirectory;
//
//		@Override
//		public void run() throws Exception {
//			int idprovider = 0;
//
//			ModularDirectory modulebuilddir = getModuleBuildModularDirectory();
//			ModularDirectory assetsdirectory = modulebuilddir.getDirectoryCreate("assets", ModularFile.FLAG_SHADOWFILE);
//			ModularDirectory assetsresdirectory = assetsdirectory.getDirectoryCreate("res", 0);
//			ModularDirectory sourcedir = modulebuilddir.getDirectoryCreate("sources");
//			ModularDirectory gendir = sourcedir.getDirectoryCreate("gen");
//
//			CCompilerOptions cppoptions = CCompilerOptions.create();
//			cppoptions.setLanguage("C++");
//			cppoptions.getIncludeDirectories().add(sourcedir.getModularPath().toString());
//			CGlobalCompilerOptions.add(cppoptions);
//
//			Optional<Integer> max = assetsIdentifierMap.values().stream().max(Comparator.naturalOrder());
//			if (max.isPresent()) {
//				idprovider = max.get() + 1;
//			}
//
//			for (ModularFile file : WildcardPath.getFiles(getContext(), ApplicationAssets)) {
//				if (file.isDirectory()) {
//					continue;
//				}
//				String assetpath = getContext().getWorkingModularDirectory().getRelativePath(file.getModularPath())
//						.toString().replace('\\', '/');
//
//				Integer id = assetsIdentifierMap.get(assetpath);
//				if (id == null) {
//					id = idprovider++;
//					assetsIdentifierMap.put(assetpath, id);
//				}
//
//				assetsresdirectory.add(StrongDelegateModularFile.create(Integer.toHexString(id), file));
//			}
//
//			AssetTypeDeclaration assettype = new AssetTypeDeclaration(assetsIdentifierMap);
//			typeDeclarations.put(assettype.getName(), assettype);
//
//			gendir.add(new TemplatedSourceModularFile("assets.h",
//					new TemplatedSource(descriptor::getInputStream, "gen/assets.h")).setThis(assetsIdentifierMap));
//
//			ApplicationAssetsDirectory = assetsresdirectory.getModularPath().toString();
//		}
//	}
//
//}
