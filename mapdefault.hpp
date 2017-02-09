//@	{"targets":[{"name":"mapdefault.hpp","type":"include"}]}

#ifndef TEMPLE_MAPDEFAULT_HPP
#define TEMPLE_MAPDEFAULT_HPP

#include "error.hpp"
#include "type.hpp"
#include <map>

namespace Temple
	{
	template<class KeyType,class ItemType>
	class MapDefault:private std::map<KeyType,ItemType>
		{
		public:
			using std::map<KeyType,ItemType>::insert;
			using std::map<KeyType,ItemType>::find;
			using std::map<KeyType,ItemType>::size;
			using std::map<KeyType,ItemType>::begin;
			using std::map<KeyType,ItemType>::end;
			using std::map<KeyType,ItemType>::value_type;
			using std::map<KeyType,ItemType>::key_type;
			using std::map<KeyType,ItemType>::iterator;
			using std::map<KeyType,ItemType>::const_iterator;
			using std::map<KeyType,ItemType>::emplace;

			template<class Type,class ExceptionHandler>
			const Type& find(const KeyType& key,ExceptionHandler&& eh) const
				{
				auto i=find(key);
				if(i==end())
					{raise(Error("Key «",key.c_str(),"» not found"),eh);}
				auto x=i->second.get();
				if(!x->template has<Type>())
					{raise(Error("Key «",key.c_str(),"» does not map to a value of type the given type."),eh);}
				return x->template value<Type>();
				}


			template<class Type,class ExceptionHandler>
			Type& find(const KeyType& key,ExceptionHandler&& eh)
				{
				return const_cast<Type&>(static_cast<const MapDefault&>(*this).find<Type>(key,eh));
				}
			
		};
	}

#endif