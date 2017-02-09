//@	{"targets":[{"name":"test","type":"application"}]}

#include "itemtree.hpp"
#include "stringconst.hpp"
#include "pathconst.hpp"
#include <cstdio>
#include <clocale>
#include <cinttypes>
#include <algorithm>

using namespace Temple;


struct Reader
	{
	const char* r_src;
	};

inline bool read(Reader& reader,char& ch_in) noexcept 
	{
	ch_in=*reader.r_src;
	if(ch_in=='\0')
		{return 0;}
	++reader.r_src;
	return 1;
	}

class Monitor
	{
	public:
		[[noreturn]] void raise(const Temple::Error& error)
			{
			auto line=Temple::convert(m_line);
			auto col=Temple::convert(m_col);
			Error e(line.data(),':',col.data(),": ",error.message());
			throw e;
			}

		void positionUpdate(uintmax_t line,uintmax_t col) noexcept
			{
			m_line=line;
			m_col=col;
			}

	private:
		uintmax_t m_line;
		uintmax_t m_col;
	};

std::string& concat(char a,std::string& b)
	{
	b+=a;
	return b;
	}

std::string& concat(std::string& a,const char* b)
	{
	a+=b;
	return a;
	}

std::string& concat(std::string& a,const std::string& b)
	{
	a+=b;
	return a;
	}

int main()
	{
	setlocale(LC_ALL,"");

	const char* src=R"EOF([{
 "\"quotation marks\" in \"key\"",i32:1234
,bar:
	{baz,i64:124380867045036}
,foo:
	{
	 "a string",s:"Hello, World"
	,"another valid string",s:This\ is\ legal\ too
	,bar,i32:[1,2,3,4]
	,"empty array",i32:[]
	,"more objects":
		{
		 foo,s:"bar"
		,value,f64:3.14
		,"value as float",f32:3.14
		}
	,xxx,s:"more stuff"
	,yyy,s:"more stuff"
	}
,"goo":{key,i32:12456}
,"compound array":
	[
		 {"a key",s:"A value"}
		,{
		"a key",s:"A value 2"
		,"array":
			[
				{
				 bar,i32:2
				,foo,i32:"1"
				}
			]
		 }
		,{foo,s:"bar"},{foo,s:"bar2"}
	]
,"compound array 2":
	[
		 {"a key",s:"A value"}
		,{"a key",s:"A value 2"}
	]
},{property,s:,"empty compound":{},"empty array":[]}])EOF";

	try
		{
		ItemTree<> tree(Reader{src},Monitor{});
		tree.store(stdout);
		write<BasicStorage>(tree.root().value<ItemTree<>::CompoundArray>()
			[0].find(ItemTree<>::KeyType("foo"))->second
			->value<ItemTree<>::Compound>()
			.find(ItemTree<>::KeyType("bar"))->second
			->value<ItemTree<>::ArrayType<int>>(),stdout);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%s\n",error.message());
		}
	return 0;
	}
