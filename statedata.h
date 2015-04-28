#pragma once

#include <GL/glew.h>
#include <unordered_map>
#include <string>
#include <memory>
#include "uniform.h"

class StateData
{
public:
	std::unordered_map<std::string, std::shared_ptr<StateData>> childData;
	virtual ~StateData() = default;

	template <class S>
	bool isA()
	{
		return typeid(*this) == typeid(S);
	}

	virtual bool isUniformData() const{
		return false;
	}

	virtual std::shared_ptr<Uniform> asUniform(std::string const&)
	{
		return std::shared_ptr < Uniform > {nullptr};
	}
};

template <class T>
class TypedStateData : public StateData
{
public:
	TypedStateData() = default;
	TypedStateData(const T& data_):
		data(data_)
	{
		// no op
	}
	T data;
};

template <class T>
class UniformStateData : public TypedStateData < T >
{
public:
	UniformStateData() = default;
	UniformStateData(const T& data_):
		TypedStateData<T>(data_)
	{

	}

	std::shared_ptr<Uniform> asUniform(std::string const &name) override
	{
		return std::make_shared<GlmUniform<T>>(name, data);
	}

	bool isUniformData() const override{
		return true;
	}
};

typedef TypedStateData<GLuint> GLObjectStateData;