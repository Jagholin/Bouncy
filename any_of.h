#pragma once
#include <type_traits>
#include <exception>
#include <array>

template <typename T, typename... Types>
struct type_index
{
	// nothing here
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
	std::cout << "conversion" << std::endl;
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

template <typename... Args>
class any_of
{
public:
	any_of(): m_object(nullptr) {
		m_typeDesc = -1;
	}

	any_of(const any_of<Args...>& rhs):m_object(rhs.m_object), m_typeDesc(rhs.m_typeDesc) {
	}

	any_of(any_of<Args...>&& rhs) :m_object(rhs.m_object), m_typeDesc(rhs.m_typeDesc) {
		rhs.m_object = nullptr;
		rhs.m_typeDesc = -1;
	}

	any_of(NullType): m_object(nullptr) {
		m_typeDesc = -1;
	}

	template <typename T>
	explicit any_of(T& obj) : m_object(&obj), m_typeDesc(type_index<T, Args...>::value)
	{
		// tada
	}

	template <typename A, typename... Bs>
	friend A& as(const any_of<Bs...>&);

	operator bool() {
		return m_object != nullptr;
	}

	template<typename T>
	bool isA() const {
		return m_typeDesc == type_index<T, Args...>::value;
	}

	any_of<Args...>& operator=(const any_of<Args...>& rhs) {
		m_object = rhs.m_object;
		m_typeDesc = rhs.m_typeDesc;
		return *this;
	}

private:
	void* m_object;
	int m_typeDesc;
	static std::array<std::array<std::pair<bool, void*(*)(void*)>, sizeof...(Args)>, sizeof...(Args)> 
		m_conversionTable;
};

template <typename T, typename... Types>
T& as(const any_of<Types...>& myAny)
{
	if (type_index<T, Types...>::value == myAny.m_typeDesc)
		return *(reinterpret_cast<T*>(myAny.m_object));

	if (any_of<Types...>::m_conversionTable[type_index<T, Types...>::value][myAny.m_typeDesc].first) {
		// Conversion to T possible, performing...
		return *(reinterpret_cast<T*>(any_of<Types...>::m_conversionTable[type_index<T, Types...>::value][myAny.m_typeDesc].second(myAny.m_object)));
	}
	throw std::exception("any_of cannot be cast to this datatype.");
}

template <typename... Args>
std::array<std::array<std::pair<bool, void*(*)(void*)>, sizeof...(Args)>, sizeof...(Args)>
any_of<Args...>::m_conversionTable = initConversionTable<Args...>(); 