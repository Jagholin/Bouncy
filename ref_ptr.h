#pragma once
#include <type_traits>
#include <iostream>

struct InnerRefcounter
{
private:
	int m_counter;
    static int objid;
    int m_objid;
public:
	InnerRefcounter() : m_counter(0) {
        //m_objid = ++objid;
        //std::cout << "Ctor countered " << m_objid << std::endl;
	}
    virtual ~InnerRefcounter() {
        //std::cout << "Destr countered " << m_objid << std::endl;
    }
	void addref() {
		++m_counter;
	}
	int deref() {
		return --m_counter;
	}
};

struct AnyofOwnsSharedData
{
	struct RefcountedData {
		int m_counter;
		void* m_data;
    };

    template <typename T>
    static std::enable_if_t<!std::is_base_of<InnerRefcounter, T>::value, void*> create(const T& rhs){
        return new RefcountedData{
            1,
            anyof_details::standard_create<T>::doit(rhs)
        };
    }

	template <typename T>
	static std::enable_if_t<!std::is_base_of<InnerRefcounter, T>::value, void*> create(T&& rhs){
		return new RefcountedData{
            1,
            anyof_details::standard_create<T>::doit(std::move(rhs))
		};
    }

    template <typename T>
    static std::enable_if_t<!std::is_base_of<InnerRefcounter, T>::value, void*> create(T* rhs){
        return new RefcountedData{
            1,
            anyof_details::standard_create<T*>::doit(rhs)
        };
    }

	template <typename T>
	static std::enable_if_t<!std::is_base_of<InnerRefcounter, T>::value, void> destroy(void* rhs){
		RefcountedData* rdata = reinterpret_cast<RefcountedData*>(rhs);
		if ((--rdata->m_counter) > 0)
			return;
		delete reinterpret_cast<T*>(rdata->m_data);
	}

    template <typename T>
    static std::enable_if_t<std::is_base_of<InnerRefcounter, T>::value, void*> create(const T& rhs){
        auto temp = anyof_details::standard_create<T>::doit(rhs);
        reinterpret_cast<T*>(temp)->addref();
        return temp;
    }

	template <typename T>
	static std::enable_if_t<std::is_base_of<InnerRefcounter, T>::value, void*> create(T&& rhs){
		//return new T{ std::move(rhs) };
        auto temp = anyof_details::standard_create<T>::doit(std::move(rhs));
        reinterpret_cast<T*>(temp)->addref();
        return temp;
	}

    template <typename T>
    static std::enable_if_t<std::is_base_of<InnerRefcounter, T>::value, void*> create(T* rhs){
        rhs->addref();
        return rhs;
    }

	template <typename T>
	static std::enable_if_t<std::is_base_of<InnerRefcounter, T>::value, void> destroy(void* rhs){
		InnerRefcounter* refdata = reinterpret_cast<T*>(rhs);
		if (refdata->deref() > 0)
			return;
		delete reinterpret_cast<T*>(rhs);
	}

	template <typename T>
	static std::enable_if_t<!std::is_base_of<InnerRefcounter, T>::value, bool> equalsCopy(void* from, void*& to){
		to = from;
		RefcountedData* rdata = reinterpret_cast<RefcountedData*>(from);
		++rdata->m_counter;
		return true;
	}

	template <typename T>
	static std::enable_if_t<std::is_base_of<InnerRefcounter, T>::value, bool> equalsCopy(void* from, void*& to){
		to = from;
		InnerRefcounter* rdata = reinterpret_cast<T*>(from);
		rdata->addref();
		return true;
	}
};

template <class T>
class ref_ptr
{
public:
	template <typename R>
	friend class ref_ptr;

	static_assert(std::is_base_of<InnerRefcounter, T>::value, "T is not derived from InnerRefcounter class");

	ref_ptr() : m_data(nullptr) {

	}

	explicit ref_ptr(T* data) : m_data(data){
		data->addref();
	}

	template <typename R>
	ref_ptr(const ref_ptr<R>& rhs) : m_data(rhs.m_data) {
        std::cout << "ref_ptr copy" << std::endl;
		static_assert(std::is_convertible<R*, T*>::value, "An implicit conversion R* -> T* must be possible");
		if (m_data)
			m_data->addref();
	}

    ref_ptr(const ref_ptr<T>& rhs) : m_data(rhs.m_data) {
        std::cout << "ref_ptr copy2" << std::endl;
        if (m_data)
            m_data->addref();
    }

	~ref_ptr() {
		if (m_data && m_data->deref() <= 0)
			delete m_data;
	}

    ref_ptr<T>& operator=(const ref_ptr<T>& rhs) {
        if (m_data && m_data->deref() <= 0)
            delete m_data;
        m_data = rhs.m_data;
        if (m_data)
            m_data->addref();
        return *this;
    }

    ref_ptr<T>& operator=(ref_ptr<T>&& rhs) {
        if (m_data && m_data->deref() <= 0)
            delete m_data;
        m_data = rhs.m_data;
        rhs.m_data = nullptr;
        return *this;
    }

	T* operator->() const {
		return m_data;
	}

	T& operator*() const {
		return *m_data;
	}

	T* get() const {
		return m_data;
	}

	T* release() {
		if (!m_data)
			return m_data;
		m_data->deref();
		T* temp = m_data;
		m_data = nullptr;
		return temp;
	}

    template <typename T_, typename R>
	friend bool operator==(const ref_ptr<T_>&, const ref_ptr<R>&);

protected:
	T* m_data;
};

template <typename T, typename R>
bool operator == (const ref_ptr<T>& lhs, const ref_ptr<R>& rhs)
{
	return lhs.m_data == rhs.m_data;
}

template<typename T, typename... Args>
ref_ptr<T> make_ref(Args&&... args) {
	return ref_ptr < T > {new T{ std::forward<Args>(args)... }};
}
