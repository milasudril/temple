//@	{"targets":[{"name":"type.hpp","type":"include"}]}

#ifndef TEMPLE_TYPE_HPP
#define TEMPLE_TYPE_HPP

#include "error.hpp"
#include <cassert>
#include <type_traits>

namespace Temple
	{
	enum class Type:int
		{
		 I8=0,I8_ARRAY
		,I16,I16_ARRAY
		,I32,I32_ARRAY
		,I64,I64_ARRAY
		,FLOAT,FLOAT_ARRAY
		,DOUBLE,DOUBLE_ARRAY
		,STRING,STRING_ARRAY
		,COMPOUND,COMPOUND_ARRAY
		};

	inline constexpr Type arraySet(Type t)
		{return static_cast<Type>(static_cast<int>(t) | 1);}

	inline constexpr Type arrayUnset(Type t)
		{return static_cast<Type>(static_cast<int>(t)&( ~1));}

	inline constexpr Type next(Type t)
		{return static_cast<Type>(static_cast<int>(t) + 1);}

	inline constexpr Type step(Type t,int x)
		{return static_cast<Type>(static_cast<int>(t) + x);}

	inline constexpr Type previous(Type t)
		{return static_cast<Type>(static_cast<int>(t) - 1);}



	template<Type t,class StorageModel>
	struct TypeGet:private TypeGet<arrayUnset(t),StorageModel>
		{
		typedef typename TypeGet<arrayUnset(t),StorageModel>::type BaseType;
		static constexpr auto id=t;
		typedef typename StorageModel::template ArrayType<BaseType> type;
		};

	template<class StorageModel>
	struct TypeGet<Type::I8,StorageModel>
		{
		static constexpr auto id=Type::I8;
		typedef int8_t type;
		};

	template<class StorageModel>
	struct TypeGet<Type::I16,StorageModel>
		{
		static constexpr auto id=Type::I16;
		typedef int8_t type;
		};

	template<class StorageModel>
	struct TypeGet<Type::I32,StorageModel>
		{
		static constexpr auto id=Type::I32;
		typedef int32_t type;
		};

	template<class StorageModel>
	struct TypeGet<Type::I64,StorageModel>
		{
		static constexpr auto id=Type::I64;
		typedef int64_t type;
		};

	template<class StorageModel>
	struct TypeGet<Type::FLOAT,StorageModel>
		{
		static constexpr auto id=Type::FLOAT;
		typedef float type;
		};

	template<class StorageModel>
	struct TypeGet<Type::DOUBLE,StorageModel>
		{
		static constexpr auto id=Type::DOUBLE;
		typedef double type;
		};

	template<class StorageModel>
	struct TypeGet<Type::STRING,StorageModel>
		{
		static constexpr auto id=Type::STRING;
		typedef typename StorageModel::StringType type;
		};

	template<class StorageModel>
	struct TypeGet<Type::COMPOUND,StorageModel>
		{
		static constexpr auto id=Type::COMPOUND;
		typedef void type;
		};



	template<Type t,int x,Type t_end,class StorageModel,bool cont>
	struct TypeProcess
		{
		template<class Callback,class ExceptionHandler>
		static void doIt(Type type,Callback& cb,ExceptionHandler& eh)
			{	
			if(t==type)
				{
				cb(TypeGet<t,StorageModel>{});
				}
			else
				{
				static constexpr auto t_next=step(t,x);
				static constexpr bool cont_next=static_cast<int>(t_next) <= static_cast<int>(t_end);
				TypeProcess<t_next,x,t_end,StorageModel,cont_next>::doIt(type,cb,eh);
				}
			}
		};

	template<Type t,int x,Type t_end,class StorageModel>
	struct TypeProcess<t,x,t_end,StorageModel,0>
		{
		template<class Callback,class ExceptionHandler>
		static void doIt(Type type,Callback& cb,ExceptionHandler& eh)
			{eh.raise(Error("Internal error: Type not found."));}
		};

	template<class StorageModel,Type start,int x,Type end,class Callback,class ExceptionHandler>
	inline void for_type(Type type,Callback&& cb,ExceptionHandler& eh) 
		{TypeProcess<start,x,end,StorageModel,1>::doIt(type,cb,eh);}

	template<class StringType,class ExceptionHandler>
	inline Type type(const StringType& str,ExceptionHandler& eh)
		{
		if(str=="s")
			{return Type::STRING;}
		if(str=="i8")
			{return Type::I8;}
		if(str=="i16")
			{return Type::I16;}
		if(str=="i32")
			{return Type::I32;}
		if(str=="i64")
			{return Type::I64;}
		if(str=="f")
			{return Type::FLOAT;}
		if(str=="d")
			{return Type::DOUBLE;}
		if(str=="")
			{return Type::COMPOUND;}
		eh.raise(Error("The type identifier ",str.c_str()," does not correspond to a known type."));
		return Type::COMPOUND;
		}

	inline const char* type(Type type)
		{
		switch(arrayUnset(type))
			{
			case Type::I8:
				return "i8";
			case Type::I16:
				return "i16";
			case Type::I32:
				return "i32";
			case Type::I64:
				return "i64";
			case Type::FLOAT:
				return "f";
			case Type::DOUBLE:
				return "d";
			case Type::STRING:
				return "s";
			case Type::COMPOUND:
				return "";
			default:
				assert(1!=1);
				return nullptr;
			}
		assert(1!=1);
		return nullptr;
		}
	}

#endif
