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
	const char* src=R"EOF({
"foo":
	{
	 "bar"i32:[1,2,3,4]
	,"a string"s:"Hello, World"
	,"another valid string"s:This\ is\ legal\ too
	,"more objects":{"foo"s:"bar","value"d:3.14}
	,"empty array"i32:[]
	}
,"bar":{"baz"i64:124380867045036}
,"compound array":[{"a key"s:"A value"},{"a key"s:"A value 2","array":[{"foo"i32:"1","bar"i32:2}]}]
,"compound array 2":[{"a key"s:"A value"},{"a key"s:"A value 2"}]
,"\"quotation marks\" in \"key\""i32:1234
})EOF";


	Monitor m;
	try
		{
		ItemTree<> tree(Reader{src},m);
		m.reset();		
		tree.store(stdout,m);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%" PRIuMAX ":%" PRIuMAX ": %s\n",m.line(),m.col(),error.message());
		}
	return 0;
	}