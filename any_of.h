#pragma once
#include <type_traits>
#include <exception>
#include <array>

namespace anyof_details 
{
	template <typename T, typename... Types>
	struct type_index
	{
		// nothing here
		//static_assert(false, "T is not in the provided type list for any_of");
	};

	template <typename T, typename... Types>
	struct type_index < T, T, Types... >
	{
		static const int value = 0;
	};

	template <typename T, typename A, typename... Types>
	struct type_index < T, A, Types... >
	{
		static const int value = type_index<T, Types...>::value + 1;
	};

	struct NullType
	{};

	template <typename A, typename B>
	void* convertFunc(void* sth)
	{
		return (static_cast<A*>(reinterpret_cast<B*>(sth)));
	}

	template <typename A, typename B, typename Enable = void>
	struct convertSelector
	{
		typedef void*(*funcPtr)(void*);
		static funcPtr value()
		{
			return nullptr;
		}
	};

	template <typename A, typename B>
	struct convertSelector<A, B, typename std::enable_if<std::is_convertible<B*, A*>::value>::type>
	{
		typedef void*(*funcPtr)(void*);
		static funcPtr value() {
			return convertFunc<A, B>;
		}
	};

	template <typename T, typename... Types>
	std::array<std::pair<bool, void*(*)(void*)>, sizeof...(Types)> initConversionRow()
	{
		return{ std::pair<bool, void*(*)(void*)>(std::is_convertible<T*, Types*>::value, convertSelector<Types, T>::value())... };
	}

	template <typename... Args>
	std::array<std::array<std::pair<bool, void*(*)(void*)>, sizeof...(Args)>, sizeof...(Args)> initConversionTable()
	{
		return{ (initConversionRow<Args, Args...>())... };
	}

	// Ownership strategy 1: any_of allocates its own copy of data with new/copy ctor, and removes with delete 
	template <typename T>
	struct standard_create
	{
		static void* doit(const T& rhs) {
			return new T{ rhs };
		}
		static void* doit(T&& rhs) {
			return new T{ rhs };
		}
	};

	template <typename T>
	struct standard_create < T* >
	{
		static void* doit(T* rhs) {
			return rhs;
		}
	};

	struct AnyofOwnsData
	{
		template <typename T>
		static void* create(T&& rhs){
			//return new T{ std::move(rhs) };
			return standard_create<T>::doit(std::forward(rhs));
		}

		template <typename T>
		static void destroy(void* rhs){
			delete reinterpret_cast<T*>(rhs);
		}

		template <typename T>
		static bool equalsCopy(void* from, void*& to){
			to = from;
			return false;
		}
	};
}

using anyof_details::AnyofOwnsData;
using anyof_details::NullType;

template <typename OwnershipStrategy, typename... Types>
class any_of
{
public:

	typedef any_of<OwnershipStrategy, Types...> any_type;

	any_of(): m_object(nullptr) {
		m_typeDesc = -1;
	}

	any_of(const any_type& rhs) :m_object(rhs.m_object), m_typeDesc(rhs.m_typeDesc), m_destructor{ rhs.m_destructor }, m_cloner{ rhs.m_cloner } {
	}

	any_of(any_type&& rhs) :m_object(rhs.m_object), m_typeDesc(rhs.m_typeDesc), m_destructor{ rhs.m_destructor }, m_cloner{ rhs.m_cloner } {
		rhs.m_object = nullptr;
		rhs.m_typeDesc = -1;
	}

	any_of(NullType): m_object(nullptr) {
		m_typeDesc = -1;
	}

	~any_of() {
		if (m_object)
			m_destructor(m_object);
	}

	template <typename T>
	explicit any_of(const T& obj) : m_object(OwnershipStrategy::create<T>(obj)), m_typeDesc(anyof_details::type_index<T, Types...>::value)
	{
		m_destructor = &OwnershipStrategy::destroy < T > ;
		m_cloner = &OwnershipStrategy::equalsCopy < T > ;
	}

	template <typename T>
	explicit any_of(T* obj) : m_object(OwnershipStrategy::create<T>(obj)), m_typeDesc(anyof_details::type_index<T, Types...>::value)
	{
		m_destructor = &OwnershipStrategy::destroy < T > ;
		m_cloner = &OwnershipStrategy::equalsCopy < T > ;
	}

	operator bool() {
		return m_object != nullptr;
	}

	template<typename T>
	bool isA() const {
		using namespace anyof_details;
		//return m_typeDesc == type_index<T, Types...>::value;
		return m_conversionTable[m_typeDesc][type_index<T, Types...>::value].first;
	}

	any_type& operator=(any_type&& rhs) {
		using namespace anyof_details;

		if (m_object) {
			m_destructor(m_object);
		}
		m_typeDesc = rhs.m_typeDesc;
		m_destructor = rhs.m_destructor;
		m_cloner = rhs.m_cloner;
		m_object = rhs.m_object;

		rhs.m_object = nullptr;
		rhs.m_typeDesc = -1;
		rhs.m_destructor = nullptr;
		rhs.m_cloner = nullptr;
		return *this;
	}

	any_type& operator=(any_type const& rhs) {
		using namespace anyof_details;

		if (m_object) {
			m_destructor(m_object);
		}
		m_typeDesc = rhs.m_typeDesc;
		m_destructor = rhs.m_destructor;
		m_cloner = rhs.m_cloner;
		if (rhs.m_cloner(rhs.m_object, m_object) == false)
		{
			rhs.m_object = nullptr;
			rhs.m_typeDesc = -1;
			rhs.m_destructor = nullptr;
			rhs.m_cloner = nullptr;
		}
		return *this;
	}

	template <typename TA, typename TB, typename... TCs>
	friend TA& as(const any_of<TB, TCs...>&);
private:
	void* m_object;
	int m_typeDesc;
	static std::array<std::array<std::pair<bool, void*(*)(void*)>, sizeof...(Types)>, sizeof...(Types)> 
		m_conversionTable;
	void (*m_destructor)(void*);
	bool (*m_cloner)(void*, void*&);
};

template <typename T, typename S, typename... Types>
T& as(const any_of<S, Types...>& myAny)
{
	using namespace anyof_details;

	if (type_index<T, Types...>::value == myAny.m_typeDesc)
		return *(reinterpret_cast<T*>(myAny.m_object));

	if (any_of<S, Types...>::m_conversionTable[myAny.m_typeDesc][type_index<T, Types...>::value].first) {
		// Conversion to T possible, performing...
		return *(reinterpret_cast<T*>(any_of<S, Types...>::m_conversionTable[myAny.m_typeDesc][type_index<T, Types...>::value].second(myAny.m_object)));
	}
	throw std::exception("any_of cannot be cast to this datatype.");
}

template <typename T>
struct smart_as
{
	template <typename S, typename... Types>
	static T& doit(const any_of<S, Types...>& myAny) {
		return ::as<T>(myAny);
	}
};

template <typename T>
struct smart_as < ref_ptr<T> >
{
	template <typename S, typename... Types>
	static ref_ptr<T> doit(const any_of<S, Types...>& myAny) {
		return ref_ptr < T > {& ::as<T>(myAny)};
	}
};

template <typename T>
struct smart_is
{
	template <typename S, typename... Types>
	static bool doit(const any_of<S, Types...>& myAny) {
		return myAny.isA<T>();
	}
};

template <typename T>
struct smart_is < ref_ptr<T> >
{
	template <typename S, typename... Types>
	static bool doit(const any_of<S, Types...>& myAny) {
		return myAny.isA<T>();
	}
};

template <typename T>
struct smart_set
{
	template <typename S, typename... Types>
	static void doit(any_of<S, Types...>& myAny, T val) {
		myAny = any_of < S, Types... > {std::move(val)};
	}
};

template <typename T>
struct smart_set < ref_ptr<T> >
{
	template <typename S, typename... Types>
	static void doit(any_of<S, Types...>& myAny, ref_ptr<T> val) {
		myAny = any_of < S, Types... > {val.get()};
	}
};

template <typename S, typename... Types>
std::array<std::array<std::pair<bool, void*(*)(void*)>, sizeof...(Types)>, sizeof...(Types)>
any_of<S, Types...>::m_conversionTable = anyof_details::initConversionTable<Types...>(); 
