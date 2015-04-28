#pragma once
#include <deque>
#include <unordered_map>
#include <map>
#include <memory>
#include "statedata.h"
#include "stateelements.h"

class CommandQueue;
class GraphicsState;
class GraphicsStateSet
{
public:
	GraphicsStateSet() = default;
	GraphicsStateSet(const GraphicsStateSet&);
	GraphicsStateSet(GraphicsStateSet&&);

	template<class T>
	T* getStateElement(const std::string& name)
	{
		if (m_stateElements.count(name) == 0)
			return nullptr;
		std::shared_ptr<GraphicsStateElement> elem = m_stateElements.at(name);
		if (!elem->isA<T>())
			return nullptr;
		return static_cast<T*>(elem.get());
	}

	void addElement(const std::string &name, const std::shared_ptr<GraphicsStateElement>&);
	CommandQueue asCommandQueue(GraphicsState&) const;

	GraphicsStateSet operator+(const GraphicsStateSet&) const;
	GraphicsStateSet operator-(const GraphicsStateSet&) const;

	GraphicsStateSet& operator=(GraphicsStateSet&&);
	GraphicsStateSet& operator=(const GraphicsStateSet&);

	GraphicsStateSet& operator+=(const GraphicsStateSet&);
	GraphicsStateSet& operator-=(const GraphicsStateSet&);

protected:
	std::map<std::string, std::shared_ptr<GraphicsStateElement>> m_stateElements;
};

class GraphicsState
{
public:

	StateData* stateData(const std::string& name) const;

	void setStateData(std::string name, std::shared_ptr<StateData> stateData);
	void commit();

	friend class ApplyStateSet;

protected:
	std::deque<GraphicsStateSet> m_stateStack;
	GraphicsStateSet m_currentState;

	// Current graphics state
	std::unordered_map<std::string, std::shared_ptr<StateData>> m_stateData;
};

class ApplyStateSet
{
public:
	ApplyStateSet(GraphicsState&, GraphicsStateSet&);
	ApplyStateSet(const ApplyStateSet&) = delete;
	ApplyStateSet(ApplyStateSet&&) = delete;
	~ApplyStateSet();

protected:
	GraphicsState& m_state;
	GraphicsStateSet& m_set;
};
