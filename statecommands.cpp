#include "statecommands.h"
#include "state.h"
#include "shader.h"
#include "uniform.h"
#include <iostream>

CommandQueue::CommandQueue(CommandQueue&& rhs) : 
m_queue(std::move(rhs.m_queue)), 
m_waitlist(std::move(rhs.m_waitlist))
{

}

std::deque<std::shared_ptr<StateChangeCommand>>& CommandQueue::queue(GraphicsState const& theState, bool finalize)
{
	if (!m_waitlist.empty() && finalize)
	{
		for (auto waitingElement : m_waitlist)
		{
			waitingElement->waitingSweep(theState);
			m_queue.push_back(waitingElement);
		}
		// Hello hello hello
	}
	return m_queue;
}

void CommandQueue::priv_addToQueue(std::shared_ptr<StateChangeCommand> const& command)
{
	m_queue.push_back(command);
	auto lastElementPointer = m_queue.cend();
	--lastElementPointer;
	bool elementAdded = false;
	std::deque<std::shared_ptr<StateChangeCommand>> newWaitList;
	do {
		elementAdded = false;
		for (auto waitingElement : m_waitlist)
		{
			AddToQueueOp op = waitingElement->canAddToQueue(m_queue, lastElementPointer, m_queue.cend());
			if (ADD_ELEMENT == op)
			{
				m_queue.push_back(waitingElement);
				elementAdded = true;
			}
			else if (DONT_ADD_ELEMENT == op)
			{
				newWaitList.push_back(waitingElement);
			} // else REMOVE_ELEMENT == op don't need to do anything in this case
		}
		m_waitlist.swap(newWaitList);
		newWaitList.clear();
	} while (elementAdded);
}

void CommandQueue::priv_addToWaitlist(std::shared_ptr<StateChangeCommand> const& command)
{
	m_waitlist.push_back(command);
}

CommandQueue::AddToQueueOp StateChangeCommand::canAddToQueue(CommandQueue::queue_type& aQueue, CommandQueue::iterator_type const& newElemBegin, CommandQueue::iterator_type const& newElemEnd)
{
	return CommandQueue::ADD_ELEMENT;
}

void StateChangeCommand::waitingSweep(GraphicsState const&) const
{

}

ProgramChangeCommand::ProgramChangeCommand(const ref_ptr<ShaderProgram>& newProgram) :
m_program(newProgram), m_currentShaderItem(nullptr), m_globalUniformsItem(nullptr)
{
	// no-op
}

void ProgramChangeCommand::apply(GraphicsState& theState)
{
	//RegistryDataItem* myData = theState.stateData("shaderProgram");
	if (!m_currentShaderItem)
	{
		GraphicsStateRegistry& stateRegistry = theState.stateData();
		m_currentShaderItem = stateRegistry.item_at("/shaderProgram");
		if (!m_currentShaderItem || !m_currentShaderItem->isValueOf<ShaderProgram>())
		{
			if (m_currentShaderItem)
			{
				// !myData->isA<GLObjectStateData>()
				std::cerr << "myData is not a <ShaderProgram>, line " << __LINE__ << " file " << __FILE__ << std::endl;
			}
			// No current program applied, apply this one
			m_program->use();
			//theState.setStateData("shaderProgram", std::make_shared<ProgramStateData>(m_program));
			m_currentShaderItem = stateRegistry.createItemAt("/", "shaderProgram", m_program);
		}
		m_dataitemSentry.reset(new LifetimeObserver(m_currentShaderItem, [this]() {
			m_currentShaderItem = nullptr;
		}));
	}

	//ProgramStateData* realData = static_cast<ProgramStateData*>(myData);
	auto realData = m_currentShaderItem->as<ref_ptr<ShaderProgram>>();
	if (realData == m_program)
		return;
	m_program->use();
	m_currentShaderItem->setValue(m_program);
}

void ProgramChangeCommand::addToQueue(const GraphicsState& theState, CommandQueue& commandQueue)
{
	for (auto & aCommand : commandQueue.queue(theState, false))
	{
		if (aCommand->type() == StateChangeCommand::PROGRAMCHANGE)
		{
			ProgramChangeCommand* realCommand = static_cast<ProgramChangeCommand*>(aCommand.get());
			realCommand->m_program = m_program;
			return;
		}
	}
	commandQueue.priv_addToQueue(shared_from_this());

	// Add all global uniforms to the queue as well
	// test text oinput
	if (!m_globalUniformsItem)
	{
		auto& registry = theState.stateData();
		m_globalUniformsItem = registry.item_at("/globalUniforms");
		if (!m_globalUniformsItem)
		{
			//m_globalUniformsItem = registry.createItemAt("/", "globalUniforms", )
			return;
		}
		m_uniformsitemSentry.reset(new LifetimeObserver{ m_globalUniformsItem, [this]() {
			m_globalUniformsItem = nullptr;
		}});
	}
	//RegistryDataItem* uniformData = theState.stateData("globalUniforms");
	for (auto anUniformIter = m_globalUniformsItem->cbegin(); anUniformIter != m_globalUniformsItem->cend(); ++anUniformIter)
	{
		//if (!anUniform.second->isUniformData())
		//	continue;
		if (!anUniformIter->second->isValueOf<ref_ptr<Uniform>>())
			continue;
		ref_ptr<Uniform> realData = anUniformIter->second->as<ref_ptr<Uniform>>();
		std::make_shared<UniformChangeCommand>(realData)->addToQueue(theState, commandQueue);
	}
}

StateChangeCommand::CommandType ProgramChangeCommand::type() const
{
	return StateChangeCommand::PROGRAMCHANGE;
}

UniformChangeCommand::UniformChangeCommand(const ref_ptr<Uniform>& newUniform):
m_uniform(newUniform), m_shaderItem(nullptr)
{

}

void UniformChangeCommand::apply(GraphicsState& theState)
{
	//RegistryDataItem* programData = theState.stateData("shaderProgram");
	if (!m_shaderItem)
	{
		m_shaderItem = theState.stateData().item_at("/shaderProgram");
		if (m_shaderItem)
		{
			m_shaderItemSentry.reset(new LifetimeObserver{ m_shaderItem, [this]() {
				m_shaderItem = nullptr;
			} });
		}
	}
	if (!m_shaderItem || !m_shaderItem->isValueOf<ref_ptr<ShaderProgram>>())
	{
		std::cerr << "Cannot retrieve current shader program from GraphicsState, line " << __LINE__ << " file " << __FILE__ << std::endl;
		return;
	}
	std::string uniformName = m_uniform->name();
	GraphicsStateRegistryItemPtr uniformData = theState.stateData().item_at(uniformName, m_shaderItem);
	if (uniformData == nullptr || ! m_uniform->isEqual(* uniformData->as<ref_ptr<Uniform>>().get()))
	{
		// program doesn't have a uniform yet or it is not set to the required value
		//ProgramStateData* realData = static_cast<ProgramStateData*>(programData);
		m_uniform->apply(m_shaderItem->as<ref_ptr<ShaderProgram>>().get());
		//realData->childData[m_uniform->name()] = m_uniform->createStateData();
		if (uniformData)
			uniformData->setValue(m_uniform);
		else
		{
			theState.stateData().createItemAt("/shaderProgram", uniformName, m_uniform);
		}
	}
}

void UniformChangeCommand::addToQueue(const GraphicsState& theState, CommandQueue& commandQueue)
{
	// Try to find a program change command
	bool programFound = false;
	for (auto aCommand : commandQueue.queue(theState, false))
	{
		if (aCommand->type() == StateChangeCommand::PROGRAMCHANGE)
		{
			programFound = true;
		}
		else if (aCommand->type() == StateChangeCommand::UNIFORMCHANGE)
		{
			UniformChangeCommand* realCommand = static_cast<UniformChangeCommand*>(aCommand.get());
			if (m_uniform->overrides(*realCommand->m_uniform))
			{
				realCommand->m_uniform = m_uniform;
				return;
			}
		}
	}
	if (programFound)
		commandQueue.priv_addToQueue(shared_from_this());
	else
		commandQueue.priv_addToWaitlist(shared_from_this());
	return;
}

StateChangeCommand::CommandType UniformChangeCommand::type() const
{
	return StateChangeCommand::UNIFORMCHANGE;
}

CommandQueue::AddToQueueOp UniformChangeCommand::canAddToQueue(CommandQueue::queue_type& aQueue, CommandQueue::iterator_type const& newElemBegin, CommandQueue::iterator_type const& newElemEnd)
{
	CommandQueue::iterator_type currentElem = newElemBegin;
	bool foundProgram = false;
	for (; currentElem != newElemEnd; ++currentElem)
	{
		if ((*currentElem)->type() == StateChangeCommand::PROGRAMCHANGE)
		{
			foundProgram = true;
		}
		else if ((*currentElem)->type() == StateChangeCommand::UNIFORMCHANGE)
		{
			UniformChangeCommand* realCommand = static_cast<UniformChangeCommand*>(currentElem->get());
			if (m_uniform->overrides(*realCommand->m_uniform))
			{
				realCommand->m_uniform = m_uniform;
				return CommandQueue::REMOVE_ELEMENT;
			}
		}
	}
	if (foundProgram)
		return CommandQueue::ADD_ELEMENT;
	else
		return CommandQueue::DONT_ADD_ELEMENT;
}

void UniformChangeCommand::waitingSweep(GraphicsState const& theState) const
{
	//RegistryDataItem* currProgram = theState.stateData("shaderProgram");
	//if (!currProgram || !currProgram->isA<ProgramStateData>())
	if (! theState.stateData().item_at("/shaderProgram"))
	{
		std::cerr << "Applying uniform with no program chosen\n";
	}
}
