//@	{"targets":[{"name":"parser.hpp","type":"include"}]}

#ifndef TEMPLE_PARSER_HPP
#define TEMPLE_PARSER_HPP

#include "item.hpp"
#include "converters.hpp"
#include <clocale>
#include <stack>

namespace Temple
	{
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
#if 0
	//SFINAE for detecting a map
		struct has_not_member{};

		struct has_member:public has_not_member{};

		template<class X> struct member_test
			{typedef int type;};

		template<class T,typename member_test<typename T::mapped_type>::type=0>
		static constexpr bool test_mapped_type(has_member)
			{return 1;}

		template<class T>
		static constexpr bool test_mapped_type(has_not_member)
			{return 0;}

		template <class T>
		struct IsMap
			{static constexpr bool value=test_mapped_type<T>(has_member{});};

		template<class T,std::enable_if_t<IsMap<T>::value,int> x=0>
		void doStuff()
			{
			printf("This is a map\n");
			}

		template<class T,std::enable_if_t<!IsMap<T>::value,int> x=0>
		void doStuff()
			{
			printf("This is not a map\n");
			}


		template<class T,class StorageModel,class Node,class ExceptionHandler
			,std::enable_if_t< std::is_same<T,typename StorageModel::MapType> > x=0>
		void append(ItemBase<StorageModel>& item,Node&& node,ExceptionHandler& eh)
			{
			if(!item.template value<T>().insert({std::move(node.key), std::move(node.item)}).second)
				{eh.raise(Error("Double key"));}
			}

		template<class T,class StorageModel,class Node,class ExceptionHandler
			,std::enable_if_t< std::is_same<T,typename StorageModel::ArrayType> > x=0>
		void append(ItemBase<StorageModel>& item,Node&& node,ExceptionHandler& eh)
			{item.template value<T>().push_back(convert<T>(node.value));}
#endif

	//	Array append functions

		template<class T,class StorageModel,class BufferType,class ExceptionHandler>
		void append(ItemBase<StorageModel>& item,const BufferType& buffer,ExceptionHandler& eh)
			{
			using value_type=typename TypeGet<arrayUnset(IdGet<T,StorageModel>::id),StorageModel>::type;
			item.template value<T>().push_back(convert<value_type>(buffer,eh));
			}

		template<class StorageModel,class BufferType,class ExceptionHandler>
		using AppendFunc=void (*)(ItemBase<StorageModel>& item,const BufferType& buffer,ExceptionHandler& eh);

		template<class StorageModel,class BufferType,class ExceptionHandler>
		auto appendFunction(Type type)
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
		void itemInsert(MapType& map,const BufferType& key,ItemType&& item,ExceptionHandler& eh)
			{
			if(!map.emplace(typename MapType::key_type(key),std::move(item)).second)
				{eh.raise(Error("Key ",key.c_str()," already exists in the current node."));}
			}
		}


	template<class StorageModel,class BufferType,class Source,class ProgressMonitor>
	ItemBase<StorageModel>* temple_load(Source& src,ProgressMonitor& monitor)
		{
		using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;

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

		struct Node
			{
			BufferType key;
			ItemBase<StorageModel>* item;
			AppendFunc<StorageModel,BufferType,ProgressMonitor> append_func;
			};
			
		auto type_current=Type::COMPOUND;

		BufferType key_current;
		
		std::unique_ptr<ItemBase<StorageModel>> item_current;
		AppendFunc<StorageModel,BufferType,ProgressMonitor> append_func;
		

		Node node_current{BufferType(""),nullptr};
		std::stack<Node> nodes;

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
							itemInsert(node_current.item->template value<MapType>()
								,key_current
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
							itemInsert(node_current.item->template value<MapType>()
								,key_current
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
							itemInsert(node_current.item->template value<MapType>()
								,key_current
								,itemCreate<StorageModel>(type_current,token_in,monitor)
								,monitor);
							state_current=State::KEY;
							break;
						case '}':
							itemInsert(node_current.item->template value<MapType>()
								,key_current
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
							itemInsert(node_current.item->template value<MapType>()
								,key_current,item_current,monitor);
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
							state_current=State::KEY;
							break;
						case '[':
							type_current=arraySet(type_current);
							state_current=State::COMPOUND;
							break;
						case '}':
						//Error if array
						//POP
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
								{
								monitor.raise(Error("Illegal character '",ch_in,"'."));
								return nullptr; //FIXME
								}
						}
					break;
				}
			}
		if(nodes.size()!=0)
			{monitor.raise(Error("Unterminated block at EOF."));}
		return node_current.item;
		}
	}

#endif
