#pragma once

#include <deque>
#include <set>
#include <memory>

#include "state.h"
#include "ref_ptr.h"
#include "lifetimeobserver.h"

class RenderCommand;
class DrawCommand;
class FutureCommand;
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

    void queue(GraphicsState const&, std::deque<ref_ptr<RenderCommand>>& output, bool finalize = true);
    void priv_addToQueue(RenderCommand* command);
    void priv_addToQueue(DrawCommand* command);
    void priv_addToQueue(FutureCommand* command);
    //void priv_addToWaitlist(RenderCommand* command);

    CommandQueue& operator=(const CommandQueue&);
    CommandQueue& operator=(CommandQueue&&);

    void replaceCommand(RenderCommand* what, RenderCommand* with);

    typedef std::deque<std::shared_ptr<RenderCommand>> queue_type;
    typedef queue_type::const_iterator iterator_type;
protected:
    //std::deque<std::shared_ptr<RenderCommand>> m_queue, m_waitlist;
    std::set<ref_ptr<RenderCommand>> m_commands;
    std::set<life_ptr<DrawCommand>> m_drawCommands;
    std::deque<life_ptr<FutureCommand>> m_futureProxies;
};

class RenderCommand : protected LifetimeObservable, protected InnerRefcounter
{
public:
    enum CommandType {
        PROGRAMCHANGE, UNIFORMCHANGE, DRAWELEMENTSDRAW
    };
    virtual ~RenderCommand() = default;

    virtual void apply(GraphicsState& theState) = 0;
    virtual void addToQueue(const GraphicsState&, CommandQueue& queue, std::deque<RenderCommand*>& addedCommands)
    {
        queue.priv_addToQueue(this);
    }

    void serialize(const GraphicsState&, std::deque<ref_ptr<RenderCommand>>& output) const;
    void swapDependencies(RenderCommand* with);

    virtual bool isDraw() const {
        return false;
    }
    virtual bool isFuture() const {
        return false;
    }
    virtual bool isState() const {
        return false;
    }

    virtual CommandType type() const = 0;

    void clearMark()
    {
        m_marked = false;
    }

protected:
    std::map<int, life_ptr<RenderCommand>> m_dependencies;

    // pair.first = index in m_dependencies map of
    // pair.second = a dependant
    std::set<std::pair<life_ptr<RenderCommand>, int>> m_dependants;
    mutable bool m_marked = false;
};

class StateChangeCommand : public RenderCommand
{
public:
    bool isState() const override {
        return true;
    }
};

class ShaderProgram;
class Uniform;

class ProgramChangeCommand : public StateChangeCommand
{
public:
    ProgramChangeCommand(const ref_ptr<ShaderProgram>& newProgram);

    void apply(GraphicsState& theState) override;

    // also adds all the global uniform commands
    void addToQueue(const GraphicsState& theState, CommandQueue& queue, std::deque<RenderCommand*>& addedCommands) override;

    CommandType type() const override;

    //typedef TypedStateData<std::shared_ptr<ShaderProgram>> ProgramStateData;
protected:
    ref_ptr<ShaderProgram> m_program;
    std::unique_ptr<LifetimeObserver> m_dataitemSentry, m_uniformsitemSentry;
    GraphicsStateRegistryItemPtr m_currentShaderItem, m_globalUniformsItem;
};

class UniformChangeCommand : public StateChangeCommand
{
public:
    UniformChangeCommand(const ref_ptr<Uniform>& newUniform);

    void apply(GraphicsState& theState) override;
    void addToQueue(const GraphicsState& theState, CommandQueue& queue, std::deque<RenderCommand*>& addedCommands) override;

    CommandType type() const override;

protected:
    ref_ptr<Uniform>  m_uniform;
    std::unique_ptr<LifetimeObserver> m_shaderItemSentry;
    GraphicsStateRegistryItemPtr m_shaderItem;
};

class FutureCommand : public RenderCommand
{
public:
    bool isFuture() const override {
        return true;
    }
};
