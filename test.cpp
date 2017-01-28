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

void test(std::vector<int32_t>& v)
	{
	}

int main()
	{
	setlocale(LC_ALL,"");
	const char* src=R"EOF([{
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
},{"property"s:"value"}])EOF";

	Monitor m;
	try
		{
		ItemTree<> tree(Reader{src},m);
		m.reset();

		Temple::ItemTree<>::StringType* ret;
		assert(TEMPLE_FIND(tree,ret,   "0000000000000001","property"));
		assert(!TEMPLE_INSERT_MOVE(tree,1234,"root","foo","bar"));
		assert(TEMPLE_INSERT_MOVE(tree,1234,"0000000000000001","test"));
		int* x;
		assert(TEMPLE_FIND(tree,x,"0000000000000001","test"));
		assert(*x==1234);
		*x=123456;

		Temple::ItemTree<>::StringType new_string("Hello, World");
		assert(TEMPLE_INSERT_COPY(tree,new_string,"0000000000000001","another property"));
		assert(TEMPLE_COMPOUND_INSERT(tree,0,"0000000000000001","a compound"));
		assert(TEMPLE_INSERT_MOVE(tree,42,"0000000000000001","a compound","the answer to the question about universe life and everything"));

		const auto& ctree=tree;
		const int* cx;
		assert(TEMPLE_FIND(ctree,cx,"0000000000000001","test"));
		ctree.store(stdout,m);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%" PRIuMAX ":%" PRIuMAX ": %s\n",m.line(),m.col(),error.message());
		}
	return 0;
	}