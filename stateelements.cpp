#include "stateelements.h"
#include "uniform.h"
#include "statecommands.h"

std::shared_ptr<GraphicsStateElement> GraphicsStateElement::combineWith(const std::shared_ptr<GraphicsStateElement>&)
{
	// do essentially nothing - current settings override those from rhs
	return shared_from_this();
}

std::shared_ptr<GraphicsStateElement> GraphicsStateElement::subtract(const std::shared_ptr<GraphicsStateElement>&)
{
	// standard implementation is to do nothing
	return shared_from_this();
}

std::shared_ptr<GraphicsStateElement> GraphicsStateElement::inverse() const
{
	return std::shared_ptr<GraphicsStateElement> {};
}

std::shared_ptr<GraphicsStateElement> operator+=(std::shared_ptr<GraphicsStateElement> lhs, const std::shared_ptr<GraphicsStateElement>& rhs)
{
	return lhs->combineWith(rhs);
}

std::shared_ptr<GraphicsStateElement> operator-=(std::shared_ptr<GraphicsStateElement> lhs, const std::shared_ptr<GraphicsStateElement>& rhs)
{
	return lhs->subtract(rhs);
}

ModelTransformStateElement::ModelTransformStateElement(glm::mat4 transform):
m_transform(transform)
{

}

void ModelTransformStateElement::setTransform(glm::mat4 newMatrix)
{
	m_transform = newMatrix;
}

void ModelTransformStateElement::apply(GraphicsState& theState, CommandQueue& commQueue)
{
	std::make_shared<UniformChangeCommand>(std::make_shared<GlmUniform<glm::mat4>>("modelMatrix", m_transform))->addToQueue(theState, commQueue);
}

std::shared_ptr<GraphicsStateElement> ModelTransformStateElement::combineWith(const std::shared_ptr<GraphicsStateElement>& rhs)
{
	std::shared_ptr<ModelTransformStateElement> realRhs = std::static_pointer_cast<ModelTransformStateElement>(rhs);
	m_transform *= realRhs->m_transform;
	return shared_from_this();
}

std::shared_ptr<GraphicsStateElement> ModelTransformStateElement::inverse() const
{
	return std::make_shared<ModelTransformStateElement>(glm::mat4{});
}

std::shared_ptr<GraphicsStateElement> ModelTransformStateElement::copy() const
{
	return std::make_shared<ModelTransformStateElement>(*this);
}

ProgramStateElement::ProgramStateElement(std::shared_ptr<ShaderProgram> const& program) : m_program(program)
{

}

void ProgramStateElement::apply(GraphicsState& theState, CommandQueue& commQueue)
{
	std::make_shared<ProgramChangeCommand>(m_program)->addToQueue(theState, commQueue);
}

std::shared_ptr<GraphicsStateElement> ProgramStateElement::copy() const
{
	return std::make_shared<ProgramStateElement>(*this);
}
