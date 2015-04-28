#pragma once

#include <map>

class GraphicsBuffer
{
public:
	GraphicsBuffer();
	GraphicsBuffer(const GraphicsBuffer&) = delete;
	GraphicsBuffer(GraphicsBuffer&&);

	~GraphicsBuffer();

	enum BindingTarget {
		ARRAY, ELEMENT_ARRAY
	};

	static GLenum getGlBindingTarget(BindingTarget);

	void bind(BindingTarget target);
	void bind();
	bool isBound();

	void unbind();

	void resetData(size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);

protected:
	GLuint m_bufferId;
	BindingTarget m_binding;
	bool m_sealedBindingTarget;

	static std::map<BindingTarget, GLuint> s_bindings;
};