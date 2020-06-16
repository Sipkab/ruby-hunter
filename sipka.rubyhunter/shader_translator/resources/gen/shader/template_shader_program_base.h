#ifndef GEN_SHADER_@program.getName()@_H_
#define GEN_SHADER_@program.getName()@_H_

#include <framework/render/ShaderProgram.h>
#include <framework/resource/Resource.h>
#include <framework/geometry/Vector.h>
#include <framework/geometry/Matrix.h>

#include <@vertex.getClassUrl().getIncludePath()@>
#include <@fragment.getClassUrl().getIncludePath()@>
#include <cstddef>

namespace rhfw {
namespace render {
	class VertexBuffer;
}  // namespace render
class @program.getName()@ : public render::ShaderProgram {
private:
	template<template<typename > class Checker, typename ... Types>
	class firstindex_of_type_check {
	public:
		template<int current_index, typename ... Remain>
		class next_index;

		template<int current_index>
		class next_index<current_index> {
		public:
			static const int value = -1;
			typedef void IndexedType;
		};

		template<int current_index, typename ToTest, typename ... Remain>
		class next_index<current_index, ToTest, Remain...> {
		public:
			template<bool selector, typename OnTrue, typename OnFalse>
			class TypeSelector;

			template<typename OnTrue, typename OnFalse>
			class TypeSelector<true,OnTrue,OnFalse> {
			public:
				typedef OnTrue SelectedType;
			};
			template<typename OnTrue, typename OnFalse>
			class TypeSelector<false,OnTrue,OnFalse> {
			public:
				typedef OnFalse SelectedType;
			};

			static const int value = Checker<ToTest>::value ? current_index : next_index<current_index + 1, Remain...>::value;
			typedef typename TypeSelector<Checker<ToTest>::value, ToTest, typename next_index<current_index + 1, Remain...>::IndexedType>::SelectedType IndexedType;
		};

		static const int value = next_index<0, Types...>::value;
		typedef typename next_index<0, Types...>::IndexedType FoundType;
	};

public:
	class VertexInput {
	public:
		@foreach input in vertex.getInputVariables() :
			@input.getType().getFrameworkType()@ @input.getName()@;
		@
	};
	class InputLayout : public render::InputLayout {
		friend class @program.getName()@;
	private:
	protected:
	public:
		struct InputDefinition {
			unsigned int bufferIndex;
			unsigned int offset;
		};
		@var vsinputsize = vertex.getInputVariables().size()@
		Resource<render::VertexBuffer> buffers[@vsinputsize@];
		unsigned int strides[@vsinputsize@];
		unsigned int bufferCount = 0;
		InputDefinition inputs[@vsinputsize@];

		@replace:shader_program_base_inputlayout_members@

		InputLayout& operator=(const InputLayout& o) {
			this->bufferCount = o.bufferCount;
			for(unsigned int i = 0; i < bufferCount; ++i) {
				buffers[i] = o.buffers[i];
				strides[i] = o.strides[i];
				inputs[i] = o.inputs[i];
			}
			for(unsigned int i = bufferCount; i < @vsinputsize@; ++i) {
				inputs[i] = o.inputs[i];
			}
			return *this;
		}
	};

private:
	@foreach input in vertex.getInputVariables() :
		@include:has_public_member.cpp ( has_public_member_name : input.getName() ) @
	@
public:
	@foreach uniform in vertex.getUniforms() :
		@var uname = uniform.getName()@
		@var vsname = vertex.getClassUrl().getExactClassName()@
		using @uname@ = @vsname@::@uname@;
		using @uname@_value = @vsname@::@uname@_value;
		Resource<@uname@> createUniform_@uname@(){
			return static_cast<@vsname@*>(vertexShader)->createUniform_@uname@();
		}
		virtual void set(@uname@& uniform) = 0;
	@
	@foreach uniform in fragment.getUniforms() :
		@var uname = uniform.getName()@
		@var fsname = fragment.getClassUrl().getExactClassName()@
		using @uname@ = @fsname@::@uname@;
		using @uname@_value = @fsname@::@uname@_value;
		Resource<@uname@> createUniform_@uname@(){
			return static_cast<@fsname@*>(fragmentShader)->createUniform_@uname@();
		}
		virtual void set(@uname@& uniform) = 0;
	@
protected:
	virtual void updateMovedResources() override {
		for (auto* o : inputLayouts.nodes()) {
			auto* res = static_cast<TrackingResourceBlock<render::InputLayout>*>(o);
			auto* nres = createInputLayoutImpl();
			auto* oldres = static_cast<InputLayout*>(res->replace(nres));
			*nres = util::move(*oldres);
			if (res->isLoaded()) {
				nres->load();
			}
			delete oldres;
		}
	}
private:

public:
	virtual InputLayout* createInputLayoutImpl() override = 0;
};

}  // namespace rhfw

#endif
