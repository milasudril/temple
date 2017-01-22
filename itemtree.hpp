#ifndef AJSON_ITEMTREE_HPP
#define AJSON_ITEMTREE_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <cassert>

namespace
	{
	template<class T,class ErrorHandler>
	struct Converter
		{};

	template<class ErrorHandler>
	struct Converter<int8_t,ErrorHandler>
		{
		static int8_t convert(const std::string& value,ErrorHandler& error)
			{
			auto x=std::stoi(value);
			if(x<-128 || x>127)
				{
				error("Value out of range");
				abort();
				}
			return x;
			}
		};

	template<class ErrorHandler>
	struct Converter<int16_t,ErrorHandler>
		{
		static int16_t convert(const std::string& value,ErrorHandler& error)
			{
			auto x=std::stoi(value);
			if(x<-32768 || x>32767)
				{
				error("Value out of range");
				abort();
				}
			return x;
			}
		};

	template<class ErrorHandler>
	struct Converter<int32_t,ErrorHandler>
		{
		static int32_t convert(const std::string& value,ErrorHandler& error)
			{
			auto x=std::stoll(value);
			if(x<-2147483648 || x>2147483647)
				{
				error("Value out of range");
				abort();
				}
			return x;
			}
		};

	template<class ErrorHandler>
	struct Converter<int64_t,ErrorHandler>
		{
		static int64_t convert(const std::string& value,ErrorHandler& error)
			{
			auto x=std::stoll(value);
		//FIXME
			return x;
			}
		};

	template<class ErrorHandler>
	struct Converter<float,ErrorHandler>
		{
		static float convert(const std::string& value,ErrorHandler& error)
			{
		//FIXME
			return std::stof(value);
			}
		};

	template<class ErrorHandler>
	struct Converter<double,ErrorHandler>
		{
		static double convert(const std::string& value,ErrorHandler& error)
			{
		//FIXME
			return std::stod(value);
			}
		};

	template<class ErrorHandler>
	struct Converter<std::string,ErrorHandler>
		{
		static const std::string& convert(const std::string& value,ErrorHandler& error)
			{return value;}
		};
	}

namespace Temple
	{
	struct BasicStorage
		{
		template<class T>
		using ArrayType=std::vector<T>;
		};

	template<class StorageModel=BasicStorage>
	class ItemTree
		{
		public:
			typedef std::string Key;
			template<class T>
			using ArrayType=typename StorageModel::template ArrayType<T>;

			template<class Reader,class ErrorHandler>
			ItemTree(Reader&& reader,ErrorHandler&& error)
				{load(reader,error);}

			template<class Reader,class ErrorHandler>
			ItemTree& load(Reader&& reader,ErrorHandler& error)
				{return load(reader,error);}

			template<class Reader,class ErrorHandler>
			ItemTree& load(Reader& reader,ErrorHandler& error);

			template<class ItemProcessor>
			void itemsProcess(ItemProcessor&& proc)
				{
				data_int8.process(proc);
				data_int16.process(proc);
				data_int32.process(proc);
				data_int64.process(proc);
				data_float.process(proc);
				data_double.process(proc);
				data_string.process(proc);
				}

		private:
			template<class T>
			using value_map=std::map<Key,T>;

			template<class T>
			using array_map=std::map<Key, ArrayType<T> >;

			template<class T>
			struct Data
				{
				value_map<T> values;
				array_map<T> arrays;
				
				template<class ItemProcessor>
				void process(ItemProcessor& proc)
					{
						{
						auto i=values.begin();
						auto i_end=values.end();
						while(i!=i_end)
							{
							proc(i->first,i->second);
							++i;
							}
						}

						{
						auto i=arrays.begin();
						auto i_end=arrays.end();
						while(i!=i_end)
							{
							proc(i->first,i->second);
							++i;
							}
						}
					}
				};
		
			Data<int8_t> data_int8;
			Data<int16_t> data_int16;
			Data<int32_t> data_int32;
			Data<int64_t> data_int64;
			Data<float> data_float;
			Data<double> data_double;
			Data<std::string> data_string;

			enum class Type:int{COMPOUND,STRING,I8,I16,I32,I64,FLOAT,DOUBLE};

			template<class ErrorHandler>
			static Type type(const std::string& str,ErrorHandler& error)
				{
				if(str=="")
					{return Type::COMPOUND;}
				if(str=="s")
					{return Type::STRING;}
				if(str=="i8")
					{return Type::I8;}
				if(str=="i16")
					{return Type::I16;}
				if(str=="i32")
					{return Type::I32;}
				if(str=="i64")
					{return Type::I64;}
				if(str=="f")
					{return Type::FLOAT;}
				if(str=="d")
					{return Type::DOUBLE;}
				error("Unknown type",str.c_str());
				return Type::COMPOUND;
				}

			template<class T>
			static ArrayType<T>* get(void* array_pointer) noexcept
				{return reinterpret_cast<ArrayType<T>*>(array_pointer);}

			template<class T,class ErrorHandler>
			static void array_append(void* array_pointer,const std::string& value
				,ErrorHandler& error)
				{get<T>(array_pointer)->push_back(Converter<T,ErrorHandler>::convert(value,error));}

			template<class ErrorHandler>
			struct ArrayPointer
				{
				void* m_object;
				void (*append)(void* object,const std::string& value,ErrorHandler& error);
				};

			template<class ErrorHandler>
			ArrayPointer<ErrorHandler> arrayGet(Type type,const std::string& key,ErrorHandler& error)
				{
				switch(type)
					{
					case Type::I8:
						return {&data_int8.arrays[key],array_append<int8_t,ErrorHandler>};
					case Type::I16:
						return {&data_int16.arrays[key],array_append<int16_t,ErrorHandler>};
					case Type::I32:
						return {&data_int32.arrays[key],array_append<int32_t,ErrorHandler>};
					case Type::I64:
						return {&data_int64.arrays[key],array_append<int64_t,ErrorHandler>};
					case Type::FLOAT:
						return {&data_float.arrays[key],array_append<float,ErrorHandler>};
					case Type::DOUBLE:
						return {&data_double.arrays[key],array_append<double,ErrorHandler>};
					case Type::STRING:
						return {&data_string.arrays[key],array_append<std::string,ErrorHandler>};
					case Type::COMPOUND:
						error("Invalid value type");
						return {nullptr,nullptr};
					}
				error("Invalid value type");
				return {nullptr,nullptr};
				}

			template<class ErrorHandler>
			void valueSet(Type type,const Key& key,const std::string& value,ErrorHandler& error)
				{
				switch(type)
					{
					case Type::I8:
						data_int8.values[key]=Converter<int8_t,ErrorHandler>::convert(value,error);
						break;
					case Type::I16:
						data_int16.values[key]=Converter<int16_t,ErrorHandler>::convert(value,error);
						break;
					case Type::I32:
						data_int32.values[key]=Converter<int32_t,ErrorHandler>::convert(value,error);
						break;
					case Type::I64:
						data_int64.values[key]=Converter<int64_t,ErrorHandler>::convert(value,error);
						break;
					case Type::FLOAT:
						data_float.values[key]=Converter<float,ErrorHandler>::convert(value,error);
						break;
					case Type::DOUBLE:
						data_double.values[key]=Converter<double,ErrorHandler>::convert(value,error);
						break;
					case Type::STRING:
						data_string.values[key]=Converter<std::string,ErrorHandler>::convert(value,error);
						break;
					case Type::COMPOUND:
						error("Invalid value type");
					}
				}
		};
	}

template<class StorageModel>
template<class Reader,class ErrorHandler>
Temple::ItemTree<StorageModel>& Temple::ItemTree<StorageModel>::load(Reader& reader,ErrorHandler& error)
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


	auto state_current=State::COMPOUND_BEGIN;
	auto state_old=state_current;
	std::string token_in;

	struct Node
		{
		std::string key;
		Type type;
		size_t item_count;
		bool array;
		};
		
	Node node_current{"",Type::COMPOUND,0,0};
	std::stack<Node> nodes;
	ArrayPointer<ErrorHandler> array_pointer;

//	Do not call feof, since that function expects that the caller
//	has tried to read data first. This is not compatible with a 
//	C-style string
	while(!eof(reader))
		{
	//	Likewise, fgetc does not work.
		auto ch_in=codepointGet(reader);

		switch(state_current)
			{
			case State::ARRAY_CHECK:
				switch(ch_in)
					{
					case '{':
						error("Unexpected compound");
						return *this;
					case '[':
						state_current=State::ARRAY;
						array_pointer=arrayGet(node_current.type,node_current.key,error);
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
					default:
						if(ch_in<0 || ch_in>' ')
							{
							error("Expected array or compound");
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
					default:
						if(ch_in<0 || ch_in>' ')
							{
							error("Expected '\"'");
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
						error("'/' cannot be used in keys");
						return *this;
					case '#':
						error("'#' cannot be used in keys");
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
						node_current.type=type(token_in,error);
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
						array_pointer.append(array_pointer.m_object,token_in,error);
						token_in.clear();
						break;
					case ']':
						array_pointer.append(array_pointer.m_object,token_in,error);
						token_in.clear();
						state_current=State::DELIMITER;
						break;
					default:
						if(ch_in<0 || ch_in>' ')
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
						node_current=nodes.top();
						nodes.pop();
						if(node_current.array)
							{error("Expected ']' terminator");}
						break;
					case ']':
						state_current=State::DELIMITER;
						node_current=nodes.top();
						nodes.pop();
						if(!node_current.array)
							{error("Expected '}' terminator");}
						break;
					default:
						if(ch_in<0 || ch_in>' ')
							{
							error("Invalid character");
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
						valueSet(node_current.type,node_current.key,token_in,error);
						token_in.clear();
						node_current=nodes.top();
						break;
					case '}':
						state_current=State::DELIMITER;
						valueSet(node_current.type,node_current.key,token_in,error);
						token_in.clear();
						node_current=nodes.top();
						nodes.pop();
						break;
					default:
						if(ch_in<0 || ch_in>' ')
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
					error("'/' or '#' cannot be used in keys");
					return *this;
					}

				token_in+=ch_in;
				state_current=state_old;
				break;

			default:
				error("Bad state");
				return *this;
			}
		}
	if(nodes.size())
		{error("Unterminated block");}
	return *this;
	}

#endif
