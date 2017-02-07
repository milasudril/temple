//@	{"targets":[{"name":"type.hpp","type":"include"}]}

#ifndef TEMPLE_TYPE_HPP
#define TEMPLE_TYPE_HPP

#include "error.hpp"
#include "stringconst.hpp"
#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <memory>

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


	template<class StorageModel>
	class ItemBase;

	template<Type t,class StorageModel>
	struct TypeGet
		{
		typedef typename TypeGet<arrayUnset(t),StorageModel>::type BaseType;
		static constexpr auto id=t;
		typedef typename StorageModel::template ArrayType<BaseType> type;

		typedef decltype(TypeGet<arrayUnset(id),StorageModel>::name.append(" array")) X;

		static constexpr X name=TypeGet<arrayUnset(id),StorageModel>::name.append(" array");
		};

	template<Type t,class StorageModel>
	constexpr typename TypeGet<t,StorageModel>::X TypeGet<t,StorageModel>::name;



	template<class T,class StorageModel>
	struct IdGet
		{
		static constexpr auto id=arraySet(IdGet<typename T::value_type,StorageModel>::id);
		typedef T type;
		};



	template<class StorageModel>
	struct TypeGet<Type::I8,StorageModel>
		{
		static constexpr auto id=Type::I8;
		typedef int8_t type;
		typedef decltype(stringconst("i8")) X;
		static constexpr X name=stringconst("i8");
		};

	template<class StorageModel>
	struct IdGet<int8_t,StorageModel>
		{
		static constexpr auto id=Type::I8;
		typedef int8_t type;
		};



	template<class StorageModel>
	struct TypeGet<Type::I16,StorageModel>
		{
		static constexpr auto id=Type::I16;
		typedef int16_t type;
		typedef decltype(stringconst("i16")) X;
		static constexpr auto name=stringconst("i16");
		};

	template<class StorageModel>
	struct IdGet<int16_t,StorageModel>
		{
		static constexpr auto id=Type::I16;
		typedef int16_t type;
		};



	template<class StorageModel>
	struct TypeGet<Type::I32,StorageModel>
		{
		static constexpr auto id=Type::I32;
		typedef int32_t type;
		typedef decltype(stringconst("i32")) X;
		static constexpr X name=stringconst("i32");
		};

	template<class StorageModel>
	constexpr typename TypeGet<Type::I32,StorageModel>::X TypeGet<Type::I32,StorageModel>::name;

	template<class StorageModel>
	struct IdGet<int32_t,StorageModel>
		{
		static constexpr auto id=Type::I32;
		typedef int32_t type;
		};

	

	template<class StorageModel>
	struct TypeGet<Type::I64,StorageModel>
		{
		static constexpr auto id=Type::I64;
		typedef int64_t type;
		typedef decltype(stringconst("i64")) X;
		static constexpr auto name=stringconst("i64");
		};

	template<class StorageModel>
	struct IdGet<int64_t,StorageModel>
		{
		static constexpr auto id=Type::I64;
		typedef int64_t type;
		};



	template<class StorageModel>
	struct TypeGet<Type::FLOAT,StorageModel>
		{
		static constexpr auto id=Type::FLOAT;
		typedef float type;
		typedef decltype(stringconst("float")) X;
		static constexpr auto name=stringconst("float");
		};

	template<class StorageModel>
	struct IdGet<float,StorageModel>
		{
		static constexpr auto id=Type::FLOAT;
		typedef float type;
		};



	template<class StorageModel>
	struct TypeGet<Type::DOUBLE,StorageModel>
		{
		static constexpr auto id=Type::DOUBLE;
		typedef double type;
		typedef decltype(stringconst("double")) X;
		static constexpr auto name=stringconst("double");
		};

	template<class StorageModel>
	struct IdGet<double,StorageModel>
		{
		static constexpr auto id=Type::DOUBLE;
		typedef double type;
		};



	template<class StorageModel>
	struct TypeGet<Type::STRING,StorageModel>
		{
		static constexpr auto id=Type::STRING;
		typedef typename StorageModel::StringType type;
		static constexpr auto name=stringconst("s");
		};

	template<class StorageModel>
	struct IdGet<typename StorageModel::StringType,StorageModel>
		{
		static constexpr auto id=Type::STRING;
		typedef typename StorageModel::StringType type;
		};



	template<class StorageModel>
	struct TypeGet<Type::COMPOUND,StorageModel>
		{
		static constexpr auto id=Type::COMPOUND;
		typedef typename StorageModel::template MapType<std::unique_ptr< ItemBase<StorageModel> >> type;
		static constexpr auto name=stringconst("");
		};

	template<class StorageModel>
	struct IdGet<typename StorageModel::template MapType<std::unique_ptr<ItemBase<StorageModel>>>,StorageModel>
		{
		static constexpr auto id=Type::COMPOUND;
		typedef typename StorageModel::template MapType<std::unique_ptr< ItemBase<StorageModel> >> type;
		};



	template<Type t,int x,Type t_end,class StorageModel,bool cont>
	struct TypeProcess
		{
		template<class Callback>
		static void doIt(Type type,Callback& cb)
			{	
			if(t==type)
				{cb(TypeGet<t,StorageModel>{});}
			else
				{
				static constexpr auto t_next=step(t,x);
				static constexpr bool cont_next=static_cast<int>(t_next) <= static_cast<int>(t_end);
				TypeProcess<t_next,x,t_end,StorageModel,cont_next>::doIt(type,cb);
				}
			}
		};

	template<Type t,int x,Type t_end,class StorageModel>
	struct TypeProcess<t,x,t_end,StorageModel,0>
		{
		template<class Callback>
		static void doIt(Type type,Callback& cb)
			{assert(0!=0);}
		};

	template<class StorageModel,Type start,int x,Type end,class Callback>
	inline void for_type(Type type,Callback&& cb) 
		{TypeProcess<start,x,end,StorageModel,1>::doIt(type,cb);}

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
		if(str=="f32")
			{return Type::FLOAT;}
		if(str=="f64")
			{return Type::DOUBLE;}
		if(str=="comp")
			{return Type::COMPOUND;}
		raise(Error("The type identifier «",str.c_str(),"» does not correspond to a known type."),eh);
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
				return "f32";
			case Type::DOUBLE:
				return "f64";
			case Type::STRING:
				return "s";
			case Type::COMPOUND:
				return "comp";
			default:
				assert(1!=1 && "Internal error");
				return nullptr;
			}
		assert(1!=1 && "Internal error");
		return nullptr;
		}
	}

#endif
