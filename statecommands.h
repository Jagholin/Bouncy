#pragma once

#include <deque>
#include <memory>

#include "state.h"
#include "ref_ptr.h"

class RenderCommandOld;
class GraphicsState;
class CommandQueueOld
{
public:
	enum AddToQueueOp{
		ADD_ELEMENT, DONT_ADD_ELEMENT, REMOVE_ELEMENT
	};
	CommandQueueOld() = default;
	CommandQueueOld(const CommandQueueOld&) = delete;
	CommandQueueOld(CommandQueueOld&&);

	std::deque<std::shared_ptr<RenderCommandOld>>& queue(GraphicsState const&, bool finalize = true);
	void priv_addToQueue(std::shared_ptr<RenderCommandOld> const& command);
	void priv_addToWaitlist(std::shared_ptr<RenderCommandOld> const& command);

	CommandQueueOld& operator=(const CommandQueueOld&);
	CommandQueueOld& operator=(CommandQueueOld&&);

	typedef std::deque<std::shared_ptr<RenderCommandOld>> queue_type;
	typedef queue_type::const_iterator iterator_type;
protected:
	std::deque<std::shared_ptr<RenderCommandOld>> m_queue, m_waitlist;
};

class RenderCommandOld : protected std::enable_shared_from_this<RenderCommandOld>
{
public:
	enum CommandType {
		PROGRAMCHANGE, UNIFORMCHANGE, DRAWELEMENTSDRAW
	};
	virtual ~RenderCommandOld() = default;

	virtual void apply(GraphicsState& theState) = 0;
	virtual void addToQueue(const GraphicsState& theState, CommandQueueOld& commandQueue)
	{
		commandQueue.priv_addToQueue(shared_from_this());
	}
	virtual CommandQueueOld::AddToQueueOp canAddToQueue(CommandQueueOld::queue_type& aQueue, CommandQueueOld::iterator_type const& newElemBegin, CommandQueueOld::iterator_type const& newElemEnd);
	virtual void waitingSweep(GraphicsState const&) const;
	virtual CommandType type() const = 0;
};

class ShaderProgram;
class ProgramChangeCommandOld : public RenderCommandOld
{
public:
	ProgramChangeCommandOld(const ref_ptr<ShaderProgram>& newProgram);

	void apply(GraphicsState& theState) override;
	void addToQueue(const GraphicsState& theState, CommandQueueOld& commandQueue) override;

	CommandType type() const override;

	//typedef TypedStateData<std::shared_ptr<ShaderProgram>> ProgramStateData;
protected:
	ref_ptr<ShaderProgram> m_program;
	std::unique_ptr<LifetimeObserver> m_dataitemSentry, m_uniformsitemSentry;
	GraphicsStateRegistryItemPtr m_currentShaderItem, m_globalUniformsItem;
};

class Uniform;
class UniformChangeCommandOld : public RenderCommandOld
{
public:
	UniformChangeCommandOld(const ref_ptr<Uniform>& newUniform);

	void apply(GraphicsState& theState) override;
	void addToQueue(const GraphicsState& theState, CommandQueueOld& commandQueue) override;

	CommandType type() const override;

	virtual CommandQueueOld::AddToQueueOp canAddToQueue(CommandQueueOld::queue_type& aQueue, CommandQueueOld::iterator_type const& newElemBegin, CommandQueueOld::iterator_type const& newElemEnd) override;
	virtual void waitingSweep(GraphicsState const&) const override;

	//typedef TypedStateData<std::shared_ptr<ShaderProgram>> ProgramStateData;
protected:
	ref_ptr<Uniform>  m_uniform;
	std::unique_ptr<LifetimeObserver> m_shaderItemSentry;
	GraphicsStateRegistryItemPtr m_shaderItem;
};

