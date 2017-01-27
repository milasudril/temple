//@	{"targets":[{"name":"pathconst.hpp","type":"include"}]}

#ifndef TEMPLE_PATHCONST_HPP
#define TEMPLE_PATHCONST_HPP

namespace Temple
	{
	template<class T,class StrA>
	constexpr auto make_path(T delimiter,const StrA& a)
		{return concat(delimiter,a);}

	template<class T,class StrA,class ... Strings>
	constexpr auto make_path(T delimiter,const StrA& a,const Strings& ... b) noexcept
		{return concat(concat(delimiter,a),make_path(delimiter,b...));}
	}

#define TEMPLE_CONCAT_IMPL(x,y) x##y
#define TEMPLE_MACRO_CONCAT( x, y ) TEMPLE_CONCAT_IMPL( x, y )

#define TEMPLE_MAKE_ID(x) TEMPLE_MACRO_CONCAT(x, __LINE__)

#define TEMPLE_USE_PATH_AS_CSTR(function,delimiter,...) \
	do \
		{ \
		static constexpr auto TEMPLE_MAKE_ID(path)=Temple::make_path(delimiter,__VA_ARGS__); \
		function(TEMPLE_MAKE_ID(path).c_str()); \
		} \
	while(0)

#endif
