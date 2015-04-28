#include <glm/gtc/epsilon.hpp>
#include "mesh.h"
#include "buffer.h"
#include "glerror.h"

EditableMesh::EditableMesh() : m_vertexBuffer(new GraphicsBuffer), m_indexBuffer(new GraphicsBuffer)
{
	check_gl_error();

	glGenVertexArrays(1, &m_vertexArray);
	check_gl_error();
	glBindVertexArray(m_vertexArray);
	check_gl_error();
	s_boundVA = m_vertexArray;

	m_vertexBuffer->bind(GraphicsBuffer::ARRAY);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void const*>(offsetof(VertexData, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void const*>(offsetof(VertexData, normal)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void const*>(offsetof(VertexData, color)));

	m_indexBuffer->bind(GraphicsBuffer::ELEMENT_ARRAY);
}

EditableMesh::~EditableMesh()
{
	check_gl_error();
	if (s_boundVA == m_vertexArray)
	{
		glBindVertexArray(0);
		check_gl_error();
		s_boundVA = 0;
	}
	glDeleteVertexArrays(1, &m_vertexArray);
	check_gl_error();
	while (s_indiciesBindingMap.count(m_vertexArray) > 0)
	{
		s_indiciesBindingMap.erase(m_vertexArray);
	}
}

void EditableMesh::addTriangle(const VertexData &a, const VertexData &b, const VertexData &c)
{
	const VertexData* const vertices[] = { &a, &b, &c };

	for (const VertexData* vertex : vertices)
	{
		// Try to find this vertex among already existing ones
		GLushort index = 0;
		bool found = false;

		for (const VertexData& otherVertex : m_vertices)
		{
			if (glm::all(glm::epsilonEqual(otherVertex.position, vertex->position, 0.001f)) &&
				glm::all(glm::epsilonEqual(otherVertex.normal, vertex->normal, 0.01f)) &&
				glm::all(glm::epsilonEqual(otherVertex.color, vertex->color, 0.01f)))
			{
				found = true;
				break;
			}
			++index;
		}

		if (!found)
		{
			m_vertices.push_back(*vertex);
			index = static_cast<GLushort>(m_vertices.size() - 1);
		}

		m_indices.push_back(index);
	}
}

void EditableMesh::updateBuffers()
{
	bindVertexArray();
	m_vertexBuffer->resetData(m_vertices.size() * sizeof(VertexData), m_vertices.data());
	m_indexBuffer->resetData(m_indices.size() * sizeof(GLushort), m_indices.data());
}


void EditableMesh::render()
{
	bindVertexArray();
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_SHORT, 0);
	check_gl_error();
}

void EditableMesh::bindVertexArray()
{
	check_gl_error();
	if (s_boundVA != m_vertexArray)
	{
		glBindVertexArray(m_vertexArray);
		check_gl_error();
		s_boundVA = m_vertexArray;
	}
}

bool EditableMesh::isIndexBufferBound(GLuint bufferId)
{
	if (0 == s_boundVA)
		return false;
	if (s_indiciesBindingMap.count(s_boundVA) == 0)
		return false;
	return s_indiciesBindingMap.at(s_boundVA) == bufferId;
}

void EditableMesh::setIndexBufferBound(GLuint bufferId)
{
	if (0 == s_boundVA)
		return;
	s_indiciesBindingMap[s_boundVA] = bufferId;
}

void EditableMesh::unbindIndexBuffer(GLuint bufferId)
{
	check_gl_error();
	for (auto& indexPair : s_indiciesBindingMap)
	{
		if (indexPair.second == bufferId)
		{
			glBindVertexArray(indexPair.first);
			check_gl_error();
			s_boundVA = indexPair.first;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			check_gl_error();
			indexPair.second = 0;
		}
	}
}

GLuint EditableMesh::s_boundVA = 0;
std::map<GLuint, GLuint> EditableMesh::s_indiciesBindingMap;