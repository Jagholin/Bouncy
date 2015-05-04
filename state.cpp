#include "state.h"
#include "statecommands.h"

GraphicsStateSet::GraphicsStateSet(GraphicsStateSet&& rhs) : m_stateElements(std::move(rhs.m_stateElements))
{
	
}

GraphicsStateSet::GraphicsStateSet(const GraphicsStateSet& rhs)
{
	for (auto const& elem : rhs.m_stateElements)
	{
		m_stateElements[elem.first] = elem.second->copy();
	}
}

void GraphicsStateSet::addElement(const std::string &name, const std::shared_ptr<GraphicsStateElement>& element)
{
	m_stateElements[name] = element;
}

CommandQueue GraphicsStateSet::asCommandQueue(GraphicsState& state) const
{
	CommandQueue result{};
	for (auto stateElement : m_stateElements)
	{
		stateElement.second->apply(state, result);
	}
	return result;
}

GraphicsStateSet GraphicsStateSet::operator+(const GraphicsStateSet& rhs) const
{
	return GraphicsStateSet{ *this } += rhs;
}

GraphicsStateSet GraphicsStateSet::operator-(const GraphicsStateSet& rhs) const
{
	return GraphicsStateSet{ *this } -= rhs;
}

GraphicsStateSet& GraphicsStateSet::operator-=(const GraphicsStateSet& rhs)
{
	for (auto stateElement : rhs.m_stateElements)
	{
		if (m_stateElements.count(stateElement.first) == 0)
		{
			auto inverseElement = stateElement.second->inverse();
			if (inverseElement)
				m_stateElements[stateElement.first] = inverseElement;
			continue;
		}
		m_stateElements[stateElement.first] -= stateElement.second;
	}
	return *this;
}

GraphicsStateSet& GraphicsStateSet::operator+=(const GraphicsStateSet& rhs)
{
	for (auto stateElement : rhs.m_stateElements)
	{
		if (m_stateElements.count(stateElement.first) == 0)
		{
			m_stateElements[stateElement.first] = stateElement.second->copy();
			continue;
		}
		m_stateElements[stateElement.first] += stateElement.second;
	}
	return *this;
}

GraphicsStateSet& GraphicsStateSet::operator=(const GraphicsStateSet& rhs)
{
	if (&rhs == this)
		return *this;
	//m_stateElements = rhs.m_stateElements;
	for (auto const& elem : rhs.m_stateElements)
	{
		m_stateElements[elem.first] = elem.second->copy();
	}
	return *this;
}

GraphicsStateSet& GraphicsStateSet::operator=(GraphicsStateSet&& rhs)
{
	if (&rhs == this)
		return *this;
	m_stateElements = std::move(rhs.m_stateElements);
	return *this;
}

GraphicsStateRegistry & GraphicsState::stateData()
{
	return m_stateData;
}

void GraphicsState::commit()
{
	// Compile compound StateSet from stack
	GraphicsStateSet newSet;
	for (auto aSet : m_stateStack)
	{
		newSet += aSet;
	}
	GraphicsStateSet diff = newSet - m_currentState;
	CommandQueue myCommands{ diff.asCommandQueue(*this) };
	auto commandQueue = myCommands.queue(*this);
	for (auto aCommand : commandQueue)
	{
		aCommand->apply(*this);
	}
	m_currentState = newSet;
}

ApplyStateSet::ApplyStateSet(GraphicsState& theState, GraphicsStateSet& aSet) :
m_state(theState),
m_set(aSet)
{
	theState.m_stateStack.push_back(aSet);
	theState.commit();
}

ApplyStateSet::~ApplyStateSet()
{
	m_state.m_stateStack.pop_back();
	m_state.commit();
}
