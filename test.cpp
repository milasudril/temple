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

	private:
		uintmax_t m_line;
		uintmax_t m_col;
	};

template<class T>
void print(const T& val)
	{printf("%d",val);}

void print(const std::string& val)
	{printf("\"%s\"",val.c_str());}

void print(double val)
	{printf("%.15g",val);}

void print(float val)
	{printf("%.7g",val);}

void print(int64_t val)
	{printf("%lld",val);}

struct Proc
	{
	template<class T>
	void operator()(const ItemTree<BasicStorage>::Key& key,const T& val)
		{
		printf("\"%s\":",key.c_str());
		print(val);
		putchar('\n');
		}

	template<class T>
	void operator()(const ItemTree<BasicStorage>::Key& key,const std::vector<T>& val)
		{
		printf("\"%s\":[",key.c_str());
		auto ptr=val.data();
		auto ptr_end=ptr + val.size();
		while(ptr!=ptr_end)
			{
			print(*ptr);
			++ptr;
			if(ptr!=ptr_end)
				{putchar(',');}
			}
		puts("]");
		}
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
})EOF";

/*	
	*/
	Monitor m;
	try
		{
		ItemTree<> tree(Reader{src},m);
		tree.itemsProcess(Proc{},m);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%" PRIuMAX ":%" PRIuMAX ": %s\n",m.line(),m.col(),error.message());
		}
	return 0;
	}