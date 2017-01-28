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

#endif
