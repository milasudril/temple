//@	{"targets":[{"name":"temple_type_mismatch","type":"application"}]}

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

struct StringTemp
	{
	const char* buffer;
	const char* c_str() const noexcept
		{return buffer;}
	};

int main(int argc,char** argv)
	{
	try
		{
		Temple::ItemTree<> tree(Reader{stdin},Monitor{});
		auto eh=[](const Temple::Error& err)
			{throw err;};

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
