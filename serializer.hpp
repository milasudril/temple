//@ {"targets":[{"name":"serializer.hpp","type":"include"}]}

#ifndef TEMPLE_SERIALIZER_HPP
#define TEMPLE_SERIALIZER_HPP

#include "item.hpp"
#include "treenode.hpp"
#include <stack>

namespace Temple
	{
	namespace
		{
		template<class Sink,class Callback>
		class VisitorBase
			{
			public:
				virtual bool atEnd() const noexcept=0;
				virtual void advance() noexcept=0;
				virtual void terminate(Sink& sink) const=0;
				virtual void nodeProcess(Callback&& cb) const=0;
				virtual ~VisitorBase()=default;
			};

		template<class Sink,class Callback,class Container>
		class Visitor:public VisitorBase<Sink,Callback>
			{
			public:
				explicit Visitor(Container&&)=delete;
				explicit Visitor(const Container& container):
					m_current(container.begin()),m_end(container.end())
					{}

				bool atEnd() const noexcept
					{return m_current==m_end;}

				void advance() noexcept
					{++m_current;}

				void nodeProcess(Callback&& cb) const
					{cb(*m_current,*this);}

			private:
				typename Container::const_iterator m_current;
				typename Container::const_iterator m_end;
			};

		class Acceptor
			{
			};
		}

	template<class StorageModel,class Sink>
	void temple_store(const ItemBase<StorageModel>& root,Sink& sink)
		{
		using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		using CompoundArray=typename StorageModel::template ArrayType<MapType>;

		using VisitorArray=Visitor<Sink,Acceptor,CompoundArray>;
		using VisitorMap=Visitor<Sink,Acceptor,MapType>;
		}
	}

#endif
