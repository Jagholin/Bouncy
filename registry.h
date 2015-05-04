#pragma once

#include <GL/glew.h>
#include <unordered_map>
#include <string>
#include <memory>
#include "uniform.h"
#include "any_of.h"
#include "lifetimeobserver.h"

template <typename T>
struct deleter_policy
{
	static void dealloc(T&) {
		throw std::invalid_argument("T is not a pointer type");
	}
};

template <typename T> 
struct deleter_policy<T*>
{
	static void dealloc(T* ptr) {
		delete ptr;
	}
};

template <typename T>
struct deleter_policy<std::shared_ptr<T>>
{
	static void dealloc(std::shared_ptr<T>) {
		// do nothin'.
	}
};

template <typename RegistryType>
class RegistryDataItem : public LifetimeObservable
{
public:
	RegistryDataItem(const RegistryDataItem&) = delete;
	RegistryDataItem(RegistryDataItem&&) = delete;

	typedef RegistryDataItem<RegistryType>* pointer;
	typedef RegistryType::container_type value_type;

	//virtual bool isUniformData() const{
	//	return false;
	//}

	//virtual std::shared_ptr<Uniform> asUniform(std::string const&)
	//{
	//	return std::shared_ptr < Uniform > {nullptr};
	//}

	auto begin() -> decltype(m_children.begin()){
		return m_children.begin();
	}

	auto end() -> decltype(m_children.end()){
		return m_children.end();
	}

	auto begin() const -> decltype(m_children.begin()){
		return m_children.begin();
	}

	auto end() const -> decltype(m_children.end()){
		return m_children.end();
	}

	auto cbegin() const -> decltype(m_children.begin()){
		return m_children.cbegin();
	}

	auto cend() const -> decltype(m_children.end()){
		return m_children.cend();
	}

	template <typename T>
	T& as() const {
		return as<T>(m_value);
	}

	template <typename T>
	bool isValueOf() const {
		return m_value.isA<T>();
	}

	template <typename T>
	void setValue(T const& val) {
		m_value = value_type{ val };
	}

	friend class RegistryType;
protected:
	RegistryDataItem(RegistryType& owner, pointer parent, std::string const& name, value_type const& val):
	m_owner(owner), m_parent(parent), m_name(name), m_value(val){

	}

	RegistryType& m_owner;
	std::unordered_map<std::string, pointer> m_children;
	pointer m_parent;
	std::string m_name;
	value_type m_value;
};

//typedef RegistryDataItem::pointer RegistryDataPointer;

template <typename... Types>
class Registry
{
public:
	typedef RegistryDataItem<Registry<Types...>> item_type;
	typedef any_of<Types...> container_type;
	typedef item_type::pointer itemptr_type;

	Registry() :m_rootItem{ new item_type(*this, nullptr, "root", container_type{}) } {
		
	}

	itemptr_type item_at(std::string const& address) {
		if (address == "/"s)
			return m_rootItem;

		enum {STATE_SLASH, STATE_CHAR} currentState;
		currentState = STATE_SLASH;
		std::string addrPart;
		itemptr_type currItem = m_rootItem;
		try{

		for (char ch : address) {
			if (currentState == STATE_SLASH) {
				if (ch != '/') {
					return (itemptr_type)nullptr;
				}
				currentState = STATE_CHAR;
			}
			else if (currentState == STATE_CHAR) {
				if (ch == '/') {
					currItem = currItem->m_children.at(addrPart);
					addrPart.clear();
					continue;
				}
				addrPart += ch;
			}
		}
		currItem = currItem->m_children.at(addrPart);
		}
		catch (std::out_of_range e) {
			return (itemptr_type)nullptr;
		}

		return currItem;
	}

	template<typename T>
	T value_at(std::string const& addr) {
		itemptr_type myItem = item_at(addr);
		if (!myItem)
			throw std::out_of_range("There is no items at "s + addr);

		return as<T>(myItem->m_value);
	}

	template <typename T>
	itemptr_type createItemAt(std::string const& addr, std::string const& name, T value) {
		itemptr_type myItem = item_at(addr);
		if (!myItem)
			throw std::out_of_range("There is no items at "s + addr);

		if ((auto oldItem = myItem->m_children.find(name)) == myItem->m_children.end()) {
			itemptr_type newItem{ new item_type(*this, myItem, name, container_type{ value } ) };
			myItem->m_children.insert(name, newItem);
		}
		else {
			oldItem->second->m_value = container_type{ value };
		}
			
	}

	void removeItem(itemptr_type const& iptr) {
		// remove item from parents children container
		if (iptr->m_parent) {
			iptr->m_parent->m_children.erase(iptr->m_name);
			deleter_policy<itemptr_type>::dealloc(iptr);
		}
	}

protected:
	itemptr_type m_rootItem;
};



//typedef TypedStateData<GLuint> GLObjectStateData;