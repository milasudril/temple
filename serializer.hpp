//@ {"targets":[{"name":"serializer.hpp","type":"include"}]}

#ifndef TEMPLE_SERIALIZER_HPP
#define TEMPLE_SERIALIZER_HPP

#include "item.hpp"
#include <stack>

namespace Temple
	{
	template<class StorageModel,class Sink>
	void temple_store(const ItemBase<StorageModel>& root,Sink& sink)
		{
		using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		using CompoundArray=typename StorageModel::template ArrayType<MapType>;

		std::stack<
			std::pair<const ItemBase<StorageModel>*,char>
		> nodes;
		nodes.push({&root,'\0'});
		while(!nodes.empty())
			{
			auto node_current=nodes.top();
			nodes.pop();
			if(node_current.second!='\0')
				{putc(node_current.second,sink);}
			else
				{
				if(node_current.first->array())
					{
					putc('[',sink);
					nodes.push({node_current.first,']'});
					auto& vals=node_current.first->template value<CompoundArray>();
				
					}
				}
			}
		}
	}

#endif
