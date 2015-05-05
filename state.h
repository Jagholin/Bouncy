#pragma once
#include <deque>
#include <unordered_map>
#include <map>
#include <memory>
#include "registry.h"
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

typedef Registry<std::shared_ptr<ShaderProgram>, std::shared_ptr<Uniform>, std::shared_ptr<GlmUniform<glm::mat4>>> GraphicsStateRegistry;
typedef GraphicsStateRegistry::itemptr_type GraphicsStateRegistryItemPtr;

class GraphicsState
{
public:
	//GraphicsStateRegistryItemPtr stateData(const std::string& name) const;

	//void setStateData(std::string name, std::shared_ptr<RegistryDataItem> stateData);
	GraphicsStateRegistry& stateData();
	const GraphicsStateRegistry& stateData() const;
	void commit();

	friend class ApplyStateSet;

protected:
	std::deque<GraphicsStateSet> m_stateStack;
	GraphicsStateSet m_currentState;

	// Current graphics state
	//std::unordered_map<std::string, std::shared_ptr<RegistryDataItem>> m_stateData;
	GraphicsStateRegistry m_stateData;
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
