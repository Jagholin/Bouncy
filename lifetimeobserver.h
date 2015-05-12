#pragma once
#include <deque>
#include <functional>

struct LifetimeObserver;
struct LifetimeObservable {
	virtual ~LifetimeObservable();

	void addObserver(LifetimeObserver*);
	void removeObserver(LifetimeObserver*);

private:
	std::deque<LifetimeObserver*> m_observers;
};

struct LifetimeObserver final {
	LifetimeObserver(LifetimeObservable* observ, std::function<void()> const destrEvent):
	m_observable(observ),
	m_destrEvent(destrEvent){
        if (m_observable)
    		m_observable->addObserver(this);
	}

    /*LifetimeObserver(const LifetimeObserver& rhs) :
        m_observable(rhs.m_observable),
        m_destrEvent(rhs.m_destrEvent){
        if (m_observable)
            m_observable->addObserver(this);
    }*/

	virtual ~LifetimeObserver() {
		if (m_observable)
			m_observable->removeObserver(this);
	}
	void emitDestroyed() {
		m_observable = nullptr;
		m_destrEvent();
	}

private:
	LifetimeObservable* m_observable;
	std::function<void()> m_destrEvent;
};

template <typename T>
class life_ptr final
{
public:
    static_assert(std::is_base_of<LifetimeObservable, T>::value, "T has to be a LifetimeObservable class");

    life_ptr() : m_value(nullptr), m_observer(nullptr, std::function < void > {}) {
        // no ope
    }

    life_ptr(T* value) : m_value(value), m_observer(m_value, std::bind(&life_ptr<T>::onDeath, this)) {

    }

    life_ptr(life_ptr<T> const& rhs) : m_value(rhs.m_value), m_observer(m_value, std::bind(&life_ptr<T>::onDeath, this)) {

    }

    life_ptr(life_ptr<T>&& rhs) : m_value(rhs.m_value), m_observer(m_value, std::bind(&life_ptr<T>::onDeath, this)) {

    }

    T* operator->() {
        return m_value;
    }

    T* operator*() {
        return m_value;
    }

    life_ptr<T>& operator=(life_ptr<T> const& rhs) {
        m_value = rhs->m_value;
        m_observer = LifetimeObserver(m_value, std::bind(&life_ptr<T>::onDeath, this));
        return *this;
    }

    life_ptr<T>& operator=(life_ptr<T>&& rhs) {
        m_value = rhs->m_value;
        m_observer = LifetimeObserver(m_value, std::bind(&life_ptr<T>::onDeath, this));
        return *this;
    }

    operator bool() const {
        return m_value;
    }

private:
    T* m_value;
    LifetimeObserver m_observer;

    void onDeath() {
        m_value = nullptr;
    }
};