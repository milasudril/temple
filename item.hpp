#ifndef TEMPLE_ITEM_HPP
#define TEMPLE_ITEM_HPP

#include "type.hpp"
#include <utility>

namespace Temple
	{
	template<class T,class StorageModel>
	class Item;

	template<class StorageModel>
	class ItemBase
		{
		public:
			Type type() const noexcept
				{return m_type;}

			template<class T>
			bool has() const noexcept
				{return IdGet<T,StorageModel>::id==m_type;}

			template<class T>
			const T& value() const noexcept
				{
				assert(has<T>());
				auto self=reinterpret_cast<const Item<T,StorageModel>*>(this);
				return self->value();
				}

			template<class T>
			T& value() noexcept
				{return const_cast<T*>(const_cast<const ItemBase*>(this))->value();}

		protected:
			template<class T>
			ItemBase() noexcept:m_type(IdGet<T,StorageModel>::id)
				{}

		private:
			Type m_type;
		};

	template<class T,class StorageModel>
	class Item:public ItemBase<StorageModel>
		{
		public:
			typedef T value_type;

			explicit Item(T&& value) noexcept:ItemBase<StorageModel>()
				,m_value(std::move(value))
				{}

			const value_type& value() const noexcept
				{return m_value;}

			value_type& value() noexcept
				{return m_value;}

		private:
			T m_value;
		};
	}
#endif