//@	{"targets":[{"name":"itemtree.hpp","type":"include"}]}

#ifndef AJSON_ITEMTREE_HPP
#define AJSON_ITEMTREE_HPP

#include "converters.hpp"
#include "type.hpp"

#include <map>
#include <vector>
#include <stack>
#include <cassert>
#include <cstddef>

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

			static constexpr typename StringType::value_type pathsep() noexcept
				{return '\001';}

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

			template<class Sink,class ExceptionHandler>
			void store(Sink& sink,ExceptionHandler& eh)
				{
				StringType path_prev;
				itemsProcess([this,&sink,&path_prev,&eh](const auto& key,auto tag,const auto& value)
					{
					auto path_end=this->rfind(key,pathsep());
					auto level=this->count(key,pathsep());
					RecordWrite<decltype(tag)::id>::doIt(sink,level,path_end,value,eh);
				//	RecordWrite<decltype(tag)::id>::write(path_end,value,sink);
				//	if(path_end!=nullptr)
				//		{
				//		fprintf(sink,"[%s] %zu\n",key.c_str(),level);
				//		}
					},eh);
				}

			template<Type t>
			auto& dataGet() noexcept
				{return dataGet<t>(m_data);}

			template<Type t>
			const auto& dataGet() const noexcept
				{return dataGet<t>(m_data);}

		private:
			template<class T>
			static constexpr bool equals_or(T x)
				{return 0;}

			template<class T,class ... U>
			static constexpr bool equals_or(T x,T y,U ... values)
				{return x==y || equals_or(x,values...);}

			template<class Cstr,class Stream,class... to_escape>
			static void write(Cstr src,Stream& stream,to_escape ... esc)
				{
				while(true)
					{
					auto ch_in=*src;
					if(ch_in=='\0')
						{return;}
					if(equals_or(ch_in,esc...))
						{putc('\\',stream);}
					putc(ch_in,stream);
					++src;
					}
				}

			template<Type t,class dummy=void>
			struct RecordWrite
				{
				template<class Sink,class KeyCstr,class Value,class ExceptionHandler>
				static void doIt(Sink& sink,size_t level,KeyCstr key,const Value& value,ExceptionHandler eh)
					{
					assert(key!=nullptr);
					assert(level!=0);
					for(size_t k=0;k<level-1;++k)
						{putc('\t',sink);}
					putc('"',sink);
					write(key,sink,'"');
					fprintf(sink,"\"%s:\n",type(t));
				//	write(value,sink);
					}
				};

			template<class dummy>
			struct RecordWrite<Type::COMPOUND,dummy>
				{
				template<class Sink,class KeyCstr,class Value,class ExceptionHandler>
				static void doIt(Sink& sink,size_t level,KeyCstr key,const Value& value,ExceptionHandler eh)
					{
					if(key!=nullptr)
						{						
						assert(level!=0);
						for(size_t k=0;k<level-1;++k)
							{putc('\t',sink);}
						putc('"',sink);
						write(key,sink,'"');
						fputs("\":\n",sink);
						}
					}
				};

			template<class dummy>
			struct RecordWrite<Type::COMPOUND_ARRAY,dummy>
				{
				template<class Sink,class KeyCstr,class Value,class ExceptionHandler>
				static void doIt(Sink& sink,size_t level,KeyCstr key,const Value& value,ExceptionHandler eh)
					{
					if(key!=nullptr)
						{						
						assert(level!=0);
						for(size_t k=0;k<level-1;++k)
							{putc('\t',sink);}
						putc('"',sink);
						write(key,sink,'"');
						fputs("\":\n",sink);
						}
					}
				};



			using Key=StringType;

			template<class KeyType,class Value>
			using MapType=std::map<KeyType,Value>;

			static const typename StringType::value_type* rfind(const StringType& str
				,typename StringType::value_type ch) noexcept
				{
				auto begin=str.begin();
				auto end=str.end();
				while(end!=begin)
					{
					--end;
					if(*end==ch)
						{return &(*end);}
					}
				return nullptr;
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
				{MapType<Key,typename TypeGet<t,StorageModel>::type> content;};

			template<bool dummy>
			struct Data<type_first,dummy>
				{MapType<Key,typename TypeGet<type_first,StorageModel>::type> content;};

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
				typename MapType<Key,typename TypeGet<t,StorageModel>::type>::iterator m_position;
				};

			template<bool dummy>
			struct Iterators<type_first,dummy>
				{
				explicit Iterators(Data<type_first>& data) noexcept:
					m_position(data.content.begin())
					{}
				typename MapType<Key,typename TypeGet<type_first,StorageModel>::type>::iterator m_position;
				};

			template<Type t>
			static auto& iteratorGet(Iterators<t>& i) noexcept
				{return i.m_position;}



			MapType<StringType,Type> m_keys;

			template<class T>
			static ArrayType<T>* get(void* array_pointer) noexcept
				{return reinterpret_cast<ArrayType<T>*>(array_pointer);}

			template<class T,class ExceptionHandler>
			static void array_append(void* array_pointer,const StringType& value
				,locale_t loc,ExceptionHandler& eh)
				{get<T>(array_pointer)->push_back(convert<T>(value,loc,eh));}

			template<class ExceptionHandler>
			struct ArrayPointer
				{
				void* m_object;
				void (*append)(void* object,const StringType& value,locale_t loc,ExceptionHandler& eh);
				};	

			template<class ExceptionHandler>
			ArrayPointer<ExceptionHandler> arrayGet(Type type,const Key& key,ExceptionHandler& eh)
				{
				type=arraySet(type);
				auto ip=m_keys.insert({key,type});
				if(!ip.second)
					{
					eh.raise(Error("Key «",key.c_str(),"» already exists."));
					return {nullptr,nullptr};
					}

				ArrayPointer<ExceptionHandler> ret{nullptr,nullptr};
				for_type<StorageModel,Type::I8_ARRAY,2,Type::STRING_ARRAY>(type,[&ret,this,&key](auto x)
					{
					static constexpr auto type_id=decltype(x)::id;
					static_assert(static_cast<int>(type_id)%2,"Type is not an array");
					typedef typename TypeGet<arrayUnset(type_id),StorageModel>::type raw_type;
					auto callback=this->array_append<raw_type,ExceptionHandler>;
					ret={&( this->dataGet<type_id>()[key]),callback};
					},eh);
				return ret;
				}

			template<class ExceptionHandler>
			void valueSet(Type type,const Key& key,const StringType& value,locale_t loc,ExceptionHandler& eh)
				{
				auto ip=m_keys.insert({key,type});
				if(!ip.second)
					{
					eh.raise(Error("Key «",key.c_str(),"» already exists."));
					return;
					}

				for_type<StorageModel,Type::I8,2,Type::STRING>(type,[this,&key,loc,&value,&eh](auto x)
					{
					static constexpr auto type_id=decltype(x)::id;
					static_assert((static_cast<int>(type_id)%2)==0,"Type is an array");
					this->dataGet<type_id>()[key]=convert<typename TypeGet<type_id,StorageModel>::type>(value,loc,eh);
					},eh);
				}

			struct Locale
				{
				Locale():m_handle(newlocale(LC_ALL,"C",0))
					{}
				~Locale()
					{freelocale(m_handle);}

				locale_t m_handle;
				};

			template<bool compound,class dummy=void>
			struct ItemProcessTrampoline
				{
				template<class ItemProcessor,class StringType,class IteratorSet,class ValueTag>
				static inline void go(ItemProcessor& proc,const StringType& key,IteratorSet& iterators
					,ValueTag x)
					{
					static constexpr auto type_id=decltype(x)::id;
					auto& j=iteratorGet<type_id>(iterators);
					proc(key,x,j->second);
					++j;
					}
				};

			template<class dummy>
			struct ItemProcessTrampoline<true,dummy>
				{
				template<class ItemProcessor,class StringType,class IteratorSet,class ValueTag>
				static inline void go(ItemProcessor& proc,const StringType& key,IteratorSet& iterators
					,ValueTag x)
					{proc(key,x,0);}
				};
		};
	}

template<class StorageModel>
template<class Source,class ProgressMonitor>
Temple::ItemTree<StorageModel>& Temple::ItemTree<StorageModel>::load(Source& src,ProgressMonitor& monitor)
	{
/* Syntax example:
{
"foo":
{
 "bar"i32:[1,2,3]
,"string"s:"Hello, World"
}
}
*/
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
//	has tried to read data first. This is not compatible with a 
//	C-style string
	while(!eof(src))
		{
	//	Likewise, fgetc does not work.
		auto ch_in=codepointGet(src);
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
						if(nodes.size() && nodes.top().array)
							{
							node_current.key+=pathsep();
							node_current.key+=std::to_string(node_current.item_count);
							++nodes.top().item_count;
							}
						node_current.array=0;
						node_current.item_count=0;
						nodes.push(node_current);
						m_keys[node_current.key]=Type::COMPOUND;
						state_current=State::KEY_BEGIN;
						break;
					case '[':
						node_current.array=1;
						nodes.push(node_current);
						m_keys[node_current.key]=Type::COMPOUND_ARRAY;
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
						array_pointer.append(array_pointer.m_object,token_in,loc.m_handle,monitor);
						token_in.clear();
						break;
					case ']':
						if(token_in.size()!=0)
							{	
							array_pointer.append(array_pointer.m_object,token_in,loc.m_handle,monitor);
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
						valueSet(node_current.type,node_current.key,token_in,loc.m_handle,monitor);
						token_in.clear();
						node_current=nodes.top();
						break;
					case '}':
						state_current=State::DELIMITER;
						valueSet(node_current.type,node_current.key,token_in,loc.m_handle,monitor);
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
		for_type<StorageModel,Type::I8,1,Type::COMPOUND_ARRAY>
		(i->second,[&key,&iterators,this,&proc](auto x)
			{
			static constexpr auto type_id=decltype(x)::id;
			ItemProcessTrampoline<arrayUnset(type_id)==Type::COMPOUND>
				::go(proc,key,iterators,x);
			},eh);
		++i;
		}
	}

#endif
