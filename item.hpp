//@	{"targets":[{"name":"item.hpp","type":"include"}]}

#ifndef TEMPLE_ITEM_HPP
#define TEMPLE_ITEM_HPP

#include "type.hpp"
#include "converters.hpp"
#include <utility>
#include <memory>
#include <cassert>

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

			bool array() const noexcept
				{return m_type==arraySet(m_type);}

			template<class T>
			const T& value() const noexcept
				{
				assert(has<T>());
				auto self=reinterpret_cast<const Item<T,StorageModel>*>(this);
				return self->value();
				}

			template<class T>
			T& value() noexcept
				{
				assert(has<T>());
				auto self=reinterpret_cast<Item<T,StorageModel>*>(this);
				return self->value();
				}

			~ItemBase()
				{
				for_type<StorageModel,Type::I8,1,Type::COMPOUND_ARRAY>(m_type,[this](auto tag)
					{
					using T=typename decltype(tag)::type;
				//	Yes, we must call DTOR here, since the subclass is never used directly.
					this->value<T>().~T();
					});
				}

		protected:
			ItemBase(Type type) noexcept:m_type(type)
				{}

		private:
			Type m_type;
		};

	template<class T,class StorageModel>
	class Item:public ItemBase<StorageModel>
		{
		public:
			typedef T value_type;

			static std::unique_ptr<ItemBase<StorageModel>> create()
				{return std::unique_ptr<ItemBase<StorageModel>>(new Item);}

			const value_type& value() const noexcept
				{return m_value;}

			value_type& value() noexcept
				{return m_value;}

		private:
			explicit Item() noexcept:ItemBase<StorageModel>(IdGet<T,StorageModel>::id){}
			T m_value;
		};

	template<class StorageModel>
	auto itemCreate(Type type)
		{
		std::unique_ptr<ItemBase<StorageModel>> ret;
		for_type<StorageModel,Type::I8,1,Type::COMPOUND_ARRAY>(type,[&ret](auto tag)
			{
			using T=typename decltype(tag)::type;
			ret=Item<T,StorageModel>::create();
			});
		return std::move(ret);
		}

	template<class StorageModel,class BufferType,class ExceptionHandler>
	auto itemCreate(Type type,const BufferType& value,ExceptionHandler& eh)
		{
		std::unique_ptr<ItemBase<StorageModel>> ret;
		for_type<StorageModel,Type::I8,2,Type::STRING>(type,[&ret,&value,&eh](auto tag)
			{
			using T=typename decltype(tag)::type;
			ret=Item<T,StorageModel>::create();
			ret->template value<T>()=convert<T>(value,eh);
			});
		return std::move(ret);
		}
	}

#endif
