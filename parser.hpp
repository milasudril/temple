#ifndef TEMPLE_PARSER_HPP
#define TEMPLE_PARSER_HPP

#include "item.hpp"
#include "converters.hpp"
#include <clocale>
#include <stack>

namespace Temple
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

	template<class StorageModel,class Source,class ProgressMonitor,class BufferType>
	ItemBase<StorageModel>* temple_load(Source& src,ProgressMonitor& monitor)
		{
		using KeyType=typename StorageModel::KeyType;

		enum class State:int
			{
			 KEY,COMMENT,ESCAPE,KEY_STRING,TYPE,COMPOUND_BEGIN,ARRAY_CHECK
			,ARRAY,KEY_NEXT,VALUE,DELIMITER,VALUE_STRING,ITEM_STRING
			};

		uintmax_t line_count=1;
		uintmax_t col_count=0;
		auto state_current=State::COMPOUND_BEGIN;
		auto state_old=state_current;
		BufferType token_in;
		Locale loc;

		struct Node
			{
			KeyType key;
			Type type;
			size_t item_count;
			bool array;
			};
			
		Node node_current{"",Type::I8,0,0};
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
						case pathsep():
							monitor.raise(Error('\'',pathsep(),"' cannot be used in keys."));
							return *this;
						case ',':
							nodes.push(node_current);
							node_current.key.push_back(pathsep());
							node_current.key.append(token_in);
							node_current.item_count=0;
							token_in.clear();
							state_current=State::TYPE;
							break;
						case ':':
							nodes.push(node_current);
							node_current.key.push_back(pathsep());
							node_current.key.append(token_in);
							node_current.item_count=0;
							token_in.clear();
							state_current=State::COMPOUND_BEGIN;
							break;
						case '}':
							if(node_current.item_count)
								{
								node_current=nodes.top();
								nodes.pop();
								++node_current.item_count;
								m_keys[node_current.key].child_count=node_current.item_count;
								}
							state_current=State::DELIMITER;
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
					if(state_old==State::KEY && ch_in==pathsep())
						{
						monitor.raise(Error('\'',ch_in," cannot be used in keys."));
						return *this;
						}

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
						case pathsep():
							monitor.raise(Error('\'',pathsep(),"' cannot be used in keys."));
							return *this;
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
							node_current.type=type(token_in,monitor);
							token_in.clear();
							state_current=State::ARRAY_CHECK;
							break;
						default:
							token_in.push_back(ch_in);
						}
					break;
					

				case State::ARRAY_CHECK:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
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
						case ',':
							valueSet(node_current.type,node_current.key,token_in,monitor);
							token_in.clear();
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							state_current=State::KEY;
							break;
						case '}':
							valueSet(node_current.type,node_current.key,token_in,monitor);
							token_in.clear();
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							m_keys[node_current.key].child_count=node_current.item_count;
							state_current=State::DELIMITER;
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
							valueSet(node_current.type,node_current.key,token_in,monitor);
							token_in.clear();
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							state_current=State::KEY;
							break;
						case '}':
							valueSet(node_current.type,node_current.key,token_in,monitor);
							token_in.clear();
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							m_keys[node_current.key].child_count=node_current.item_count;
							state_current=State::DELIMITER;
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

				case State::ARRAY:
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
							array_pointer.append(array_pointer.m_object,token_in,monitor);
							token_in.clear();
							break;
						case ']':
							if(token_in.size()!=0)
								{	
								array_pointer.append(array_pointer.m_object,token_in,monitor);
								token_in.clear();
								}
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							state_current=State::KEY_NEXT;
							break;
						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{token_in.push_back(ch_in);}
						}
					break;
				case State::KEY_NEXT:
					switch(ch_in)
						{
						case ',':
							state_current=State::KEY;
							break;
						default:
							if(!(ch_in>=0 && ch_in<=' ')) //Eat whitespace
								{
								monitor.raise(Error("Expected ',' after value array."));
								return *this;
								}
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
							token_in.push_back(ch_in);
						}
					break;

				case State::COMPOUND_BEGIN:
					switch(ch_in)
						{
						case '#':
							state_old=state_current;
							state_current=State::COMMENT;
							break;
						case '{':
							if(!m_keys.insert({node_current.key,{Type::COMPOUND,0}}).second)
								{
								monitor.raise(Error("Key «",node_current.key.c_str(),"» already exists."));
								return *this;
								}
							state_current=State::KEY;
							break;
						case '[':
							if(!m_keys.insert({node_current.key,{Type::COMPOUND_ARRAY,0}}).second)
								{
								monitor.raise(Error("Key «",node_current.key.c_str(),"» already exists."));
								return *this;
								}
							node_current.array=1;
							nodes.push(node_current);
							node_current.key.push_back( pathsep() );
							node_current.key.append( idCreate(node_current.item_count) );
							node_current.item_count=0;
							node_current.array=0;
							state_current=State::COMPOUND_BEGIN;
							break;
						case ']':
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							state_current=State::DELIMITER;
							break;
						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{
								monitor.raise(Error("Array '[' or compound '{' expected."));
								return *this;
								}
						}
					break;

				case State::DELIMITER:
					switch(ch_in)
						{
						case ',':
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							if(node_current.array)
								{
								nodes.push(node_current);
								node_current.key.push_back( pathsep() );
								node_current.key.append( idCreate(node_current.item_count) );
								node_current.item_count=0;
								node_current.array=0;
								state_current=State::COMPOUND_BEGIN;
								}
							else
								{state_current=State::KEY;}
							break;
						case '}':
							if(nodes.size()==0)
								{
								monitor.raise(Error("There is no more block to terminate."));
								return *this;
								}
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							m_keys[node_current.key].child_count=node_current.item_count;
							if(node_current.array)
								{monitor.raise(Error("An array must be terminated with ']'."));}
							break;
						case ']':
							if(nodes.size()==0)
								{
								monitor.raise(Error("There is no more block to terminate."));
								return *this;
								}
							node_current=nodes.top();
							nodes.pop();
							++node_current.item_count;
							m_keys[node_current.key].child_count=node_current.item_count;
							if(!node_current.array)
								{monitor.raise(Error("A compound must be terminated with '}'."));}
							break;
						default:
							if(!(ch_in>=0 && ch_in<=' '))
								{
								monitor.raise(Error('\'',ch_in," is an invalid delimiter."));
								return *this;
								}
						}
					break;
				}
			}
		if(nodes.size()!=0)
			{monitor.raise(Error("Unterminated block at EOF."));}
		return *this;
		}
	}

#endif
