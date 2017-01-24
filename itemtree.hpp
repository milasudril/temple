//@	{"targets":[{"name":"itemtree.hpp","type":"include"}]}

#ifndef AJSON_ITEMTREE_HPP
#define AJSON_ITEMTREE_HPP

#include "numberformat.hpp"
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

		template<class Key,class Value>
		using MapType=std::map<Key,Value>;

		using StringType=std::string;

		using BufferType=std::string;
		};

	template<class StorageModel=BasicStorage>
	class ItemTree
		{
		public:
			template<class T>
			using ArrayType=typename StorageModel::template ArrayType<T>;

			template<class Key,class Value>
			using MapType=typename StorageModel::template MapType<Key,Value>;

			using StringType=typename StorageModel::StringType;

			using BufferType=typename StorageModel::BufferType;

			template<class Reader,class ProgressMonitor>
			ItemTree(Reader&& reader,ProgressMonitor&& monitor)
				{load(reader,monitor);}

			template<class Reader,class ProgressMonitor>
			ItemTree& load(Reader&& reader,ProgressMonitor& monitor)
				{return load(reader,monitor);}

			template<class Reader,class ProgressMonitor>
			ItemTree& load(Reader& reader,ProgressMonitor& monitor);

			template<class ItemProcessor,class ExceptionHandler>
			void itemsProcess(ItemProcessor&& proc,ExceptionHandler&& eh)
				{
				auto i=m_keys.begin();
				auto i_end=m_keys.end();
				while(i!=i_end)
					{
					auto& key=i->first;
					for_type<StorageModel,0,1>(i->second,[&key,this,&proc](auto x)
						{
						static constexpr auto type_id=decltype(x)::id;
						proc(key,this->dataGet<type_id>().find(key)->second);
						},eh);
					++i;
					}
				}

			template<Type t>
			auto& dataGet() noexcept
				{return dataGet<t>(m_data);}

			template<Type t>
			const auto& dataGet() const noexcept
				{return dataGet<t>(m_data);}

		private:
			using Key=StringType;

			static constexpr auto type_last=Type::COMPOUND;
			static constexpr auto type_first=Type::I8;
		
			template<Type t=type_last,bool dummy=1>
			struct Data:public Data<previous(t),dummy>
				{MapType<Key,typename TypeGet<previous(t),StorageModel>::type> content;};

			template<bool dummy>
			struct Data<type_first,dummy>
				{};

			Data<> m_data;
			
			template<Type t>
			static auto& dataGet(Data<next(t)>& data) noexcept
				{return data.content;};

			template<Type t>
			static const auto& dataGet(const Data<next(t)>& data) noexcept
				{return data.content;};

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
				for_type<StorageModel,1,2>(type,[&ret,this,&key](auto x)
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

				for_type<StorageModel,0,2>(type,[this,&key,loc,&value,&eh](auto x)
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
		};
	}

template<class StorageModel>
template<class Reader,class ProgressMonitor>
Temple::ItemTree<StorageModel>& Temple::ItemTree<StorageModel>::load(Reader& reader,ProgressMonitor& monitor)
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
	while(!eof(reader))
		{
	//	Likewise, fgetc does not work.
		auto ch_in=codepointGet(reader);
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
							node_current.key+="/#";
							node_current.key+=std::to_string(node_current.item_count);
							++nodes.top().item_count;
							}
						node_current.array=0;
						node_current.item_count=0;
						nodes.push(node_current);
						state_current=State::KEY_BEGIN;
						break;
					case '[':
						node_current.array=1;
						nodes.push(node_current);
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
					case '/':
						monitor.raise(Error("'/' cannot be used in keys."));
						return *this;
					case '#':
						monitor.raise(Error("'#' cannot be used in keys."));
						return *this;
					case '"':
						node_current.key+='/';
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
				if(state_old==State::KEY && (ch_in=='/' || ch_in=='#'))
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

#endif
