//@	{"targets":[{"name":"temple_type_mismatch","type":"application"}]}

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

struct StringTemp
	{
	const char* buffer;
	const char* c_str() const noexcept
		{return buffer;}
	};

int main(int argc,char** argv)
	{
	setlocale(LC_ALL,"");
	try
		{
		auto eh=[](const Temple::Error& err) [[noreturn]]
			{throw err;};

		Temple::ItemTree<> tree(Reader{stdin},eh);
		auto& x=Temple::find_typed< decltype(tree)::ArrayType<int> >(eh,tree.root()
			,0,"foo");

		Temple::write<decltype(tree)::StorageModel>(x,stdout);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%s\n",error.message());
		return -1;
		}
	return 0;
	}
