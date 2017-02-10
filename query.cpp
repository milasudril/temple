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
		if(argc<2)
			{throw Temple::Error("Path expression missing");}

		++argv;
		Temple::ItemTree<> tree(Reader{stdin},Monitor{});
		auto eh=[](const Temple::Error& err)
			{throw err;};
		auto* root=&tree.root();
		while(*argv!=nullptr)
			{
			switch(root->type())
				{
				case Temple::Type::COMPOUND_ARRAY:
					{
					auto index=Temple::convert<size_t>(StringTemp{*argv},eh);
					auto& vals=root->value<decltype(tree)::CompoundArray>();
					if(index>=vals.size())
						{throw Temple::Error("Array index «",*argv,"» out of bounds");}
					if(*(argv+1)==nullptr)
						{throw Temple::Error("An array index must be followed by a key");}
					++argv;
					root=&vals[index].find(*argv,eh);
					}
					break;
				case Temple::Type::COMPOUND:
					{
					auto& vals=root->value<decltype(tree)::Compound>();
					root=&vals.find(*argv,eh);
					}
					break;
				default:
					throw Temple::Error("«",*argv,"» is not a compound");
				}

			++argv;
			}
		if(arrayUnset(root->type())==Temple::Type::COMPOUND)
			{
			Temple::temple_store(*root,stdout);
			}
		else
			{
			Temple::for_type<decltype(tree)::StorageModel,Temple::Type::I8,1,Temple::Type::STRING_ARRAY>
			(root->type(),[root,&tree](auto tag)
				{
				using type=typename decltype(tag)::type;
				Temple::write<decltype(tree)::StorageModel>(root->value<type>(),stdout);
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
