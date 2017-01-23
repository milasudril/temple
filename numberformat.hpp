//@	{
//@	"targets":[{"name":"numberformat.hpp","type":"include"}]
//@	}

#ifndef TEMPLE_NUMBERFORMAT_HPP
#define TEMPLE_NUMBERFORMAT_HPP

#include "error.hpp"
#include <string>
#include <cstdlib>
#include <cerrno>
#include <locale.h>

namespace Temple
	{
	template<class T,class ExceptionHandler>
	struct Converter
		{};

	template<class ExceptionHandler>
	long strtol(const std::string& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtol(str.c_str(),&endptr,0);
		if(errno==ERANGE)
			{eh.raise(Temple::Error("Value ",str.c_str()," out of range."));}

		if(*endptr!='\0' || endptr==str.c_str())
			{eh.raise(Temple::Error("«",str.c_str(),"» is not a valid integer."));}

		return x;
		}

	template<class ExceptionHandler>
	long long strtoll(const std::string& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtoll(str.c_str(),&endptr,0);
		if(errno==ERANGE)
			{eh.raise(Temple::Error("Value ",str.c_str()," out of range."));}

		if(*endptr!='\0' || endptr==str.c_str())
			{eh.raise(Temple::Error("«",str.c_str(),"» is not a valid integer."));}

		return x;
		}

	template<class ExceptionHandler>
	float strtof(const std::string& str,locale_t loc,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtof_l(str.c_str(),&endptr,loc);
		if(errno==ERANGE)
			{eh.raise(Temple::Error("Value ",str.c_str()," out of range."));}

		if(*endptr!='\0' || endptr==str.c_str())
			{eh.raise(Temple::Error("«",str.c_str(),"» is not a valid floating point number."));}

		return x;
		}

	template<class ExceptionHandler>
	double strtod(const std::string& str,locale_t loc,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtod_l(str.c_str(),&endptr,loc);
		if(errno==ERANGE)
			{eh.raise(Temple::Error("Value ",str.c_str()," out of range."));}

		if(*endptr!='\0' || endptr==str.c_str())
			{eh.raise(Temple::Error("«",str.c_str(),"» is not a valid floating point number."));}

		return x;
		}


	template<class ExceptionHandler>
	struct Converter<int8_t,ExceptionHandler>
		{
		static int8_t convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
			{
			auto x=strtol(value,eh);
			if(x<-128 || x>127)
				{
				eh.raise(Temple::Error("Value ",value.c_str()," out of range."));
				abort();
				}
			return x;
			}
		};

	template<class ExceptionHandler>
	struct Converter<int16_t,ExceptionHandler>
		{
		static int16_t convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
			{
			auto x=strtol(value,eh);
			if(x<-32768 || x>32767)
				{
				eh.raise(Temple::Error("Value ",value.c_str()," out of range."));
				abort();
				}
			return x;
			}
		};

	template<class ExceptionHandler>
	struct Converter<int32_t,ExceptionHandler>
		{
		static int32_t convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
			{
			auto x=strtoll(value,eh);
			if(x<-2147483648 || x>2147483647)
				{
				eh.raise(Temple::Error("Value ",value.c_str()," out of range."));
				abort();
				}
			return x;
			}
		};

	template<class ExceptionHandler>
	struct Converter<int64_t,ExceptionHandler>
		{
		static int64_t convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
			{return strtoll(value,eh);}
		};

	template<class ExceptionHandler>
	struct Converter<float,ExceptionHandler>
		{
		static float convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
			{return strtof(value,loc,eh);}
		};

	template<class ExceptionHandler>
	struct Converter<double,ExceptionHandler>
		{
		static double convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
			{return strtod(value,loc,eh);}
		};

	template<class ExceptionHandler>
	struct Converter<std::string,ExceptionHandler>
		{
		static const std::string& convert(const std::string& value
			,locale_t loc,ExceptionHandler& eh)
			{return value;}
		};

	template<class T,class ExceptionHandler>
	T convert(const std::string& value,locale_t loc,ExceptionHandler& eh)
		{return Converter<T,ExceptionHandler>::convert(value,loc,eh);}
	}

#endif
