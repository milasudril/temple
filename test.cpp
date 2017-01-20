//@	{"targets":[{"name":"test","type":"application"}]}

#include "itemtree.hpp"
#include <cstdio>

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
		{fprintf(stderr,"Error: %s (%s)",message,comment);}
	};

int main()
	{
	const char* src=R"EOF({
"foo":
	{
	 "bar"i32:[1,2,3]
	,"string"s:"Hello, World"
	}
})EOF";

	AJSONLight::ItemTree tree(Reader{src},ErrorHandler{});

	return 0;
	}