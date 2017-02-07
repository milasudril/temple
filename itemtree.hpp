//@	{"targets":[{"name":"itemtree.hpp","type":"include"}]}

#ifndef TEMPLE_ITEMTREE_HPP
#define TEMPLE_ITEMTREE_HPP

#include "parser.hpp"

#include <cassert>
#include <map>
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
		using MapType=std::map<KeyType,ItemType>;
		};

	template<class StorageModel=BasicStorage>
	class ItemTree
		{
		public:
			template<class Source,class ProgressMonitor,class BufferType=std::string>
			ItemTree(Source&& src,ProgressMonitor&& m):
				m_root(temple_load<StorageModel,BufferType>(src,m))
				{}

		private:
			std::unique_ptr<ItemBase<StorageModel>> m_root;
		};
	}

#endif
