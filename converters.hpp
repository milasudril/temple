//@	{
//@	"targets":[{"name":"converters.hpp","type":"include"}]
//@	}

#ifndef TEMPLE_CONVERTERS_HPP
#define TEMPLE_CONVERTERS_HPP

#include "error.hpp"
#include <cstdlib>
#include <cerrno>
#include <locale.h>
#include <cmath>
#include <limits>
#include <cinttypes>
#include <cstdio>
#include <type_traits>

namespace Temple
	{

//Helpers for numeric types

	template<class StringType,class ExceptionHandler>
	long strtol(const StringType& str,ExceptionHandler& eh)
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

	template<class StringType,class ExceptionHandler>
	long long strtoll(const StringType& str,ExceptionHandler& eh)
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

	template<class StringType,class ExceptionHandler>
	float strtof(const StringType& str,locale_t loc,ExceptionHandler& eh)
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

	template<class StringType,class ExceptionHandler>
	double strtod(const StringType& str,locale_t loc,ExceptionHandler& eh)
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


//	String to number converters

	template<class T>
	struct Converter
		{
		template<class StringType,class ExceptionHandler>
		static const StringType& convert(const StringType& value
			,locale_t loc,ExceptionHandler& eh)
			{return value;}
		};

	template<>
	struct Converter<int8_t>
		{
		template<class StringType,class ExceptionHandler>
		static int8_t convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
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

	template<>
	struct Converter<int16_t>
		{
		template<class StringType,class ExceptionHandler>
		static int16_t convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
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

	template<>
	struct Converter<int32_t>
		{
		template<class StringType,class ExceptionHandler>
		static int32_t convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
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

	template<>
	struct Converter<int64_t>
		{
		template<class StringType,class ExceptionHandler>
		static int64_t convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
			{return strtoll(value,eh);}
		};

	template<>
	struct Converter<float>
		{
		template<class StringType,class ExceptionHandler>
		static float convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
			{return strtof(value,loc,eh);}
		};

	template<>
	struct Converter<double>
		{
		template<class StringType,class ExceptionHandler>
		static double convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
			{return strtod(value,loc,eh);}
		};


//	Wrapper function

	template<class T,class StringType,class ExceptionHandler>
	T convert(const StringType& value,locale_t loc,ExceptionHandler& eh)
		{return Converter<T>::convert(value,loc,eh);}


//	Helper for counting digits
	template<class T>
	static constexpr size_t digits() noexcept
		{return static_cast<size_t>( std::log10( std::numeric_limits<T>::max() ) ) + 1;}


//	Number to string conversion

	template<class StringType>
	const StringType& convert(const StringType& str)
		{return str;}

	template<class StringType,class Integer>
	std::enable_if_t<std::is_integral<Integer>::value,StringType>
	convert(Integer i)
		{
		char buffer[digits<Integer>() + 2]; //Sign + nul character
		sprintf(buffer,"%" PRIdMAX ,static_cast<intmax_t>(i));
		return StringType(buffer);
		}

	template<class StringType>
	StringType convert(float x)
		{
		char buffer[16];
		sprintf(buffer,"%.9g",x);
		return StringType(buffer);
		}

	template<class StringType>
	StringType convert(double x)
		{
		char buffer[24];
		sprintf(buffer,"%.17g",x);
		return StringType(buffer);
		}
	}

#endif
