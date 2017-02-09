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
		using MapType=MapDefault<KeyType,ItemType>;
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
	}

#endif
