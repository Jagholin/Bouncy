#pragma once
#include <GL/glew.h>
#include <string>
#include <memory>
#include <glm/fwd.hpp>

class StateData;
class ShaderProgram;
class Uniform
{
public:
	Uniform(const std::string& name);
	virtual ~Uniform();

	virtual void apply(ShaderProgram*) = 0;
	bool overrides(const Uniform& otherUniform);

	virtual bool checkData(std::shared_ptr<StateData> const &data) = 0;
	virtual std::shared_ptr<StateData> createStateData() = 0;

	std::string name() const;

protected:
	std::string m_name;
	bool m_noUniform;

	GLint pre_apply_impl(ShaderProgram*);

	void apply_impl(ShaderProgram*, glm::vec2);
	void apply_impl(ShaderProgram*, glm::vec3);
	void apply_impl(ShaderProgram*, float);
	void apply_impl(ShaderProgram*, int);
	void apply_impl(ShaderProgram*, glm::mat4);
};

template<class GlmT>
class GlmUniform : public Uniform
{
public:
	GlmUniform(const std::string& name, GlmT value) : Uniform(name),
		m_value(value)
	{

	}

	virtual void apply(ShaderProgram* prog) override
	{
		apply_impl(prog, m_value);
	}

	virtual bool checkData(std::shared_ptr<StateData> const &data) override
	{
		if (!data->isA<UniformStateData<GlmT>>())
			return false;
		const GlmT rvalue = static_cast<UniformStateData<GlmT>*>(data.get())->data;
		return rvalue == m_value;
	}

	virtual std::shared_ptr<StateData> createStateData() override
	{
		return std::make_shared<UniformStateData<GlmT>>(m_value);
	}

protected:
	GlmT m_value;
};