//@	{"targets":[{"name":"test2","type":"application"}]}

#include "stringconst.hpp"
#include "pathconst.hpp"
#include <cstdio>

int main()
	{
	TEMPLE_USE_PATH_AS_CSTR(puts,'/',"home","alice","Documents","foo","bar");
	return 0;
	}
