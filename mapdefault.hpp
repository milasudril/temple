//@	{"targets":[{"name":"mapdefault.hpp","type":"include"}]}

#ifndef TEMPLE_MAPDEFAULT_HPP
#define TEMPLE_MAPDEFAULT_HPP

#include "error.hpp"
#include "type.hpp"
#include <map>

namespace Temple
	{
	template<class StorageModel,class ItemType>
	class MapDefault:private std::map<typename StorageModel::KeyType,ItemType>
		{
		public:
			using key_type=typename StorageModel::KeyType;
			using std::map<key_type,ItemType>::insert;
			using std::map<key_type,ItemType>::find;
			using std::map<key_type,ItemType>::size;
			using std::map<key_type,ItemType>::begin;
			using std::map<key_type,ItemType>::end;
			using std::map<key_type,ItemType>::value_type;
			using std::map<key_type,ItemType>::iterator;
			using std::map<key_type,ItemType>::const_iterator;
			using std::map<key_type,ItemType>::emplace;

			template<class Type,class ExceptionHandler>
			const Type& find_typed(const key_type& key,ExceptionHandler&& eh) const
				{
				auto& x=find(key,eh);
				if(!x.template has<Type>())
					{
					raise(Error("Key «",key.c_str(),"» does not map to a value of type "
						,type(IdGet<Type,StorageModel>::id),'.'),eh);
					}
				return x.template value<Type>();
				}

			template<class Type,class ExceptionHandler>
			Type& find_typed(const key_type& key,ExceptionHandler&& eh)
				{
				return const_cast<Type&>(static_cast<const MapDefault&>(*this).find_typed<Type>(key,eh));
				}

			template<class ExceptionHandler>
			const auto& find(const key_type& key,ExceptionHandler&& eh) const
				{
				auto i=find(key);
				if(i==end())
					{raise(Error("Key «",key.c_str(),"» not found"),eh);}
				return *(i->second.get());
				}

			template<class ExceptionHandler>
			auto& find(const key_type& key,ExceptionHandler&& eh)
				{
				return unconst_cast(static_cast<const MapDefault&>(*this).find(key,eh));
				}

		private:
			template<class T>
			static T& unconst_cast(const T &v) noexcept
				{return const_cast<T &>(v);}
		};
	}

#endif