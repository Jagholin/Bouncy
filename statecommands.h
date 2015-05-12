#pragma once

#include <deque>
#include <memory>

#include "state.h"
#include "ref_ptr.h"

class RenderCommand;
class GraphicsState;
class CommandQueue
{
public:
	enum AddToQueueOp{
		ADD_ELEMENT, DONT_ADD_ELEMENT, REMOVE_ELEMENT
	};
	CommandQueue() = default;
	CommandQueue(const CommandQueue&) = delete;
	CommandQueue(CommandQueue&&);

	std::deque<std::shared_ptr<RenderCommand>>& queue(GraphicsState const&, bool finalize = true);
	void priv_addToQueue(std::shared_ptr<RenderCommand> const& command);
	void priv_addToWaitlist(std::shared_ptr<RenderCommand> const& command);

	CommandQueue& operator=(const CommandQueue&);
	CommandQueue& operator=(CommandQueue&&);

	typedef std::deque<std::shared_ptr<RenderCommand>> queue_type;
	typedef queue_type::const_iterator iterator_type;
protected:
	std::deque<std::shared_ptr<RenderCommand>> m_queue, m_waitlist;
};

class RenderCommand : protected std::enable_shared_from_this<RenderCommand>
{
public:
	enum CommandType {
		PROGRAMCHANGE, UNIFORMCHANGE, DRAWELEMENTSDRAW
	};
	virtual ~RenderCommand() = default;

	virtual void apply(GraphicsState& theState) = 0;
	virtual void addToQueue(const GraphicsState& theState, CommandQueue& commandQueue)
	{
		commandQueue.priv_addToQueue(shared_from_this());
	}
	virtual CommandQueue::AddToQueueOp canAddToQueue(CommandQueue::queue_type& aQueue, CommandQueue::iterator_type const& newElemBegin, CommandQueue::iterator_type const& newElemEnd);
	virtual void waitingSweep(GraphicsState const&) const;
	virtual CommandType type() const = 0;
};

class ShaderProgram;
class ProgramChangeCommand : public RenderCommand
{
public:
	ProgramChangeCommand(const ref_ptr<ShaderProgram>& newProgram);

	void apply(GraphicsState& theState) override;
	void addToQueue(const GraphicsState& theState, CommandQueue& commandQueue) override;

	CommandType type() const override;

	//typedef TypedStateData<std::shared_ptr<ShaderProgram>> ProgramStateData;
protected:
	ref_ptr<ShaderProgram> m_program;
	std::unique_ptr<LifetimeObserver> m_dataitemSentry, m_uniformsitemSentry;
	GraphicsStateRegistryItemPtr m_currentShaderItem, m_globalUniformsItem;
};

class Uniform;
class UniformChangeCommand : public RenderCommand
{
public:
	UniformChangeCommand(const ref_ptr<Uniform>& newUniform);

	void apply(GraphicsState& theState) override;
	void addToQueue(const GraphicsState& theState, CommandQueue& commandQueue) override;

	CommandType type() const override;

	virtual CommandQueue::AddToQueueOp canAddToQueue(CommandQueue::queue_type& aQueue, CommandQueue::iterator_type const& newElemBegin, CommandQueue::iterator_type const& newElemEnd) override;
	virtual void waitingSweep(GraphicsState const&) const override;

	//typedef TypedStateData<std::shared_ptr<ShaderProgram>> ProgramStateData;
protected:
	ref_ptr<Uniform>  m_uniform;
	std::unique_ptr<LifetimeObserver> m_shaderItemSentry;
	GraphicsStateRegistryItemPtr m_shaderItem;
};

