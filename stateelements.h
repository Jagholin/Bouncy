#pragma once
#include <memory>
#include <glm/mat4x4.hpp>

class CommandQueue;
class GraphicsState;

class GraphicsStateElement : protected std::enable_shared_from_this<GraphicsStateElement>
{
public:
	virtual ~GraphicsStateElement() = default;

	template <class T>
	bool isA() const{
		return typeid(this).hash_code() == typeid(T).hash_code();
	}

	virtual void apply(GraphicsState& theState, CommandQueue& commQueue) = 0;
	virtual std::shared_ptr<GraphicsStateElement> copy() const = 0;

	virtual std::shared_ptr<GraphicsStateElement> combineWith(const std::shared_ptr<GraphicsStateElement>&);
	virtual std::shared_ptr<GraphicsStateElement> subtract(const std::shared_ptr<GraphicsStateElement>&);
	virtual std::shared_ptr<GraphicsStateElement> inverse() const;

	friend std::shared_ptr<GraphicsStateElement> operator+=(std::shared_ptr<GraphicsStateElement>, const std::shared_ptr<GraphicsStateElement>&);
	friend std::shared_ptr<GraphicsStateElement> operator-=(std::shared_ptr<GraphicsStateElement>, const std::shared_ptr<GraphicsStateElement>&);
};

class ModelTransformStateElement : public GraphicsStateElement
{
public:
	ModelTransformStateElement(glm::mat4 transform);

	void setTransform(glm::mat4 newMatrix);

	virtual void apply(GraphicsState& theState, CommandQueue& commQueue) override;
	virtual std::shared_ptr<GraphicsStateElement> combineWith(const std::shared_ptr<GraphicsStateElement>&) override;
	virtual std::shared_ptr<GraphicsStateElement> inverse() const override;

	virtual std::shared_ptr<GraphicsStateElement> copy() const override;

protected:
	glm::mat4 m_transform;
};

class ShaderProgram;

class ProgramStateElement : public GraphicsStateElement
{
public:
	ProgramStateElement(std::shared_ptr<ShaderProgram> const& program);

	virtual void apply(GraphicsState& theState, CommandQueue& commQueue) override;

	virtual std::shared_ptr<GraphicsStateElement> copy() const override;

protected:
	std::shared_ptr<ShaderProgram> m_program;
};