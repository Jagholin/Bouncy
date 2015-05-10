#pragma once

#include <GL/glew.h>
#include <unordered_map>
#include <string>
#include <memory>
#include "uniform.h"
#include "any_of.h"
#include "lifetimeobserver.h"
#include "ref_ptr.h"

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

template <typename T>
struct deleter_policy < ref_ptr<T> >
{
	static void dealloc(ref_ptr<T>) {
		// do nothin'.
	}
};

template <typename RegistryType>
class RegistryDataItem : public LifetimeObservable
{
public: 
	typedef RegistryDataItem<RegistryType>* pointer;
	typedef typename RegistryType::container_type value_type;
	typedef std::unordered_map<std::string, pointer> itemmap_type;

protected:
	//RegistryType& m_owner;
	itemmap_type m_children;
	pointer m_parent;
	std::string m_name;
	value_type m_value;

public:
	RegistryDataItem(const RegistryDataItem&) = delete;
	RegistryDataItem(RegistryDataItem&&) = delete;

	void deleteChild(typename itemmap_type::const_iterator it) {
		m_children.erase(it);
		deleter_policy<pointer>::dealloc(it->second);
	}

	void deleteChild(std::string name) {
		auto child = m_children.at(name);
		m_children.erase(name);
		deleter_policy<pointer>::dealloc(child);
	}

	pointer createChild(std::string name, value_type val) {
		pointer newVal{ new RegistryDataItem<RegistryType>(this, name, value_type{ std::move(val) }) };
		m_children.insert(std::make_pair(name, newVal));
		return newVal;
	}

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

	auto cbegin() const -> decltype(m_children.cbegin()){
		return m_children.cbegin();
	}

	auto cend() const -> decltype(m_children.cend()){
		return m_children.cend();
	}

	template <typename T>
	auto as() const -> decltype(::smart_as<T>::doit(m_value)) {
		return ::smart_as<T>::doit(m_value);
	}

	template <typename T>
	bool isValueOf() const {
		return ::smart_is<T>::doit(m_value);
	}

	template <typename T>
	void setValue(T& val) {
		//m_value = value_type{ val };
		smart_set<T>::doit(m_value, val);
	}

	friend RegistryType;
	friend struct deleter_policy < pointer > ;

protected:
	RegistryDataItem(/*RegistryType& owner,*/ pointer parent, std::string const& name, value_type val):
	/*m_owner(owner),*/ m_parent(parent), m_name(name), m_value(std::move(val)){

	}

	~RegistryDataItem() {
		for (auto const & child : m_children) {
			deleter_policy<pointer>::dealloc(child.second);
		}
	}
};

//typedef RegistryDataItem::pointer RegistryDataPointer;

template <typename... Types>
class Registry
{
public:
	typedef RegistryDataItem<Registry<Types...>> item_type;
	typedef any_of<AnyofOwnsSharedData, Types...> container_type;
	typedef typename item_type::pointer itemptr_type;

	Registry() :m_rootItem{ new item_type( nullptr, "root", container_type{}) } {
		
	}

	~Registry() {

	}

	itemptr_type item_at(std::string const& address, itemptr_type rootOverload = nullptr) const {
		if (address == "/")
			return m_rootItem;

		enum {STATE_SLASH, STATE_CHAR} currentState;
		std::string addrPart;
		itemptr_type currItem;
		if (rootOverload)
		{
			currItem = rootOverload;
			currentState = STATE_CHAR;
		}
		else
		{
			currItem = m_rootItem;
			currentState = STATE_SLASH;
		}

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
	T value_at(std::string const& addr) const {
		itemptr_type myItem = item_at(addr);
		if (!myItem)
			throw std::out_of_range("There is no items at " + addr);

		return as<T>(myItem->m_value);
	}

	template <typename T>
	itemptr_type createItemAt(std::string const& addr, std::string const& name, T value) const {
		itemptr_type myItem = item_at(addr);
		if (!myItem)
			throw std::out_of_range("There is no items at " + addr);
		return createItemAt<T>(myItem, name, std::move(value));
	}

private:
	template <typename T_>
	struct value2any {
		static container_type doit(T_& val) {
			return container_type{ std::move(val) };
		}
	};
	template <typename T_>
	struct value2any < ref_ptr<T_> > {
		static container_type doit(ref_ptr<T_> const& val) {
			return container_type{ val.get() };
		}
	};

public:
	template <typename T>
	itemptr_type createItemAt(itemptr_type parent, std::string const& name, T value) const {
		auto oldItem = parent->m_children.find(name);

		if (oldItem == parent->m_children.end()) {
			//itemptr_type newItem{ new item_type( parent, name, container_type{ std::move(value) } ) };
			//parent->m_children.insert(std::make_pair(name, newItem));
			auto newItem = parent->createChild(name, value2any<T>::doit(value));
			return newItem;
		}
		else {
			oldItem->second->m_value = value2any<T>::doit(value);
			return oldItem->second;
		}
	}

	void removeItem(itemptr_type const& iptr) const {
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