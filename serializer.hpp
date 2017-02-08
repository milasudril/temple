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
		template<class Callback>
		class VisitorBase
			{
			public:
				virtual bool atEnd() const noexcept=0;
				virtual void advance() noexcept=0;
				virtual void itemProcess(Callback& cb) const=0;
				virtual ~VisitorBase()=default;
				char terminator() const noexcept
					{return m_terminator;}
			protected:
				explicit VisitorBase(char term) noexcept:
					m_terminator(term)
					{}

			private:
				char m_terminator;
			};

		template<class Callback,class Container>
		class Visitor:public VisitorBase<Callback>
			{
			public:
				explicit Visitor(Container&&)=delete;
				explicit Visitor(const Container& container,char term):
					 VisitorBase<Callback>(term)
					,m_begin(container.begin())
					,m_current(container.begin())
					,m_end(container.end())
					{}

				bool atEnd() const noexcept
					{return m_current==m_end;}

				bool atBegin() const noexcept
					{return m_current==m_begin;}

				void advance() noexcept
					{++m_current;}

				void itemProcess(Callback& cb) const
					{cb(*m_current,*this);}

				static auto create(const Container& cnt,char term)
					{return std::unique_ptr<VisitorBase<Callback>>(new Visitor(cnt,term));}

			private:
				typename Container::const_iterator m_begin;
				typename Container::const_iterator m_current;
				typename Container::const_iterator m_end;
			};

		template<class Cstr,class Sink>
		static void write(Cstr src,Sink& sink)
			{
			while(true)
				{
				auto ch_in=*src;
				if(ch_in=='\0')
					{return;}
				if(ch_in=='\\' || ch_in=='\"')
					{putc('\\',sink);}
				putc(ch_in,sink);
				++src;
				}
			}
		
		template<class StorageModel,class Sink>
		class Acceptor
			{
			public:
				using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
				using CompoundArray=typename StorageModel::template ArrayType<MapType>;

				using VisitorArray=Visitor<Acceptor,CompoundArray>;
				using VisitorMap=Visitor<Acceptor,MapType>;

				explicit Acceptor(ItemBase<StorageModel>&&)=delete;

				explicit Acceptor(const ItemBase<StorageModel>& root,Sink& sink):r_sink(sink)
					{
					if(root.array())
						{m_stack.push(VisitorArray::create(root.template value<CompoundArray>(),']') );}
					else
						{m_stack.push(VisitorMap::create(root.template value<MapType>(),'}'));}
					}

				void run()
					{
					while(!m_stack.empty())
						{
						auto& visitor=m_stack.top();
						if(visitor->atEnd())
							{
							putc(visitor->terminator(),r_sink);
							m_stack.pop();
							}
						else
							{
							visitor->itemProcess(*this);
							visitor->advance();	
							}
						}
					}

				void operator()(const MapType& node_current,const VisitorArray& visitor)
					{
					putc(visitor.atBegin()?'[':',',r_sink);
					m_stack.push(VisitorMap::create(node_current,'}'));
					}

				void operator()(const typename MapType::value_type& node_current,const VisitorMap& visitor)
					{
					putc(visitor.atBegin()?'{':',',r_sink);
					putc('"',r_sink);
					write(node_current.first.c_str(),r_sink);
					auto type_current=node_current.second->type();
					fprintf(r_sink,"\",%s:",type(type_current));
					if(type_current==Type::COMPOUND)
						{
						auto node=VisitorMap::create(node_current.second->template value<MapType>(),'}');
						putc('\n',r_sink);
						if(node->atEnd())
							{putc('{',r_sink);}
						m_stack.push(std::move(node));
						}
					else
					if(type_current==Type::COMPOUND_ARRAY)
						{
						auto node=VisitorArray::create(node_current.second->template value<CompoundArray>(),']');
						putc('\n',r_sink);
						if(node->atEnd())
							{putc('[',r_sink);}
						m_stack.push(std::move(node));
						}
					else
						{
						for_type<StorageModel,Type::I8,1,Type::STRING_ARRAY>(type_current,[&node_current](auto tag)
							{
							using TypeCurrent=typename decltype(tag)::type;
						//	write();
							});
						}
					}

			private:
				Sink& r_sink;
				std::stack< std::unique_ptr< VisitorBase<Acceptor> > > m_stack;
			};
		}

	template<class StorageModel,class Sink>
	void temple_store(const ItemBase<StorageModel>& root,Sink& sink)
		{Acceptor<StorageModel,Sink>(root,sink).run();}
	}

#endif
