//@	{"targets":[{"name":"treenode.hpp","type":"include"}]}

#ifndef TEMPLE_TREENODE_HPP
#define TEMPLE_TREENODE_HPP

#include <type_traits>

namespace Temple
	{
	template<class ArrayType,class MapType>
	class TreeNode
		{
		public:
			TreeNode():m_container(nullptr),m_array(0){}
				
			template<class T>
			explicit TreeNode(T& container):m_container(&container)
				,m_array(std::is_same<T,ArrayType>::value)
				{static_assert(std::is_same<T,ArrayType>::value || std::is_same<T,MapType>::value,"");}

			template<class BufferType,class ItemType,class ExceptionHandler>
			auto& insert(const BufferType& key,ItemType&& item,ExceptionHandler& eh)
				{
				assert(m_container);
				assert(!m_array);
				auto map=reinterpret_cast<MapType*>(m_container);
				auto ret=item.get();
				if(!map->emplace(typename MapType::key_type(key),std::move(item)).second)
					{raise(Error("Key «",key.c_str(),"» already exists in the current block."),eh);}
				return *ret;
				}

			auto& append()
				{
				assert(m_container);
				assert(m_array);
				auto array=reinterpret_cast<ArrayType*>(m_container);
				array->emplace_back(typename ArrayType::value_type{});
				return array->back();
				}

			bool array() const noexcept
				{return m_array;}

			template<class T>
			T& container() noexcept
				{return *reinterpret_cast<T*>(m_container);}

			const void* pointer() const noexcept
				{return m_container;}

		
		private:
			void* m_container;
			bool m_array;
		};
	}

#endif
