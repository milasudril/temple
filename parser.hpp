//@	{"targets":[{"name":"parser.hpp","type":"include"}]}

#ifndef TEMPLE_PARSER_HPP
#define TEMPLE_PARSER_HPP

#include "item.hpp"
#include "converters.hpp"
#include <clocale>
#include <stack>
#include <type_traits>

namespace Temple
	{
	namespace
		{
		template<class StorageModel,class BufferType,class ExceptionHandler>
		using AppendFunc=void (*)(ItemBase<StorageModel>& item,const BufferType& buffer,ExceptionHandler& eh);
		}

	template<class ExceptionHandler>
	[[noreturn]] static void raise(const Error& msg,ExceptionHandler& eh)
		{
		eh.raise(msg);
		assert(0 && "Exception handler must not return to its caller.");
		}


//	Array append functions
	template<class ArrayType>
	static typename ArrayType::value_type& append(ArrayType& array)
		{
		array.emplace_back( typename ArrayType::value_type{} );
		return array.back();
		}

	template<class T,class StorageModel,class BufferType,class ExceptionHandler>
	static void append(ItemBase<StorageModel>& item,const BufferType& buffer,ExceptionHandler& eh)
		{
		using value_type=typename TypeGet<arrayUnset(IdGet<T,StorageModel>::id),StorageModel>::type;
		item.template value<T>().push_back(convert<value_type>(buffer,eh));
		}

	template<class StorageModel,class BufferType,class ExceptionHandler>
	static auto appendFunction(Type type)
		{
		AppendFunc<StorageModel,BufferType,ExceptionHandler> ret;
		for_type<StorageModel,Type::I8_ARRAY,2,Type::STRING_ARRAY>(type,[&ret](auto tag)
			{
			using T=typename decltype(tag)::type;
			ret=append<T,StorageModel,BufferType,ExceptionHandler>;
			});
		return ret;
		}

	//	Item insertion (into map)
	template<class MapType,class BufferType,class ItemType,class ExceptionHandler>
	static auto& insert(MapType& map,const BufferType& key,ItemType&& item,ExceptionHandler& eh)
		{
		auto ret=item.get();
		if(!map.emplace(typename MapType::key_type(key),std::move(item)).second)
			{raise(Error("Key ",key.c_str()," already exists in the current node."),eh);}
		return *ret;
		}

	namespace
		{
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

		template<class ArrayType,class MapType>
		class Node
			{
			public:
				Node():m_container(nullptr),m_array(0){}

				template<class T>
				explicit Node(T& container):m_container(&container)
					,m_array(std::is_same<std::remove_reference_t<T>,ArrayType>::value)
					{}

				template<class BufferType,class ItemType,class ExceptionHandler>
				auto& insert(const BufferType& key,ItemType&& item,ExceptionHandler& eh)
					{
					assert(m_container);
					assert(!m_array);
					return Temple::insert(*(reinterpret_cast<MapType*>(m_container))
						,key,std::move(item),eh);
					}

				auto& append()
					{
					assert(m_container);
					assert(m_array);
					return Temple::append(*(reinterpret_cast<ArrayType*>(m_container)));
					}

				bool array() const noexcept
					{return m_array;}

				template<class T>
				T& container() noexcept
					{return *reinterpret_cast<T*>(m_container);}

			
			private:
				void* m_container;
				bool m_array;
			};
		}

	template<class StorageModel,class BufferType,class Source,class ProgressMonitor>
	ItemBase<StorageModel>* temple_load(Source& src,ProgressMonitor& monitor)
		{
		using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;

		using CompoundArray=typename StorageModel::template ArrayType<MapType>;

		enum class State:int
			{
			 KEY,COMMENT,ESCAPE,KEY_STRING,TYPE,COMPOUND,VALUE_ARRAY_CHECK
			,VALUE_ARRAY,VALUE,VALUE_STRING,ITEM_STRING
			};

		uintmax_t line_count=1;
		uintmax_t col_count=0;
		auto state_current=State::COMPOUND;
		auto state_old=state_current;
		
		Locale loc;

		
		auto type_current=Type::COMPOUND;

		BufferType key_current;
		
		std::unique_ptr<ItemBase<StorageModel>> item_current;
		AppendFunc<StorageModel,BufferType,ProgressMonitor> append_func;

		Node<CompoundArray,MapType> node_current;
		std::stack<decltype(node_current)> nodes;

	//	Do not call feof, since that function expects that the caller
	//	has tried to read data first. This is not compatible with API:s 
	//	that have UB when trying to read at EOF. Using a wrapper solves
	//	that problem.
		typename BufferType::value_type ch_in;
		BufferType token_in;
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
				case State::KEY:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case '\\':
							state_old=state_current;
							state_current=State::ESCAPE;
							break;
						case '"':
							state_current=State::KEY_STRING;
							break;
						case ',':
							key_current=token_in;
							token_in.clear();
							state_current=State::TYPE;
							break;
						case ':':
							key_current=token_in;
							type_current=Type::COMPOUND;
							token_in.clear();
							state_current=State::COMPOUND;
							break;
						case '}':
							node_current.insert(key_current
								,itemCreate<StorageModel>(type_current,token_in,monitor)
								,monitor);
						//POP
							state_current=State::COMPOUND;
							break;

						default:
							if(! (ch_in>=0 && ch_in<=' ') )
								{token_in.push_back(ch_in);}
						}
					break;

				case State::COMMENT:
					switch(ch_in)
						{
						case '\n':
							state_current=state_old;
							break;
						default:
							break;
						}
					break;

				case State::ESCAPE:
					token_in+=ch_in;
					state_current=state_old;
					break;

				case State::KEY_STRING:
					switch(ch_in)
						{
						case '\\':
							state_old=state_current;
							state_current=State::ESCAPE;
							break;
						case '"':
							state_current=State::KEY;
							break;
						default:
							token_in.push_back(ch_in);
						}
					break;

				case State::TYPE:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case ':':
							type_current=type(token_in,monitor);
							token_in.clear();
							state_current=type_current==Type::COMPOUND?
								State::COMPOUND : State::VALUE_ARRAY_CHECK;
							break;
						default:
							token_in.push_back(ch_in);
						}
					break;
					

				case State::VALUE_ARRAY_CHECK:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case '[':
							type_current=arraySet(type_current);
							item_current=itemCreate<StorageModel>(type_current);
							append_func=appendFunction<StorageModel,BufferType,ProgressMonitor>(type_current);
							state_current=State::VALUE_ARRAY;
							break;
						case '"':
							state_current=State::VALUE_STRING;
							break;
						case ',':
							state_current=State::KEY;
							break;
						case '}':
							node_current.insert(key_current
								,itemCreate<StorageModel>(type_current,token_in,monitor)
								,monitor);
						//POP
							state_current=State::COMPOUND;
							break;
						default:
							if(!(ch_in>=0 && ch_in<=' ') )
								{
								token_in.push_back(ch_in);
								state_current=State::VALUE;
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
							node_current.insert(key_current
								,itemCreate<StorageModel>(type_current,token_in,monitor)
								,monitor);
							state_current=State::KEY;
							break;
						case '}':
							node_current.insert(key_current
								,itemCreate<StorageModel>(type_current,token_in,monitor)
								,monitor);
						//POP
							state_current=State::COMPOUND;
							break;
						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{token_in.push_back(ch_in);}
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
							token_in.push_back(ch_in);
						}
					break;

				case State::VALUE_ARRAY:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case '"':
							state_current=State::ITEM_STRING;
							break;
						case '\\':
							state_old=state_current;
							state_current=State::ESCAPE;
							break;
						case ',':
							append_func(*item_current.get(),token_in,monitor);
							state_current=State::VALUE_ARRAY;
							break;
						case ']':
							append_func(*item_current.get(),token_in,monitor);
							node_current.insert(key_current,item_current,monitor);
							state_current=State::KEY;
							break;

						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{token_in.push_back(ch_in);}
						}
					break;

				case State::ITEM_STRING:
					switch(ch_in)
						{
						case '\"':
							state_current=State::VALUE_ARRAY;
							break;
						case '\\':
							state_old=state_current;
							state_current=State::ESCAPE;
							break;
						default:
							token_in.push_back(ch_in);
						}
					break;

				case State::COMPOUND:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case '{':
							nodes.push(node_current);
							if(node_current.array())
								{node_current=decltype(node_current)(node_current.append());}
							else
								{
								node_current=decltype(node_current)(node_current.insert(key_current
									,Item<MapType,StorageModel>::create(),monitor));
								}
							state_current=State::KEY;
							break;
						case '[':
							nodes.push(node_current);
							if(node_current.array())
								{raise(Error("An array cannot contain another array"),monitor);}
							else
								{
								node_current=decltype(node_current)(node_current.insert(key_current
									,Item<CompoundArray,StorageModel>::create(),monitor));
								}
							state_current=State::COMPOUND;
							break;
						case '}':
							if(node_current.array())
								{raise(Error("An array of compounds must be terminated with ']'"),monitor);}
							node_current=nodes.top();
							nodes.pop();
							state_current=State::COMPOUND;
							break;
						case ']':
						//Error if not array
						//POP
							state_current=State::COMPOUND;
							break;
						case ',':
						//KEY or COMPOUND (depending on array)
							state_current=State::COMPOUND;
							break;

						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{raise(Error("Illegal character '",ch_in,"'."),monitor);}
						}
					break;
				}
			}
		if(nodes.size()!=0)
			{raise(Error("Unterminated block at EOF."),monitor);}
		return nullptr; //FIXME
		}
	}

#endif
