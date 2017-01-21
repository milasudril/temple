//@	{"targets":[{"name":"test","type":"application"}]}

#include "itemtree.hpp"
#include <cstdio>

using namespace TeMpLe;

struct Reader
	{
	const char* r_src;
	};

int getc(Reader& reader)
	{
	auto v=*(reader.r_src);
	++reader.r_src;
	return v;
	}

int eof(Reader& reader)
	{return 0;}

struct ErrorHandler
	{
	void operator()(const char* message)
		{
		fprintf(stderr,"Error: %s",message);
		abort();
		}

	void operator()(const char* message,const char* comment)
		{
		fprintf(stderr,"Error: %s (%s)",message,comment);
		abort();
		}
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
	const char* src=R"EOF({
"foo":
	{
	 "bar"i32:[1,2,3]
	,"a string"s:"Hello, World"
	,"another valid string"s:This\ is\ legal\ too
	,"more objects":{"foo"s:"bar","value"d:3.14}
	}
,"bar":{"baz"i64:124380867045036}
,"compound array":[{"a key"s:"A value"},{"a key"s:"A value 2","array":[{"foo"i32:1,"bar"i32:2}]}]
,"compound array 2":[{"a key"s:"A value"},{"a key"s:"A value 2"}]
})EOF";

	ItemTree<> tree(Reader{src},ErrorHandler{});
	tree.itemsProcess(Proc{});

	return 0;
	}