#pragma once
#include <GL/glew.h>
#include <string>
#include <memory>
#include <glm/fwd.hpp>

class ShaderProgram;
class Uniform
{
public:
	Uniform(const std::string& name);
	virtual ~Uniform();

	virtual void apply(ShaderProgram*) = 0;
	bool overrides(const Uniform& otherUniform);

	virtual bool isEqual(const Uniform& rhs) const;

	std::string name() const;
	virtual unsigned int type() const;

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

	bool isEqual(const Uniform& rhs) const override;

	unsigned int type() const override 
	{
		return m_type;
	}

	void setValue(GlmT newVal)
	{
		m_value = std::move(newVal);
	}

protected:
	GlmT m_value;
	static const int m_type;
};

template<class GlmT>
bool GlmUniform<GlmT>::isEqual(const Uniform& rhs) const
{
	if (rhs.name() != name())
		return false;
	if (rhs.type() != type())
		return false;
	const GlmUniform<GlmT>* realRhs = static_cast<const GlmUniform<GlmT>*>(&rhs);
	return realRhs->m_value == m_value;
}

namespace detail {
	unsigned int genNextType() {
		static unsigned int currType = 0;
		return ++currType;
	}
}

template <typename T>
const int GlmUniform<T>::m_type = detail::genNextType();