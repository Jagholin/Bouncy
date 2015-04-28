#include <GL/glew.h>
#include <iostream>
#include "buffer.h"
#include "mesh.h"
#include "glerror.h"

GraphicsBuffer::GraphicsBuffer() : m_bufferId(0), m_sealedBindingTarget(false)
{
	check_gl_error();
	glGenBuffers(1, &m_bufferId);
	check_gl_error();
}

GraphicsBuffer::GraphicsBuffer(GraphicsBuffer&& rhs) : m_bufferId(rhs.m_bufferId), m_binding(rhs.m_binding), m_sealedBindingTarget(rhs.m_sealedBindingTarget)
{
	rhs.m_bufferId = 0;
	rhs.m_sealedBindingTarget = false;
}

GraphicsBuffer::~GraphicsBuffer()
{
	check_gl_error();
	if (m_bufferId > 0 && s_bindings.count(m_binding) > 0 && m_bufferId == s_bindings.at(m_binding))
	{
		s_bindings[m_binding] = 0;
		glBindBuffer(getGlBindingTarget(m_binding), 0);
		check_gl_error();
	}
	if (m_bufferId > 0)
	{
		glDeleteBuffers(1, &m_bufferId);
		check_gl_error();
	}
}

GLenum GraphicsBuffer::getGlBindingTarget(BindingTarget target)
{
	static const std::map<BindingTarget, GLenum> BindingTargetsMap = { 
			{ BindingTarget::ARRAY, GL_ARRAY_BUFFER }, 
			{ BindingTarget::ELEMENT_ARRAY, GL_ELEMENT_ARRAY_BUFFER } 
	};

	if (BindingTargetsMap.count(target) > 0)
		return BindingTargetsMap.at(target);
	
	return -1;
}

void GraphicsBuffer::bind(BindingTarget target)
{
	check_gl_error();
	if (m_sealedBindingTarget && target != m_binding)
	{
		std::cerr << "Buffer binding point redefinition" << std::endl;
		return;
	}

	// If it is already bound, don't rebind

	if (0 == m_bufferId || isBound())
		return;

	m_binding = target;
	glBindBuffer(getGlBindingTarget(m_binding), m_bufferId);
	check_gl_error();

	if (m_binding == ELEMENT_ARRAY)
		EditableMesh::setIndexBufferBound(m_bufferId);
	else
		s_bindings[m_binding] = m_bufferId;
	m_sealedBindingTarget = true;
}

void GraphicsBuffer::bind()
{
	check_gl_error();
	if (!m_sealedBindingTarget)
		return;

	// If it is already bound, don't rebind
	if (m_bufferId == s_bindings[m_binding])
		return;

	if (m_bufferId == 0)
		return;

	glBindBuffer(getGlBindingTarget(m_binding), m_bufferId);
	check_gl_error();
	if (m_binding == ELEMENT_ARRAY)
		EditableMesh::setIndexBufferBound(m_bufferId);
	else
		s_bindings[m_binding] = m_bufferId;
}

bool GraphicsBuffer::isBound()
{
	if (!m_sealedBindingTarget || 0 == m_bufferId)
		return false;

	if (ELEMENT_ARRAY == m_binding)
	{
		// Special case: ELEMENT_ARRAY is contained within VAO.
		return EditableMesh::isIndexBufferBound(m_bufferId);
	}

	return m_bufferId == s_bindings[m_binding];
}

void GraphicsBuffer::unbind()
{
	check_gl_error();
	if (m_sealedBindingTarget && s_bindings[m_binding] == m_bufferId)
	{
		glBindBuffer(getGlBindingTarget(m_binding), m_bufferId);
		check_gl_error();
		s_bindings[m_binding] = 0;
	}
}

void GraphicsBuffer::resetData(size_t size, const void* data, GLenum usage /*= GL_STATIC_DRAW*/)
{
	check_gl_error();
	// Check if the current buffer was bound
	if (!m_sealedBindingTarget)
	{
		std::cerr << "Can not assign data to an unbound buffer";
		return;
	}

	if (m_bufferId == 0)
		return;

	bind();
	glBufferData(getGlBindingTarget(m_binding), size, data, usage);
	check_gl_error();
}

std::map<GraphicsBuffer::BindingTarget, GLuint> GraphicsBuffer::s_bindings;