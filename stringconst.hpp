//@	{"targets":[{"name":"stringconst.hpp","type":"include"}]}


#ifndef TEMPLE_STRINGCONST_HPP
#define TEMPLE_STRINGCONST_HPP

#include <cstddef>
#include <cassert>

namespace Temple
	{
	template<class T,size_t K>
	class StringConst
		{
		public:
			constexpr StringConst() noexcept:m_buffer{}
				{}
			constexpr explicit StringConst(T a) noexcept :m_buffer{a}
				{}
			constexpr explicit StringConst(const T(&a)[K]):m_buffer{}
				{
				for(size_t k=0;k<K;++k)
					{m_buffer[k]=a[k];}
				}
			  
			constexpr const T* begin() const noexcept
				{return m_buffer;}

			constexpr const T* end() const noexcept
				{return m_buffer + (K-1);}

			constexpr const T* c_str() const noexcept
				{return m_buffer;}

			constexpr T* data() noexcept
				{return m_buffer;}
			  
			template<size_t L>
			constexpr StringConst<T,K+L-1> append(const T(&a)[L]) const noexcept
				{
				StringConst<T,K+L-1> ret{};
				for(size_t k=0;k<K-1;++k)
					{ret[k]=m_buffer[k];}
				for(size_t l=0;l<L;++l)
					{ret[K+l-1]=a[l];}
				return ret;
				}

			template<size_t L>
			constexpr StringConst<T,K+L-1> append(const StringConst<T,L>& a) const noexcept
				{
				StringConst<T,K+L-1> ret{};
				for(size_t k=0;k<K-1;++k)
					{ret[k]=m_buffer[k];}
				for(size_t l=0;l<L;++l)
					{ret[K+l-1]=a[l];}
				return ret;
				}
			  

			constexpr StringConst<T,K+1> append(const T& x) const noexcept
				{
				StringConst<T,K+1> ret{};
				for(size_t k=0;k<K-1;++k)
					{ret[k]=m_buffer[k];}
				ret[K-1]=x;
				return ret;
				}
			  
			constexpr T& operator[](size_t k) noexcept
				{
				assert(k<K);
				return m_buffer[k];
				}
			  
			constexpr const T& operator[](size_t k) const noexcept
				{
				assert(k<K);
				return m_buffer[k];
				}

			static constexpr size_t size() noexcept 
				{return K-1;}
		  
		private:
			T m_buffer[K];
		};

		template<class T,size_t K>
		constexpr auto stringconst(const T(&a)[K]) noexcept
			{return StringConst<T,K>(a);}

		template<class T,size_t K,size_t L>
		constexpr auto concat(const T(&a)[K],const T(&b)[L]) noexcept
			{return StringConst<T,K>(a).append(b);}

		template<class T,size_t K,class StrB>
		constexpr auto concat(const StringConst<T,K>& a,const StrB& b) noexcept
			{return a.append(b);}

		template<class T,size_t L>
		constexpr auto concat(T a,const T(&b)[L]) noexcept
			{return StringConst<T,2>(a).append(b);}

		template<class StrA,class StrB,class ... Strings>
		constexpr auto concat(const StrA& a,const StrB& b,const Strings&...strings) noexcept
			{return concat(concat(a,b),strings...);}
	}
#endif
