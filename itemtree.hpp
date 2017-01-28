//@	{"targets":[{"name":"itemtree.hpp","type":"include"}]}

#ifndef TEMPLE_ITEMTREE_HPP
#define TEMPLE_ITEMTREE_HPP

#include "converters.hpp"
#include "type.hpp"

#include <map>
#include <vector>
#include <stack>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Temple
	{
	struct BasicStorage
		{
		template<class T>
		using ArrayType=std::vector<T>;
		using StringType=std::string;
		using BufferType=std::string;
		};

	template<class StorageModel=BasicStorage>
	class ItemTree
		{
		public:
			template<class T>
			using ArrayType=typename StorageModel::template ArrayType<T>;

			using StringType=typename StorageModel::StringType;

			using BufferType=typename StorageModel::BufferType;

			using Key=StringType;
			using KeyPointer=const typename Key::value_type*;

			static constexpr typename StringType::value_type pathsep() noexcept
				{return '\001';}

			ItemTree(const ItemTree&)=delete;
			ItemTree& operator=(const ItemTree&)=delete;

			template<class Source,class ProgressMonitor>
			ItemTree(Source&& src,ProgressMonitor&& monitor)
				{load(src,monitor);}

			template<class Source,class ProgressMonitor>
			ItemTree& load(Source&& src,ProgressMonitor& monitor)
				{return load(src,monitor);}

			template<class Source,class ProgressMonitor>
			ItemTree& load(Source& src,ProgressMonitor& monitor);

			template<class ItemProcessor,class ExceptionHandler>
			void itemsProcess(ItemProcessor&& proc,ExceptionHandler&& eh);

			template<class ItemProcessor,class ExceptionHandler>
			void itemsProcess(ItemProcessor&& proc,ExceptionHandler&& eh) const;

			template<class Sink,class ExceptionHandler>
			void store(Sink& sink,ExceptionHandler& eh) const 
				{
				Locale loc;
				size_t level_prev=0;
				std::stack<char> close_symb;
				itemsProcess([this,&close_symb,&sink,&level_prev](const auto& key,size_t child_count
					,auto tag,const auto& value)
					{
					auto level=this->count(key,pathsep());
					if(level==0)
						{
						putc(decltype(tag)::id==Type::COMPOUND_ARRAY?'[':'{',sink);
						close_symb.push(decltype(tag)::id==Type::COMPOUND_ARRAY?']':'}');
						return;
						}
					for(size_t k=0;k<level-1;++k)
						{putc('\t',sink);}
					while(level<level_prev)
						{
						putc(close_symb.top(),sink);
						close_symb.pop();
						--level_prev;
						}
					putc(level==level_prev?',':' ',sink);
					auto path_end=this->rfind(key,pathsep());
					RecordWrite<decltype(tag)::id>::doIt(close_symb,path_end,value,sink);
					level_prev=level;
					},eh);
				while(level_prev!=0)
					{
					putc(close_symb.top(),sink);
					close_symb.pop();
					--level_prev;
					}
				}

			template<class T>
			bool find(T*& ret,KeyPointer key) noexcept
				{
				static constexpr auto id=IdGet<T,StorageModel>::id;
				auto& data=dataGet<id>();
				auto i=data.find(key);
				if(i==data.end())
					{return 0;}
				ret=&i->second;
				return 1;
				}

			template<class T>
			bool find(const T*& ret,KeyPointer key) const noexcept
				{
				static constexpr auto id=IdGet<T,StorageModel>::id;
				auto& data=dataGet<id>();
				auto i=data.find(key);
				if(i==data.end())
					{return 0;}
				ret=&i->second;
				return 1;
				}


			template<class T,class KeyType>
			bool insert(T&& value,const KeyType& key)
				{
				auto path_end=rfind(key.begin(),key.end(),pathsep());
				assert(path_end!=nullptr);
				auto compound=Key(key.begin(),(path_end - 1) - key.begin());
				auto i=m_keys.find(compound);
				if(i==m_keys.end())
					{return 0;}
				if(i->second.type!=Type::COMPOUND)
					{return 0;}
				
				auto key_tot=Key(key.begin());
				auto i2=m_keys.find(key_tot);
				if(i2!=m_keys.end())
					{return 0;}
				static constexpr auto id=IdGet<std::remove_reference_t<T>,StorageModel>::id;
				i2=m_keys.insert({std::move(key_tot),{id,0}}).first;
				auto& data=dataGet<id>();
				data.insert({i2->first.c_str(),std::move(value)});
				++(i->second.child_count);
				return 1;
				}

			template<class T,class KeyType>
			bool insert(const T& value,const KeyType& key)
				{
				T copy(value);
				return insert(std::move(copy),key);
				}

			template<class KeyType>
			bool compoundInsert(const KeyType& name,bool array)
				{
				auto path_end=rfind(name.begin(),name.end(),pathsep());
				assert(path_end!=nullptr);
				auto parent=Key(name.begin(),(path_end - 1) - name.begin());
				auto i=m_keys.find(parent);
				if(i==m_keys.end())
					{return 0;}
				if(i->second.type!=Type::COMPOUND)
					{return 0;}
				auto key_tot=Key(name.begin());
				auto i2=m_keys.find(key_tot);
				if(i2!=m_keys.end())
					{return 0;}
				m_keys.insert({std::move(key_tot),{array?Type::COMPOUND_ARRAY:Type::COMPOUND,0}});
				++(i->second.child_count);
				return 1;
				}


			template<class KeyType>
			bool elementInsert(const KeyType& parent,bool array)
				{
				auto key_tot=Key(parent.begin());
				auto i=m_keys.find(key_tot);
				if(i==m_keys.end())
					{return 0;}
				if(i->second!=Type::COMPOUND_ARRAY)
					{return 0;}
				key_tot+=pathsep();
			#if 0
				key_tot+=idCreate(item_count);
				m_keys.insert({std::move(key_tot),array?Type::COMPOUND_ARRAY?TYPE::COMPOUND});
			#else
				return 0;
			#endif
				}
				

		private:
			template<Type t>
			auto& dataGet() noexcept
				{return dataGet<t>(m_data);}

			template<Type t>
			const auto& dataGet() const noexcept
				{return dataGet<t>(m_data);}

			template<class T>
			static constexpr bool equals_or(T x)
				{return 0;}

			template<class T,class ... U>
			static constexpr bool equals_or(T x,T y,U ... values)
				{return x==y || equals_or(x,values...);}

			template<class Cstr,class Sink,class... to_escape>
			static void write(Cstr src,Sink& sink,to_escape ... esc)
				{
				while(true)
					{
					auto ch_in=*src;
					if(ch_in=='\0')
						{return;}
					if(equals_or(ch_in,esc...))
						{putc('\\',sink);}
					putc(ch_in,sink);
					++src;
					}
				}

			template<class T,class Sink>
			static std::enable_if_t<std::is_arithmetic<T>::value>
			write(const T& value,Sink& sink)
				{
				auto buffer=convert<BufferType>(value);
				write(buffer.c_str(),sink);
				}

			template<class Sink>
			static void write(const StringType& value,Sink& sink)
				{
				putc('"',sink);
				write(value.c_str(),sink,'"','\\');
				putc('"',sink);
				}

			template<class T,class Sink>
			static void write(const ArrayType<T>& values,Sink& sink)
				{
				putc('[',sink);
				auto ptr=values.begin();
				auto ptr_end=values.end();
				size_t elem_count=0;
				while(ptr!=ptr_end)
					{
					if(elem_count!=0)
						{putc(',',sink);}
					write(*ptr,sink);
					++elem_count;
					if(elem_count%16==0)
						{putc('\n',sink);}
					++ptr;
					}
				putc(']',sink);
				}

			template<Type t,class dummy=void>
			struct RecordWrite
				{
				template<class Sink,class KeyCstr,class Value>
				static void doIt(std::stack<char>& close_symb,KeyCstr key,const Value& value,Sink& sink)
					{
					assert(key!=nullptr);
					putc('"',sink);
					write(key,sink,'"');
					fprintf(sink,"\"%s:",type(t));
					write(value,sink);
					putc('\n',sink);
					}
				};

			template<class dummy>
			struct RecordWrite<Type::COMPOUND,dummy>
				{
				template<class Sink,class KeyCstr,class Value>
				static void doIt(std::stack<char>& close_symb,KeyCstr key,const Value& value,Sink& sink)
					{
					assert(key!=nullptr);
					if(close_symb.top()=='}')
						{
						putc('"',sink);
						write(key,sink,'"');
						fputs("\":",sink);
						}
					fputs("{\n",sink);
					close_symb.push('}');
					}
				};

			template<class dummy>
			struct RecordWrite<Type::COMPOUND_ARRAY,dummy>
				{
				template<class Sink,class KeyCstr,class Value>
				static void doIt(std::stack<char>& close_symb,KeyCstr key,const Value& value,Sink& sink)
					{
					assert(key!=nullptr);
					if(close_symb.top()=='}')
						{
						putc('"',sink);
						write(key,sink,'"');
						fputs("\":",sink);
						}

					fputs("[\n",sink);
					close_symb.push(']');
					}
				};

			struct KeyCompare
				{
				bool operator()(KeyPointer a,KeyPointer b) const noexcept
					{return strcmp(a,b)<0;}
				};

			template<class KeyType,class Value,class Compare=KeyCompare>
			using MapType=std::map<KeyType,Value,Compare>;

			template<class T>
			static const T* rfind(const T* begin,const T* end,T ch) noexcept
				{
				while(end!=begin)
					{
					--end;
					if(*end==ch)
						{return end+1;}
					}
				return nullptr;
				}

			static const typename StringType::value_type* rfind(const StringType& str
				,typename StringType::value_type ch) noexcept
				{
				return rfind(str.data(),str.data() + str.size(),ch);
				}

			static size_t count(const StringType& str,typename StringType::value_type ch) noexcept
				{
				auto begin=str.begin();
				auto end=str.end();
				size_t n=0;
				while(end!=begin)
					{
					--end;
					if(*end==ch)
						{++n;}
					}
				return n;
				}

			static constexpr auto type_last=Type::COMPOUND;
			static constexpr auto type_first=Type::I8;

			template<Type t=previous(type_last),bool dummy=1>
			struct Data:public Data<previous(t),dummy>
				{
				Data():content(KeyCompare{}){}
				MapType<KeyPointer,typename TypeGet<t,StorageModel>::type> content;
				};

			template<bool dummy>
			struct Data<type_first,dummy>
				{
				Data():content(KeyCompare{}){}
				MapType<KeyPointer,typename TypeGet<type_first,StorageModel>::type> content;
				};

			Data<> m_data;
			
			template<Type t>
			static auto& dataGet(Data<t>& data) noexcept
				{return data.content;};

			template<Type t>
			static const auto& dataGet(const Data<t>& data) noexcept
				{return data.content;};

			template<Type t=previous(type_last),bool dummy=1>
			struct Iterators:public Iterators<previous(t),dummy>
				{
				explicit Iterators(Data<t>& data) noexcept:
					Iterators<previous(t),dummy>(data),m_position(data.content.begin())
					{}
				typename MapType<KeyPointer,typename TypeGet<t,StorageModel>::type>::iterator m_position;
				};

			template<bool dummy>
			struct Iterators<type_first,dummy>
				{
				explicit Iterators(Data<type_first>& data) noexcept:
					m_position(data.content.begin())
					{}
				typename MapType<KeyPointer,typename TypeGet<type_first,StorageModel>::type>::iterator m_position;
				};

			template<Type t=previous(type_last),bool dummy=1>
			struct IteratorsConst:public IteratorsConst<previous(t),dummy>
				{
				explicit IteratorsConst(const Data<t>& data) noexcept:
					IteratorsConst<previous(t),dummy>(data),m_position(data.content.begin())
					{}
				typename MapType<KeyPointer,typename TypeGet<t,StorageModel>::type>::const_iterator m_position;
				};

			template<bool dummy>
			struct IteratorsConst<type_first,dummy>
				{
				explicit IteratorsConst(const Data<type_first>& data) noexcept:
					m_position(data.content.begin())
					{}
				typename MapType<KeyPointer,typename TypeGet<type_first,StorageModel>::type>::const_iterator m_position;
				};

			template<Type t>
			static auto& iteratorGet(Iterators<t>& i) noexcept
				{return i.m_position;}

			template<Type t>
			static auto& iteratorGet(IteratorsConst<t>& i) noexcept
				{return i.m_position;}


			struct NodeInfo
				{
				Type type;
				size_t child_count;
				};
				
			MapType<StringType,NodeInfo, std::less<StringType> > m_keys;

			template<class T>
			static ArrayType<T>* get(void* array_pointer) noexcept
				{return reinterpret_cast<ArrayType<T>*>(array_pointer);}

			template<class T,class ExceptionHandler>
			static void array_append(void* array_pointer,const StringType& value,ExceptionHandler& eh)
				{get<T>(array_pointer)->push_back(convert<T>(value,eh));}

			template<class ExceptionHandler>
			struct ArrayPointer
				{
				void* m_object;
				void (*append)(void* object,const StringType& value,ExceptionHandler& eh);
				};	

			template<class ExceptionHandler>
			ArrayPointer<ExceptionHandler> arrayGet(Type type,const Key& key,ExceptionHandler& eh)
				{
				type=arraySet(type);
			//	FIXME increment parent child count
				auto ip=m_keys.insert({key,{type,0}});
				if(!ip.second)
					{
					eh.raise(Error("Key «",key.c_str(),"» already exists."));
					return {nullptr,nullptr};
					}

				ArrayPointer<ExceptionHandler> ret{nullptr,nullptr};
				for_type<StorageModel,Type::I8_ARRAY,2,Type::STRING_ARRAY>(type,[&ret,this,&ip](auto x)
					{
					static constexpr auto type_id=decltype(x)::id;
					static_assert(static_cast<int>(type_id)%2,"Type is not an array");
					typedef typename TypeGet<arrayUnset(type_id),StorageModel>::type raw_type;
					auto callback=this->array_append<raw_type,ExceptionHandler>;
					ret={&( this->dataGet<type_id>()[ip.first->first.c_str()]),callback};
					},eh);
				return ret;
				}

			template<class ExceptionHandler>
			void valueSet(Type type,const Key& key,const StringType& value,ExceptionHandler& eh)
				{
			//	FIXME increment parent child count
				auto ip=m_keys.insert({key,{type,0}});
				if(!ip.second)
					{
					eh.raise(Error("Key «",key.c_str(),"» already exists."));
					return;
					}

				for_type<StorageModel,Type::I8,2,Type::STRING>(type,[this,&ip,&value,&eh](auto x)
					{
					static constexpr auto type_id=decltype(x)::id;
					static_assert((static_cast<int>(type_id)%2)==0,"Type is an array");
					this->dataGet<type_id>()[ip.first->first.c_str()]=convert<typename TypeGet<type_id,StorageModel>::type>(value,eh);
					},eh);
				}

			struct Locale
				{
				Locale():m_handle(newlocale(LC_ALL,"C",0))
					{m_loc_old=uselocale(m_handle);}
				~Locale()
					{
					uselocale(m_loc_old);
					freelocale(m_handle);
					}

				locale_t m_handle;
				locale_t m_loc_old;
				};

			template<bool compound,class dummy=void>
			struct ItemProcessTrampoline
				{
				template<class ItemProcessor,class StringType,class IteratorSet,class ValueTag>
				static inline void go(ItemProcessor& proc,const StringType& key
					,size_t child_count,IteratorSet& iterators,ValueTag x)
					{
					static constexpr auto type_id=decltype(x)::id;
					auto& j=iteratorGet<type_id>(iterators);
					proc(key,child_count,x,j->second);
					++j;
					}
				};

			template<class dummy>
			struct ItemProcessTrampoline<true,dummy>
				{
				template<class ItemProcessor,class StringType,class IteratorSet,class ValueTag>
				static inline void go(ItemProcessor& proc,const StringType& key,size_t child_count,IteratorSet& iterators
					,ValueTag x)
					{proc(key,child_count,x,0);}
				};

			BufferType idCreate(size_t index)
				{
				char buffer[sizeof(index)*2 + 1];
				sprintf(buffer,"%016zx",index);
				return BufferType(buffer);
				}
		};
	}

#define TEMPLE_CONCAT_IMPL(x,y) x##y
#define TEMPLE_MACRO_CONCAT( x, y ) TEMPLE_CONCAT_IMPL( x, y )

#define TEMPLE_MAKE_ID(x) TEMPLE_MACRO_CONCAT(x, __LINE__)

#define TEMPLE_FIND(tree,value,...) \
	[](auto& tree_,auto& value_) \
		{ \
		static constexpr auto TEMPLE_MAKE_ID(path)=Temple::make_path(tree_.pathsep(),__VA_ARGS__); \
		return tree_.find(value_,TEMPLE_MAKE_ID(path).c_str()); \
		}(tree,value)

#define TEMPLE_INSERT_COPY(tree,value,...) \
	[](auto& tree_,const auto& value_)\
		{ \
		static constexpr auto TEMPLE_MAKE_ID(path)=Temple::make_path(tree_.pathsep(),__VA_ARGS__); \
		return tree_.insert(value_,TEMPLE_MAKE_ID(path)); \
		}(tree,value)

#define TEMPLE_INSERT_MOVE(tree,value,...) \
	[](auto& tree_,auto&& value_)\
		{ \
		static constexpr auto TEMPLE_MAKE_ID(path)=Temple::make_path(tree_.pathsep(),__VA_ARGS__); \
		return tree_.insert(value_,TEMPLE_MAKE_ID(path)); \
		}(tree,value)

#define TEMPLE_COMPOUND_INSERT(tree,array,...) \
	[](auto& tree_,auto array_) \
		{ \
		static constexpr auto TEMPLE_MAKE_ID(path)=Temple::make_path(tree_.pathsep(),__VA_ARGS__); \
		return tree_.compoundInsert(TEMPLE_MAKE_ID(path),array_);\
		}(tree,array)

template<class StorageModel>
template<class Source,class ProgressMonitor>
Temple::ItemTree<StorageModel>& Temple::ItemTree<StorageModel>::load(Source& src,ProgressMonitor& monitor)
	{
	enum class State:int
		{
		 KEY,ESCAPE,TYPE,COMPOUND_BEGIN,ARRAY_CHECK
		,ARRAY,VALUE,DELIMITER,KEY_BEGIN,VALUE_STRING
		,ITEM_STRING
		};

	uintmax_t line_count=1;
	uintmax_t col_count=0;
	auto state_current=State::COMPOUND_BEGIN;
	auto state_old=state_current;
	BufferType token_in;
	Locale loc;

	struct Node
		{
		StringType key;
		Type type;
		size_t item_count;
		bool array;
		};
		
	Node node_current{"",Type::COMPOUND,0,0};
	std::stack<Node> nodes;
	ArrayPointer<ProgressMonitor> array_pointer;
	

//	Do not call feof, since that function expects that the caller
//	has tried to read data first. This is not compatible with API:s 
//	that have UB when trying to read at EOF. Using a wrapper solves
//	that problem.
	typename BufferType::value_type ch_in;
	while(read(src,ch_in))
		{
		if(ch_in=='\n')
			{
			++line_count;
			col_count=0;
			}
		++col_count;
		monitor.positionUpdate(line_count,col_count);
		switch(state_current)
			{
			case State::ARRAY_CHECK:
				switch(ch_in)
					{
					case '{':
						monitor.raise(Error("A compound has been opened, but current object has "
							"been declared as '",type(node_current.type),'\''));
						return *this;
					case '[':
						state_current=State::ARRAY;
						array_pointer=arrayGet(node_current.type,node_current.key,monitor);
						break;
					case '"':
						state_current=State::VALUE_STRING;
						break;
					default:
						token_in+=ch_in;
						state_current=State::VALUE;
					}
				break;
			case State::COMPOUND_BEGIN:
				switch(ch_in)
					{
					case '{':
						if(nodes.size())
							{
							if(nodes.top().array)
								{
								node_current.key+=pathsep();
								node_current.key+=idCreate(node_current.item_count);
								}
						//	++nodes.top().item_count;
							}
						node_current.array=0;
						node_current.item_count=0;
						nodes.push(node_current);
					//	FIXME: Increment counter on parent node
						m_keys[node_current.key]={Type::COMPOUND,0};
						state_current=State::KEY_BEGIN;
						break;
					case '[':
						if(nodes.size() && nodes.top().array)
							{
							if(nodes.top().array)
								{
								node_current.key+=pathsep();
								node_current.key+=idCreate(node_current.item_count);
								}
						//	++nodes.top().item_count;
							}
						node_current.array=1;
						nodes.push(node_current);
					//	FIXME: Increment counter on parent node
						m_keys[node_current.key]={Type::COMPOUND_ARRAY,0};
						state_current=State::COMPOUND_BEGIN;
						break;
					case ']':
						monitor.raise(Error("Empty array of compounds is not allowed."));
						return *this;
					default:
						if(!(ch_in>=0 && ch_in<=' ')) //Eat whitespace
							{
							monitor.raise(Error("Array '[' or compound '{' expected."));
							return *this;
							}
					}
				break;
			case State::KEY_BEGIN:
				switch(ch_in)
					{
					case '"':
						state_current=State::KEY;
						break;
					case '}':
						monitor.raise(Error("Empty compound is not allowed."));
						break;
					default:
						if(!(ch_in>=0 && ch_in<=' ')) //Eat whitespace
							{
							monitor.raise(Error("A key must begin with '\"', not '",ch_in,"'."));
							return *this;
							}
					}
				break;

			case State::KEY:
				switch(ch_in)
					{
					case '\\':
						state_old=state_current;
						state_current=State::ESCAPE;
						break;
					case pathsep():
						monitor.raise(Error('\'',pathsep(),"' cannot be used in keys."));
						return *this;
					case '"':
						node_current.key+=pathsep();
						node_current.key+=token_in;
						state_current=State::TYPE;
						token_in.clear();
						break;
					default:
						token_in+=ch_in;
					}
				break;

			case State::TYPE:
				switch(ch_in)
					{
					case '\\':
						state_old=state_current;
						state_current=State::ESCAPE;
						break;
					case ':':
						node_current.type=type(token_in,monitor);
						token_in.clear();
						if(node_current.type==Type::COMPOUND)
							{state_current=State::COMPOUND_BEGIN;}
						else
							{state_current=State::ARRAY_CHECK;}
						break;
					default:
						token_in+=ch_in;
					}
				break;

			case State::ARRAY:
				switch(ch_in)
					{
					case '"':
						state_current=State::ITEM_STRING;
						break;
					case '\\':
						state_old=state_current;
						state_current=State::ESCAPE;
						break;
					case ',':
						array_pointer.append(array_pointer.m_object,token_in,monitor);
						token_in.clear();
						break;
					case ']':
						if(token_in.size()!=0)
							{	
							array_pointer.append(array_pointer.m_object,token_in,monitor);
							token_in.clear();
							}
						state_current=State::DELIMITER;
						break;
					default:
						if(!(ch_in>=0 && ch_in<=' ')) //Eat whitespace
							{token_in+=ch_in;}
					}
				break;

			case State::DELIMITER:
				switch(ch_in)
					{
					case ',':
						node_current=nodes.top();
						++node_current.item_count;
						if(node_current.array)
							{state_current=State::COMPOUND_BEGIN;}
						else
							{state_current=State::KEY_BEGIN;}
						break;
					case '}':
						state_current=State::DELIMITER;
						if(nodes.size()==0)
							{
							monitor.raise(Error("There is no more block to terminate."));
							return *this;
							}
						node_current=nodes.top();
						++node_current.item_count;
						nodes.pop();
						if(node_current.array)
							{monitor.raise(Error("An array must be terminated with ']'."));}
						break;
					case ']':
						state_current=State::DELIMITER;
						if(nodes.size()==0)
							{
							monitor.raise(Error("There is no more block to terminate."));
							return *this;
							}
						node_current=nodes.top();
						++node_current.item_count;
						nodes.pop();
						if(!node_current.array)
							{monitor.raise(Error("A compound must be terminated with '}'."));}
						break;
					default:
						if(!(ch_in>=0 && ch_in<=' ')) //Eat whitespace
							{
							monitor.raise(Error('\'',ch_in," is an invalid delimiter."));
							return *this;
							}
					}
				break;

			case State::VALUE:
				switch(ch_in)
					{
					case '\\':
						state_old=state_current;
						state_current=State::ESCAPE;
						break;
					case '"':
						state_current=State::VALUE_STRING;
						break;
					case ',':
						state_current=State::KEY_BEGIN;
						valueSet(node_current.type,node_current.key,token_in,monitor);
						token_in.clear();
						node_current=nodes.top();
						break;
					case '}':
						state_current=State::DELIMITER;
						valueSet(node_current.type,node_current.key,token_in,monitor);
						token_in.clear();
						node_current=nodes.top();
						nodes.pop();
						break;
					default:
						if(!(ch_in>=0 && ch_in<=' ')) //Eat whitespace
							{token_in+=ch_in;}
					}
				break;

			case State::VALUE_STRING:
				switch(ch_in)
					{
					case '\"':
						state_current=State::VALUE;
						break;
					case '\\':
						state_old=state_current;
						state_current=State::ESCAPE;
						break;
					default:
						token_in+=ch_in;
					}
				break;

			case State::ITEM_STRING:
				switch(ch_in)
					{
					case '\"':
						state_current=State::ARRAY;
						break;
					case '\\':
						state_old=state_current;
						state_current=State::ESCAPE;
						break;
					default:
						token_in+=ch_in;
					}
				break;

			case State::ESCAPE:
				if(state_old==State::KEY && ch_in==pathsep())
					{
					monitor.raise(Error('\'',ch_in," cannot be used in keys."));
					return *this;
					}

				token_in+=ch_in;
				state_current=state_old;
				break;
			}
		}
	if(nodes.size())
		{monitor.raise(Error("Unterminated block at EOF."));}
	return *this;
	}

template<class StorageModel>
template<class ItemProcessor,class ExceptionHandler>
void Temple::ItemTree<StorageModel>::itemsProcess(ItemProcessor&& proc,ExceptionHandler&& eh)
	{
	auto i=m_keys.begin();
	auto i_end=m_keys.end();

	Iterators<> iterators(m_data);

	while(i!=i_end)
		{
		auto& key=i->first;
		auto child_count=i->second.child_count;
		for_type<StorageModel,Type::I8,1,Type::COMPOUND_ARRAY>
		(i->second,[&key,&iterators,this,&proc,child_count](auto x)
			{
			static constexpr auto type_id=decltype(x)::id;
			ItemProcessTrampoline<arrayUnset(type_id)==Type::COMPOUND>
				::go(proc,key,child_count,iterators,x);
			},eh);
		++i;
		}
	}

template<class StorageModel>
template<class ItemProcessor,class ExceptionHandler>
void Temple::ItemTree<StorageModel>::itemsProcess(ItemProcessor&& proc,ExceptionHandler&& eh) const
	{
	auto i=m_keys.begin();
	auto i_end=m_keys.end();

	IteratorsConst<> iterators(m_data);

	while(i!=i_end)
		{
		auto& key=i->first;
		auto child_count=i->second.child_count;
		for_type<StorageModel,Type::I8,1,Type::COMPOUND_ARRAY>
		(i->second.type,[&key,&iterators,this,&proc,child_count](auto x)
			{
			static constexpr auto type_id=decltype(x)::id;
			ItemProcessTrampoline<arrayUnset(type_id)==Type::COMPOUND>
				::go(proc,key,child_count,iterators,x);
			},eh);
		++i;
		}
	}

#endif
