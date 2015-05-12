#include <iostream>
#include "commandqueue.h"
#include "drawcommands.h"

CommandQueue::CommandQueue(CommandQueue&& rhs):
m_commands(std::move(rhs.m_commands)),
m_drawCommands(std::move(rhs.m_drawCommands)),
m_futureProxies(std::move(rhs.m_futureProxies))
{

}

void CommandQueue::queue(GraphicsState const& theState, std::deque<ref_ptr<RenderCommand>>& output, bool finalize /*= true*/)
{
    decltype(m_drawCommands) newDrawList;
    for (auto const& drawCommand : m_drawCommands)
    {
        if (drawCommand)
        {
            drawCommand->serialize(theState, output);
            newDrawList.insert(drawCommand);
        }
    }
    for (auto const& aCommand : m_commands)
    {
        aCommand->clearMark();
    }
    if (finalize && !m_futureProxies.empty())
    {
        std::cerr << "CommandQueue::queue still has futureProxies in finalized state\n";
    }
}

void CommandQueue::priv_addToQueue(RenderCommand* command)
{
    m_commands.insert(ref_ptr<RenderCommand>(command));
}

void CommandQueue::priv_addToQueue(DrawCommand* command)
{
    m_commands.insert(ref_ptr<RenderCommand>(command));
    m_drawCommands.insert(life_ptr<DrawCommand>(command));
}

void CommandQueue::priv_addToQueue(FutureCommand* command)
{
    m_commands.insert(ref_ptr<RenderCommand>(command));
    m_futureProxies.push_back(life_ptr<FutureCommand>(command));
}

void CommandQueue::replaceCommand(RenderCommand* what, RenderCommand* with)
{
    what->swapDependencies(with);
    m_commands.erase(ref_ptr<RenderCommand>(what));
    if (what->isDraw())
    {
        m_drawCommands.erase(life_ptr<DrawCommand>(static_cast<DrawCommand*>(what)));
    }
    if (what->isFuture())
    {
        m_futureProxies.erase(std::find(m_futureProxies.cbegin(), 
            m_futureProxies.cend(), 
            life_ptr<FutureCommand>(static_cast<FutureCommand*>(what))));
    }
}

void RenderCommand::serialize(const GraphicsState& s, std::deque<ref_ptr<RenderCommand>>& output) const
{
    if (m_marked)
        return;
    for (const auto& dependant : m_dependencies)
    {
        dependant.second->serialize(s, output);
    }
    output.push_back(ref_ptr<RenderCommand>(this));
    m_marked = true;
}

void RenderCommand::swapDependencies(RenderCommand* with)
{
    for (const auto& dep : m_dependants)
    {

    }
}
