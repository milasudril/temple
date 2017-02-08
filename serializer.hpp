//@ {"targets":[{"name":"serializer.hpp","type":"include"}]}

#ifndef TEMPLE_SERIALIZER_HPP
#define TEMPLE_SERIALIZER_HPP

#include "item.hpp"
#include "treenode.hpp"
#include <stack>

namespace Temple
	{
	template<class CharType,class Sink>
	void write(const CharType* src,Sink& sink)
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
	void write(const typename StorageModel::StringType& string,Sink& sink)
		{
		putc('"',sink);
		write(string.c_str(),sink);
		putc('"',sink);
		}

	template<class StorageModel,class T,class Sink,std::enable_if_t<std::is_arithmetic<T>::value,int> a=0> 
	void write(T x,Sink& sink)
		{write(convert<std::string>(x).c_str(),sink);}

	template<class StorageModel,class T,class Sink>
	void write(const typename StorageModel::template ArrayType<T>& array,Sink& sink)
		{
		putc('[',sink);
		auto ptr=array.begin();
		auto ptr_end=array.end();
		if(ptr!=ptr_end)
			{
			write<StorageModel>(*ptr,sink);
			++ptr;
			}
		while(ptr!=ptr_end)
			{
			fputs(", ",sink);
			write<StorageModel>(*ptr,sink);
			++ptr;
			}

		putc(']',sink);
		}

	namespace
		{
		template<class Sink>
		void indent(size_t level,Sink& sink)
			{
			assert(level!=0);
			--level;
			while(level!=0)
				{
				putc('\t',sink);
				--level;
				}
			}

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
				size_t size() const noexcept
					{return m_size;}
			protected:
				explicit VisitorBase(char term,size_t size) noexcept:
					m_terminator(term),m_size(size)
					{}

			private:
				char m_terminator;
				size_t m_size;
			};

		template<class Callback,class Container>
		class Visitor:public VisitorBase<Callback>
			{
			public:
				explicit Visitor(Container&&)=delete;
				explicit Visitor(const Container& container,char term):
					 VisitorBase<Callback>(term,container.size())
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
							if(visitor->size()>1 
								|| (visitor->terminator()==']' && visitor->size()!=0))
								{indent(m_stack.size(),r_sink);}
							putc(visitor->terminator(),r_sink);
							putc('\n',r_sink);
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
					indent(m_stack.size(),r_sink);
					putc(visitor.atBegin()?'[':',',r_sink);
					putc('\n',r_sink);
					m_stack.push(VisitorMap::create(node_current,'}'));
					}

				void operator()(const typename MapType::value_type& node_current,const VisitorMap& visitor)
					{
					indent(m_stack.size(),r_sink);
					if(visitor.size()==1)
						{putc('{',r_sink);}
					else
						{
						fputs(visitor.atBegin()?"{\n":",",r_sink);
						if(visitor.atBegin())
							{
							indent(m_stack.size(),r_sink);
							putc(' ',r_sink);
							}
						}
					putc('"',r_sink);
					write(node_current.first.c_str(),r_sink);
					auto type_current=node_current.second->type();
					fprintf(r_sink,"\",%s:",type(type_current));
					if(type_current==Type::COMPOUND)
						{
						auto node=VisitorMap::create(node_current.second->template value<MapType>(),'}');
						putc('\n',r_sink);
						if(node->atEnd())
							{
							indent(m_stack.size()+1,r_sink);
							putc('{',r_sink);
							}
						m_stack.push(std::move(node));
						}
					else
					if(type_current==Type::COMPOUND_ARRAY)
						{
						auto node=VisitorArray::create(node_current.second->template value<CompoundArray>(),']');
						putc('\n',r_sink);
						if(node->atEnd())
							{
							indent(m_stack.size()+1,r_sink);
							putc('[',r_sink);
							}
						m_stack.push(std::move(node));
						}
					else
						{
						for_type<StorageModel,Type::I8,1,Type::STRING_ARRAY>(type_current,[&node_current,this,&visitor](auto tag)
							{
							using TypeCurrent=typename decltype(tag)::type;
							auto& object=node_current.second->template value<TypeCurrent>();
							write<StorageModel>(object,this->r_sink);
							if(visitor.size()!=1)
								{putc('\n',this->r_sink);}
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
		{
		Locale loc;
		Acceptor<StorageModel,Sink>(root,sink).run();
		}
	}

#endif
