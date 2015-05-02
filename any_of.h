#pragma once
#include <type_traits>
#include <exception>

template <typename T, typename... Types>
struct type_index
{
	// nothing here
};

template <typename T, typename... Types>
struct type_index < T, T, Types... >
{
	static const unsigned int value = 0;
};

template <typename T, typename A, typename... Types>
struct type_index < T, A, Types... >
{
	static const unsigned int value = type_index<T, Types...>::value + 1;
};

template <typename... Args>
class any_of
{
public:
	any_of(): m_object(nullptr) {
		m_typeDesc = -1;
	}

	template <typename T>
	explicit any_of(const T& obj) : m_object(&obj), m_typeDesc(type_index<T, Args...>::value)
	{
		// tada
	}

	template <typename A, typename... Bs>
	friend A as(const any_of<Bs...>&);

	operator bool() {
		return m_object != nullptr;
	}

	template<typename T>
	bool isA() {
		return m_typeDesc == type_index<T, Args...>::value;
	}

private:
	void* m_object;
	const int m_typeDesc;
};

template <typename T, typename... Types>
T& as(const any_of<Types...>& myAny)
{
	if (type_index<T>::value == myAny.m_typeDesc)
		return *(reinterpret_cast<T*>(m_object));
	else
		throw std::exception("any_of cannot be cast to this datatype.");
}