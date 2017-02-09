//@	{"targets":[{"name":"test","type":"application"}]}

#include "itemtree.hpp"
#include "stringconst.hpp"
#include "pathconst.hpp"
#include <cstdio>
#include <clocale>
#include <cinttypes>
#include <algorithm>

using namespace Temple;

class Monitor
	{
	public:
		[[noreturn]] void operator()(const Temple::Error& error)
			{
			auto line=Temple::convert(m_line);
			auto col=Temple::convert(m_col);
			Error e(line.data(),':',col.data(),": ",error.message());
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

std::string& concat(char a,std::string& b)
	{
	b+=a;
	return b;
	}

std::string& concat(std::string& a,const char* b)
	{
	a+=b;
	return a;
	}

std::string& concat(std::string& a,const std::string& b)
	{
	a+=b;
	return a;
	}

struct Reader
	{
	Reader(const char* src)
		{handle=fopen(src,"rb");}
	~Reader()
		{fclose(handle);}

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

int main()
	{
	setlocale(LC_ALL,"");
	try
		{
		ItemTree<> tree(Reader("test.temple"),Monitor{});
		tree.store(stdout);

		auto& x=tree.root().value<ItemTree<>::CompoundArray>()[0]
			.find<long>("\"quotation marks\" in \"key\"",[](auto err){throw err;});

		printf("Found value: %d",x);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%s\n",error.message());
		}
	return 0;
	}
