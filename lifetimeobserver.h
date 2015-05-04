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

struct LifetimeObserver {
	LifetimeObserver(LifetimeObservable* observ, std::function<void()> const destrEvent):
	m_observable(observ),
	m_destrEvent(destrEvent){
		m_observable->addObserver(this);
	}
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