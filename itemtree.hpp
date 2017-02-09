//@	{"targets":[{"name":"itemtree.hpp","type":"include"}]}

#ifndef TEMPLE_ITEMTREE_HPP
#define TEMPLE_ITEMTREE_HPP

#include "parser.hpp"
#include "serializer.hpp"
#include "mapdefault.hpp"

#include <cassert>
#include <vector>
#include <string>

namespace Temple
	{
	struct BasicStorage
		{
		template<class T>
		using ArrayType=std::vector<T>;

		using StringType=std::string;

		using KeyType=std::string;

		template<class ItemType>
		using MapType=MapDefault<BasicStorage,ItemType>;
		};

	template<class StorageModel=BasicStorage>
	class ItemTree
		{
		public:
			using KeyType=typename StorageModel::KeyType;
			template<class T>
			using ArrayType=typename StorageModel::template ArrayType<T>;
			using Compound=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
			using CompoundArray=ArrayType<Compound>;

			template<class Source,class ExceptionHandler,class BufferType=std::string>
			ItemTree(Source&& src,ExceptionHandler&& eh):
				m_root(temple_load<StorageModel,BufferType>(src,eh))
				{}

			template<class Sink>
			void store(Sink&& sink)
				{temple_store(*m_root.get(),sink);}

			ItemBase<StorageModel>& root() noexcept
				{return *m_root.get();}

			const ItemBase<StorageModel>& root() const noexcept
				{return *m_root.get();}

		private:
			std::unique_ptr<ItemBase<StorageModel>> m_root;
		};

	template<class Type,class ExceptionHandler,class StorageModel,class ... Path>
	Type& find_typed(ExceptionHandler&& eh,ItemBase<StorageModel>& root,const Path&...path)
		{
		auto& x=find(eh,root,path...);
		if(!x.template has<Type>())
			{
			raise(Error("Requested item is not a ",type(IdGet<Type,StorageModel>::id),"."),eh);
			}
		return x.template value<Type>();
		}

	template<class Type,class ExceptionHandler,class StorageModel,class ... Path>
	const Type& find_typed(ExceptionHandler&& eh,const ItemBase<StorageModel>& root
		,const Path&...path)
		{
		auto& x=find(eh,root,path...);
		if(!x.template has<Type>())
			{
			raise(Error("Requested item is not a ",type(IdGet<Type,StorageModel>::id),"."),eh);
			}
		return x.template value<Type>();
		}

	template<class ExceptionHandler,class StorageModel>
	auto& find(ExceptionHandler&& eh,ItemBase<StorageModel>& root)
		{return root;}

	template<class ExceptionHandler,class StorageModel,class ... Path>
	auto& find(ExceptionHandler&& eh,ItemBase<StorageModel>& root
		,const typename StorageModel::KeyType& key
		,const Path&...path)
		{
		using Compound=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		if(root.template has<Compound>())
			{
			auto& map=root.template value<Compound>();
			return find(eh,map.find(key,eh),path...);
			}
		raise(Error("Current node is not a compound."),eh);
		}

	template<class ExceptionHandler,class StorageModel,class ... Path>
	const auto& find(ExceptionHandler&& eh,const ItemBase<StorageModel>& root
		,const typename StorageModel::KeyType& key
		,const Path&...path)
		{return find(eh,const_cast<ItemBase<StorageModel>&>(root),key,path...);}

	template<class ExceptionHandler,class MapType,class ... Path>
	auto& find(ExceptionHandler&& eh,MapType& map
		,const typename MapType::key_type& key
		,const Path&...path)
		{return find(eh,map.find(key,eh),path...);}

	template<class ExceptionHandler,class StorageModel>
	auto& find(ExceptionHandler&& eh,ItemBase<StorageModel>& root,size_t index)
		{
		using Compound=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		using CompoundArray=typename StorageModel::template ArrayType<Compound>;

		if(root.template has<CompoundArray>())
			{
			auto& a=root.template value<CompoundArray>();
			if(index>=a.size())
				{raise(Error("Array index out of bounds."),eh);}
			return a[index];
			}
		raise(Error("Current node is not an array."),eh);
		}

	template<class ExceptionHandler,class StorageModel>
	const auto& find(ExceptionHandler&& eh,ItemBase<StorageModel>& root,size_t index)
		{return find(eh,const_cast<ItemBase<StorageModel>&>(root),index);}

	template<class ExceptionHandler,class StorageModel,class ... Path>
	auto& find(ExceptionHandler&& eh,ItemBase<StorageModel>& root,size_t index
		,const Path&...path)
		{
		using Compound=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		using CompoundArray=typename StorageModel::template ArrayType<Compound>;

		if(root.template has<CompoundArray>())
			{
			auto& a=root.template value<CompoundArray>();
			if(index>=a.size())
				{raise(Error("Array index out of bounds."),eh);}
			return find(eh,a[index],path...);
			}
		raise(Error("Current node is not an array."),eh);
		}

	template<class ExceptionHandler,class StorageModel,class ... Path>
	const auto& find(ExceptionHandler&& eh,const ItemBase<StorageModel>& root,size_t index
		,const Path&...path)
		{return find(eh,const_cast<ItemBase<StorageModel>&>(root),index,path...);}
	}

#endif
