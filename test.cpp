//@	{"targets":[{"name":"test","type":"application"}]}

#include "itemtree.hpp"
#include <cstdio>
#include <clocale>
#include <cinttypes>

using namespace Temple;

struct Reader
	{
	const char* r_src;
	};

char codepointGet(Reader& reader)
	{
	auto v=*(reader.r_src);
	++reader.r_src;
	return v;
	}

bool eof(Reader& reader)
	{return *reader.r_src==0;}

class Monitor
	{
	public:
		void raise(const Temple::Error& error)
			{throw error;}

		void positionUpdate(uintmax_t line,uintmax_t col) noexcept
			{
			m_line=line;
			m_col=col;
			}

		auto line() noexcept
			{return m_line;}

		auto col() noexcept
			{return m_col;}

		void reset() noexcept
			{m_line=0;m_col=0;}

	private:
		uintmax_t m_line;
		uintmax_t m_col;
	};

int main()
	{
	setlocale(LC_ALL,"");
/*	const char* src=R"EOF([{
 "\"quotation marks\" in \"key\""i32:1234
,"bar":
	{"baz"i64:124380867045036}
,"foo":
	{
	 "a string"s:"Hello, World"
	,"another valid string"s:This\ is\ legal\ too
	,"bar"i32:[1,2,3,4]
	,"empty array"i32:[]
	,"more objects":
		{
		"foo"s:"bar"
		,"value"d:3.14
		,"value as float"f:3.14
		}
	,"xxx"s:"more stuff"
	,"yyy"s:"more stuff"
	}
,"goo":{"key"i32:12456}
,"compound array":
	[
		 {"a key"s:"A value"}
		,{
		"a key"s:"A value 2"
		,"array":
			[
				{
				 "bar"i32:2
				,"foo"i32:"1"
				}
			]
		 }
		,[{"foo"s:"bar"},{"foo"s:"bar2"}]
	]
,"compound array 2":
	[
		 {"a key"s:"A value"}
		,{"a key"s:"A value 2"}
	]
},{"property"s:"value"}])EOF";*/

	Monitor m;
	try
		{
		ItemTree<> tree(Reader{"{\"values\":[[{\"index\"i32:1},{\"index\"i32:2}],[{\"index1\"i32:3},{\"index1\"i32:4}]]}"},m);
		m.reset();		
		tree.store(stdout,m);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%" PRIuMAX ":%" PRIuMAX ": %s\n",m.line(),m.col(),error.message());
		}
	return 0;
	}