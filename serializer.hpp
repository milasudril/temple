//@ {"targets":[{"name":"serializer.hpp","type":"include"}]}

#ifndef TEMPLE_SERIALIZER_HPP
#define TEMPLE_SERIALIZER_HPP

#include "item.hpp"
#include "treenode.hpp"
#include <stack>

namespace Temple
	{
	template<class StorageModel,class MapType,class Sink>
	void keysProcess(const MapType& map,Sink& sink);

	template<class StorageModel,class MapType,class ArrayType,class Sink>
	void elementsProcess(const ArrayType& array,Sink& sink)
		{
		putc('[',sink);
		auto ptr=array.begin();
		auto ptr_end=array.end();
		if(ptr!=ptr_end)
			{
			keysProcess<StorageModel,MapType>(*ptr,sink);
			++ptr;
			}

		while(ptr!=ptr_end)
			{
			putc('\n',sink);
			putc(',',sink);
			keysProcess<StorageModel,MapType>(*ptr,sink);
			++ptr;
			}
		putc(']',sink);
		}

	template<class T,class Sink>
	void write(const T& data,Sink& sink)
		{}

	template<class StorageModel,class MapType,class Sink>
	void keyProcess(const typename MapType::value_type& pair,Sink& sink)
		{
		using CompoundArray=typename StorageModel::template ArrayType<MapType>;

		auto type_current=pair.second->type();
		fprintf(sink,"%s,%s:",pair.first.c_str(),type(type_current));
		if(type_current==Type::COMPOUND)
			{keysProcess<StorageModel,MapType>(pair.second->template value<MapType>(),sink);}
		else
		if(type_current==Type::COMPOUND_ARRAY)
			{elementsProcess<StorageModel,MapType>(pair.second->template value<CompoundArray>(),sink);}
		else
			{
			auto& handle=pair.second;
			for_type<StorageModel,Type::I8,1,Type::STRING_ARRAY>(type_current,[&handle,&sink](auto tag)
				{
				using TypeCurrent=typename decltype(tag)::type;
				write(handle->template value<TypeCurrent>(),sink);
				});
			}
		}

	template<class StorageModel,class MapType,class Sink>
	void keysProcess(const MapType& map,Sink& sink)
		{
		putc('{',sink);
		auto ptr=map.begin();
		auto ptr_end=map.end();
		if(ptr!=ptr_end)
			{
			keyProcess<StorageModel,MapType>(*ptr,sink);
			++ptr;
			}

		while(ptr!=ptr_end)
			{
			putc('\n',sink);
			putc(',',sink);
			keyProcess<StorageModel,MapType>(*ptr,sink);
			++ptr;
			}
		putc('}',sink);
		}

	template<class StorageModel,class Sink>
	void temple_store(const ItemBase<StorageModel>& root,Sink& sink)
		{
		using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		using CompoundArray=typename StorageModel::template ArrayType<MapType>;
//		using NodeType=TreeNodeConst<CompoundArray,MapType>;

		if(root.array())
			{elementsProcess<StorageModel,MapType>(root.template value<CompoundArray>(),sink);}
		else
			{keysProcess<StorageModel,MapType>(root.template value<MapType>(),sink);}
		}
	}

#endif
