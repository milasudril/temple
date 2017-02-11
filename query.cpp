//@	{"targets":[{"name":"temple_query","type":"application"}]}

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

int main(int argc,char** argv)
	{
	setlocale(LC_ALL,"");
	try
		{
		if(argc<2)
			{throw Temple::Error("Path expression missing.");}


		auto eh=[](const Temple::Error& err) [[noreturn]]
			{throw err;};

		Temple::ItemTree<> tree(Reader{stdin},eh);
		if(tree.empty())
			{throw Temple::Error("Item tree is empty.");}

		auto& item=Temple::find(eh,tree.root(),argv + 1);

		if(arrayUnset(item.type())==Temple::Type::COMPOUND)
			{
			Temple::temple_store(item,stdout);
			}
		else
			{
			Temple::for_type<decltype(tree)::StorageModel,Temple::Type::I8,1,Temple::Type::STRING_ARRAY>
			(item.type(),[&item,&tree](auto tag)
				{
				using type=typename decltype(tag)::type;
				Temple::write<decltype(tree)::StorageModel>(item.value<type>(),stdout);
				});
			}
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%s\n",error.message());
		return -1;
		}
	return 0;
	}
