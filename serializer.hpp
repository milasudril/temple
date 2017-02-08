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
				virtual void itemProcess(Callback& cb) const=0;
				virtual ~VisitorBase()=default;
			};

		template<class Sink,class Callback,class Container>
		class Visitor:public VisitorBase<Sink,Callback>
			{
			public:
				explicit Visitor(Container&&)=delete;
				explicit Visitor(const Container& container):
					m_begin(container.begin()),m_current(container.begin()),m_end(container.end())
					{}

				bool atEnd() const noexcept
					{return m_current==m_end;}

				bool atBegin() const noexcept
					{return m_current==m_begin;}

				void advance() noexcept
					{++m_current;}

				void itemProcess(Callback& cb) const
					{cb(*m_current,*this);}

				static auto create(const Container& cnt)
					{
					return std::unique_ptr<VisitorBase<Sink,Callback>>(new Visitor(cnt));
					}

			private:
			typename Container::const_iterator m_begin;
				typename Container::const_iterator m_current;
				typename Container::const_iterator m_end;
			};
		
		template<class StorageModel,class Sink>
		class Acceptor
			{
			public:
				using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
				using CompoundArray=typename StorageModel::template ArrayType<MapType>;

				using VisitorArray=Visitor<Sink,Acceptor,CompoundArray>;
				using VisitorMap=Visitor<Sink,Acceptor,MapType>;

				explicit Acceptor(ItemBase<StorageModel>&&)=delete;

				explicit Acceptor(const ItemBase<StorageModel>& root,Sink& sink):r_sink(sink)
					{
					if(root.array())
						{m_stack.push(VisitorArray::create(root.template value<CompoundArray>() ) );}
					else
						{}
					}

				void run()
					{
					while(!m_stack.empty())
						{
						auto visitor=std::move(m_stack.top());
						m_stack.pop();
						visitor->itemProcess(*this);
						if(!visitor->atEnd())
							{
							visitor->advance();
							m_stack.push(std::move(visitor));
							}
						}
					}

				void operator()(const MapType& node_current,const VisitorArray& visitor)
					{
					if(visitor.atEnd())
						{
						putc(']',r_sink);
						return;
						}

					if(visitor.atBegin())
						{putc('[',r_sink);}

					}

			private:
				Sink& r_sink;
				std::stack< std::unique_ptr< VisitorBase<Sink,Acceptor> > > m_stack;
			};
		}

	template<class StorageModel,class Sink>
	void temple_store(const ItemBase<StorageModel>& root,Sink& sink)
		{Acceptor<StorageModel,Sink>(root,sink).run();}
	}

#endif
