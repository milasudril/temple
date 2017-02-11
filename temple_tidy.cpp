//@	{"targets":[{"name":"temple_tidy","type":"application"}]}

#include "itemtree.hpp"
#include <cstdio>
#include <clocale>


struct Reader
	{
	FILE* handle;
	};

bool read(Reader& reader,char& ch)
	{
	auto ch_in=getc(reader.handle);
	if(ch_in==-1)
		{return 0;}
	ch=ch_in;
	return 1;
	}

const char* name(Reader& reader) noexcept
	{return "stdin";}

int main()
	{
	setlocale(LC_ALL,"");
	try
		{
		using namespace Temple;
		ItemTree<> tree(Reader{stdin},[](const Temple::Error& err)[[noreturn]]
			{throw err;});
		if(tree.empty())
			{throw Temple::Error("Item tree is empty.");}
		tree.store(stdout);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%s\n",error.message());
		return -1;
		}
	return 0;
	}
