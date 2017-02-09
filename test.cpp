//@	{"targets":[{"name":"test","type":"application"}]}

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
		using namespace Temple;
		ItemTree<> tree(Reader("test.temple"),Monitor{});
		tree.store(stdout);

		auto& x=tree.root().value<ItemTree<>::CompoundArray>()[0]
			.find_typed<ItemTree<>::Compound>("foo",[](auto err){throw err;})
			.find_typed<ItemTree<>::Compound>("more objects",[](auto err){throw err;})
			.find_typed<double>("value",[](auto err){throw err;});

		printf("Found value: %.7g\n",x);

		auto& y=find_typed<double>([](auto err){throw err;}
			,tree.root(),0,"foo","more objects","value");

		assert(x==y);

		const auto& tree_const=tree;

		auto& xc=tree_const.root().value<ItemTree<>::CompoundArray>()[0]
			.find_typed<ItemTree<>::Compound>("foo",[](auto err){throw err;})
			.find_typed<ItemTree<>::Compound>("more objects",[](auto err){throw err;})
			.find_typed<double>("value",[](auto err){throw err;});
		printf("Found value: %.7g\n",xc);

		auto& yc=find_typed<double>([](auto err){throw err;}
			,tree_const.root(),0,"foo","more objects","value");

		assert(yc==xc);
		}
	catch(const Temple::Error& error)
		{
		fprintf(stderr,"%s\n",error.message());
		}
	return 0;
	}
