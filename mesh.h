#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <map>
#include <memory>

#include "drawable.h"

//#include "buffer.h"

class GraphicsBuffer;

struct VertexData
{
	glm::vec4 position;
	glm::vec3 normal;
	glm::vec3 color;
};

class EditableMesh : public Drawable
{
public:
	EditableMesh();
	EditableMesh(const EditableMesh&) = delete;
	template<class T1, class T2>
	EditableMesh(T1&& vData, T2&& iData) : EditableMesh()
	{
		m_vertices.swap(vData);
		m_indices.swap(iData);
		updateBuffers();
	}

	virtual ~EditableMesh();

	void addTriangle(const VertexData &a, const VertexData &b, const VertexData &c);

	void updateBuffers();
	virtual void render();

	void bindVertexArray();
	static bool isIndexBufferBound(GLuint bufferId);
	static void setIndexBufferBound(GLuint bufferId);
	static void unbindIndexBuffer(GLuint bufferId);

	virtual void addToQueue(GraphicsState&, CommandQueue& out) override;

protected:

	std::vector<VertexData> m_vertices;
	std::vector<GLushort> m_indices;

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer;

	GLuint m_vertexArray;

	static GLuint s_boundVA;
	static std::map<GLuint, GLuint> s_indiciesBindingMap;
};