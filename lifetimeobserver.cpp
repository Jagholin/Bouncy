#include "lifetimeobserver.h"

LifetimeObservable::~LifetimeObservable()
{
	for (auto const& o : m_observers) {
		o->emitDestroyed();
	}
}

void LifetimeObservable::addObserver(LifetimeObserver *o)
{
	m_observers.push_back(o);
}

void LifetimeObservable::removeObserver(LifetimeObserver *o)
{
	m_observers.erase(std::find(m_observers.begin(), m_observers.end(), o));
}
