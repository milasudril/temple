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
	struct Locale
		{
		Locale():m_handle(newlocale(LC_ALL,"C",0))
			{m_loc_old=uselocale(m_handle);}
		~Locale()
			{
			uselocale(m_loc_old);
			freelocale(m_handle);
			}

		locale_t m_handle;
		locale_t m_loc_old;
		};

	template<size_t size>
	struct Integer
		{};

	template<>
	struct Integer<1>
		{using type=int8_t;};

	template<>
	struct Integer<2>
		{using type=int16_t;};

	template<>
	struct Integer<4>
		{using type=int32_t;};

	template<>
	struct Integer<8>
		{using type=int64_t;};
		


//Helpers for numeric types

	template<class StringType,class ExceptionHandler>
	long strtol(const StringType& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtol(str.c_str(),&endptr,0);
		if(errno==ERANGE)
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		if(*endptr!='\0' || endptr==str.c_str())
			{raise(Temple::Error("«",str.c_str(),"» is not a valid integer."),eh);}

		return x;
		}

	template<class StringType,class ExceptionHandler>
	long long strtoll(const StringType& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtoll(str.c_str(),&endptr,0);
		if(errno==ERANGE)
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		if(*endptr!='\0' || endptr==str.c_str())
			{raise(Temple::Error("«",str.c_str(),"» is not a valid integer."),eh);}

		return x;
		}

	template<class StringType,class ExceptionHandler>
	unsigned long strtoul(const StringType& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		
		if(*str.c_str()=='-')
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}
		errno=0;
		auto x=::strtoul(str.c_str(),&endptr,0);
		if(errno==ERANGE)
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		if(*endptr!='\0' || endptr==str.c_str())
			{raise(Temple::Error("«",str.c_str(),"» is not a valid integer."),eh);}

		return x;
		}

	template<class StringType,class ExceptionHandler>
	unsigned long long strtoull(const StringType& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		if(*str.c_str()=='-')
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		errno=0;
		auto x=::strtoull(str.c_str(),&endptr,0);
		if(errno==ERANGE)
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		if(*endptr!='\0' || endptr==str.c_str())
			{raise(Temple::Error("«",str.c_str(),"» is not a valid integer."),eh);}

		return x;
		}

	template<class StringType,class ExceptionHandler>
	float strtof(const StringType& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtof(str.c_str(),&endptr);
		if(errno==ERANGE)
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		if(*endptr!='\0' || endptr==str.c_str())
			{raise(Temple::Error("«",str.c_str(),"» is not a valid floating point number."),eh);}

		return x;
		}

	template<class StringType,class ExceptionHandler>
	double strtod(const StringType& str,ExceptionHandler& eh)
		{
		char* endptr=nullptr;
		errno=0;
		auto x=::strtod(str.c_str(),&endptr);
		if(errno==ERANGE)
			{raise(Temple::Error("Value ",str.c_str()," out of range."),eh);}

		if(*endptr!='\0' || endptr==str.c_str())
			{raise(Temple::Error("«",str.c_str(),"» is not a valid floating point number."),eh);}

		return x;
		}


//	String to number converters

	template<class T>
	struct Converter
		{
		template<class StringType,class ExceptionHandler>
		static const StringType& convert(const StringType& value,ExceptionHandler& eh)
			{return value;}
		};

	template<>
	struct Converter<int8_t>
		{
		template<class StringType,class ExceptionHandler>
		static int8_t convert(const StringType& value,ExceptionHandler& eh)
			{
			auto x=strtol(value,eh);
			if(x<-128 || x>127)
				{raise(Temple::Error("Value ",value.c_str()," out of range."),eh);}
			return x;
			}
		};

	template<>
	struct Converter<int16_t>
		{
		template<class StringType,class ExceptionHandler>
		static int16_t convert(const StringType& value,ExceptionHandler& eh)
			{
			auto x=strtol(value,eh);
			if(x<-32768 || x>32767)
				{raise(Temple::Error("Value ",value.c_str()," out of range."),eh);}
			return x;
			}
		};

	template<>
	struct Converter<int32_t>
		{
		template<class StringType,class ExceptionHandler>
		static int32_t convert(const StringType& value,ExceptionHandler& eh)
			{
			auto x=strtoll(value,eh);
			if(x<-2147483648 || x>2147483647)
				{raise(Temple::Error("Value ",value.c_str()," out of range."),eh);}
			return x;
			}
		};

	template<>
	struct Converter<int64_t>
		{
		template<class StringType,class ExceptionHandler>
		static int64_t convert(const StringType& value,ExceptionHandler& eh)
			{return strtoll(value,eh);}
		};



	template<>
	struct Converter<uint8_t>
		{
		template<class StringType,class ExceptionHandler>
		static uint8_t convert(const StringType& value,ExceptionHandler& eh)
			{
			auto x=strtol(value,eh);
			if(x>0xff)
				{raise(Temple::Error("Value ",value.c_str()," out of range."),eh);}
			return x;
			}
		};

	template<>
	struct Converter<uint16_t>
		{
		template<class StringType,class ExceptionHandler>
		static uint16_t convert(const StringType& value,ExceptionHandler& eh)
			{
			auto x=strtoul(value,eh);
			if(x>0xffff)
				{raise(Temple::Error("Value ",value.c_str()," out of range."),eh);}
			return x;
			}
		};

	template<>
	struct Converter<uint32_t>
		{
		template<class StringType,class ExceptionHandler>
		static uint32_t convert(const StringType& value,ExceptionHandler& eh)
			{
			auto x=strtoull(value,eh);
			if(x>0xffffffff)
				{raise(Temple::Error("Value ",value.c_str()," out of range."),eh);}
			return x;
			}
		};

	template<>
	struct Converter<uint64_t>
		{
		template<class StringType,class ExceptionHandler>
		static uint64_t convert(const StringType& value,ExceptionHandler& eh)
			{return strtoull(value,eh);}
		};



	template<>
	struct Converter<float>
		{
		template<class StringType,class ExceptionHandler>
		static float convert(const StringType& value,ExceptionHandler& eh)
			{return strtof(value,eh);}
		};

	template<>
	struct Converter<double>
		{
		template<class StringType,class ExceptionHandler>
		static double convert(const StringType& value,ExceptionHandler& eh)
			{return strtod(value,eh);}
		};


//	Wrapper function

	template<class T,class StringType,class ExceptionHandler>
	T convert(const StringType& value,ExceptionHandler& eh)
		{return Converter<T>::convert(value,eh);}


//	Helper for counting digits
	template<class T>
	static constexpr size_t digits() noexcept
		{return static_cast<size_t>( std::log10( std::numeric_limits<T>::max() ) ) + 1;}


//	Number to string conversion

	template<class Number>
	auto convert(Number i) noexcept
		{
		std::array<char,digits<Number>() + 2> buffer; //Sign + nul character
		sprintf(buffer.data(),"%" PRIdMAX ,static_cast<intmax_t>(i));
		return buffer;
		}

	template<>
	auto convert<float>(float x) noexcept
		{
		std::array<char,16> buffer;
		sprintf(buffer.data(),"%.9g",x);
		return buffer;
		}

	template<>
	auto convert<double>(double x) noexcept
		{
		std::array<char,24> buffer;
		sprintf(buffer.data(),"%.17g",x);
		return buffer;
		}
	}

#endif
