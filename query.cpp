//@	{"targets":[{"name":"temple_query","type":"application"}]}

#include "itemtree.hpp"
#include <cstdio>
#include <clocale>
#include <cinttypes>
#include <algorithm>

class Monitor
	{
	public:
		[[noreturn]] void operator()(const Temple::Error& error)
			{
			auto line=Temple::convert(m_line);
			auto col=Temple::convert(m_col);
			Temple::Error e(line.data(),':',col.data(),": ",error.message());
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

int main(int argc,char** argv)
	{
	try
		{
		if(argc<2)
			{throw Temple::Error("Path expression missing.");}

		Temple::ItemTree<> tree(Reader{stdin},Monitor{});
		if(tree.empty())
			{throw Temple::Error("Item tree is empty.");}

		auto eh=[](const Temple::Error& err)
			{throw err;};

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
