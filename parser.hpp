//@	{"targets":[{"name":"parser.hpp","type":"include"}]}

#ifndef TEMPLE_PARSER_HPP
#define TEMPLE_PARSER_HPP

#include "item.hpp"
#include "converters.hpp"
#include "treenode.hpp"
#include <stack>

namespace Temple
	{
	namespace
		{
		template<class StorageModel,class BufferType,class ExceptionHandler>
		using AppendFunc=void (*)(ItemBase<StorageModel>& item,const BufferType& buffer,ExceptionHandler& eh);

		template<class Source,class ExceptionHandler>
		class ProgressMonitor
			{
			public:
				explicit ProgressMonitor(Source& source,ExceptionHandler& eh) noexcept:
					r_source(source),r_eh(eh)
					{}

				[[noreturn]] void operator()(const Error& error)
					{
					auto line=convert(m_line);
					auto col=convert(m_col);
					raise(Error(name(r_source),':',line.data(),':',col.data(),": ",error.message()),r_eh);
					}

				void positionUpdate(uintmax_t line,uintmax_t col) noexcept
					{
					m_line=line;
					m_col=col;
					}

			private:
				uintmax_t m_line;
				uintmax_t m_col;
				Source& r_source;
				ExceptionHandler& r_eh;
			};
		}

	template<class StackType,class ExceptionHandler>
	auto pop(StackType& stack,ExceptionHandler& eh)
		{
		if(stack.empty())
			{raise(Error("There are no more blocks to end."),eh);}
		auto ret=stack.top();
		stack.pop();
		return ret;
		}

//	Array append functions
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

	template<class StorageModel,class BufferType,class Source,class ExceptionHandler>
	std::unique_ptr<ItemBase<StorageModel>> temple_load(Source& src,ExceptionHandler& eh)
		{
		using MapType=typename StorageModel::template MapType< std::unique_ptr< ItemBase<StorageModel> > > ;
		using CompoundArray=typename StorageModel::template ArrayType<MapType>;
		ProgressMonitor<Source,ExceptionHandler> monitor(src,eh);

		enum class State:int
			{
			 KEY,COMMENT,ESCAPE,KEY_STRING,TYPE,COMPOUND,VALUE_ARRAY_CHECK
			,VALUE_ARRAY,VALUE,VALUE_STRING,ITEM_STRING,INIT
			};

		uintmax_t line_count=1;
		uintmax_t col_count=0;
		auto state_current=State::INIT;
		auto state_old=state_current;
		
		Locale loc; //Fix number conversion
		
		auto type_current=Type::COMPOUND;

		BufferType key_current;
		
		std::unique_ptr<ItemBase<StorageModel>> item_current;
		AppendFunc<StorageModel,BufferType,decltype(monitor)> append_func;

		TreeNode<CompoundArray,MapType> node_current;
		std::unique_ptr<ItemBase<StorageModel>> root;
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
						case '{':
							if(token_in.size()==0)
								{raise(Error("Unterminated block, or '{' must be escaped when used as first character in a key."),monitor);}
							token_in.push_back(ch_in);
							break;
						case '\\':
							state_old=state_current;
							state_current=State::ESCAPE;
							break;
						case '"':
							state_current=State::KEY_STRING;
							break;
						case ',':
							if(token_in.size()==0)
								{raise(Error("Empty keys are not allowed."),monitor);}
							key_current=token_in;
							token_in.clear();
							state_current=State::TYPE;
							break;
						case ':':
							if(token_in.size()==0)
								{raise(Error("Empty keys are not allowed."),monitor);}
							key_current=token_in;
							type_current=Type::COMPOUND;
							token_in.clear();
							state_current=State::COMPOUND;
							break;
						case '}':
							if(token_in.size()!=0)
								{
								node_current.insert(key_current
									,itemCreate<StorageModel>(type_current,token_in,monitor)
									,monitor);
								token_in.clear();
								}
							node_current=pop(nodes,monitor);
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
							append_func=appendFunction<StorageModel,BufferType,decltype(monitor)>(type_current);
							state_current=State::VALUE_ARRAY;
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
							token_in.clear();
							node_current=pop(nodes,monitor);
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
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
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
							token_in.clear();
							state_current=State::KEY;
							break;
						case '}':
							node_current.insert(key_current
								,itemCreate<StorageModel>(type_current,token_in,monitor)
								,monitor);
							token_in.clear();
							node_current=pop(nodes,monitor);
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
							token_in.clear();
							state_current=State::VALUE_ARRAY;
							break;
						case ']':
							if(token_in.size()!=0)
								{
								append_func(*item_current.get(),token_in,monitor);
								token_in.clear();
								}
							node_current.insert(key_current,item_current,monitor);
							state_current=State::COMPOUND;
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
									,Item<MapType,StorageModel>::create(),monitor)
										.template value<MapType>() );
								}
							state_current=State::KEY;
							break;
						case '[':
							nodes.push(node_current);
							if(node_current.array())
								{raise(Error("An array cannot contain another array."),monitor);}
							else
								{
								node_current=decltype(node_current)(node_current.insert(key_current
									,Item<CompoundArray,StorageModel>::create(),monitor)
										.template value<CompoundArray>() );
								}
							break;
						case '}':
							if(node_current.array())
								{raise(Error("An array of compounds must be terminated with ']'."),monitor);}
							node_current=pop(nodes,monitor);
							break;
						case ']':
							if(!node_current.array())
								{raise(Error("A compound must be terminated with '}'"),monitor);}
							node_current=pop(nodes,monitor);
							break;
						case ',':
							if(!node_current.array())
								{state_current=State::KEY;}
							break;

						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{raise(Error("Compound: illegal character '",ch_in,"'."),monitor);}
						}
					break;

				case State::INIT:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case '{':
							nodes.push(node_current); //For symmetry
							root=Item<MapType,StorageModel>::create();
							node_current=decltype(node_current)( root->template value<MapType>() );
							state_current=State::KEY;
							break;
						case '[':
							nodes.push(node_current); //For symmetry
							root=Item<CompoundArray,StorageModel>::create();
							node_current=decltype(node_current)(root->template value<CompoundArray>() );
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
		assert(node_current.pointer()==nullptr);
		return root;
		}
	}

#endif
