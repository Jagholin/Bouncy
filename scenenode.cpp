#include "scenenode.h"
#include "state.h"
#include "stateelements.h"
#include "scene.h"
#include <utility>
#include <algorithm>

SceneNode::SceneNode(GraphicsScene* scene, std::string const &name) : m_nodeState(new GraphicsStateSet), m_parent(nullptr), m_nodeTransform(1.0f), m_name(name), m_scene(scene)
{
	//if (parent)
	//	parent->addChild(shared_from_this());
	m_nodeState->addElement("modelTransform", std::make_shared<ModelTransformStateElement>(m_nodeTransform));
}

SceneNode::~SceneNode()
{
	if (m_parent)
		m_parent->childDestroyed(this);
	for (auto aChild : m_children)
	{
		aChild->orphan();
	}
	if (m_scene)
		m_scene->unregisterSceneNode(m_name);
}

void SceneNode::render(GraphicsState& aState, bool recursive /*= true*/)
{
	ApplyStateSet _{ aState, *m_nodeState };
	for (auto aDrawable : m_drawables)
		aDrawable->render();
	if (recursive)
	{
		for (auto aChild : m_children)
			aChild->render(aState, recursive);
	}
}

void SceneNode::setNodeTransform(const glm::mat4& newTransform)
{
	if (newTransform == m_nodeTransform)
		return;
	m_nodeTransform = newTransform;
	//m_nodeState->setModelMatrix(newTransform);
	auto stateElement = m_nodeState->getStateElement<ModelTransformStateElement>("modelTransform");
	if (stateElement)
		stateElement->setTransform(m_nodeTransform);
	else
		m_nodeState->addElement("modelTransform", std::make_shared<ModelTransformStateElement>(m_nodeTransform));
}

void SceneNode::addDrawable(const std::shared_ptr<Drawable>& aDrawable)
{
	m_drawables.push_back(aDrawable);
}

GraphicsStateSet& SceneNode::stateSet()
{
	return *m_nodeState;
}

void SceneNode::addChild(const std::shared_ptr<SceneNode>& aChild)
{
	m_children.push_back(aChild);
	aChild->resetParent(this);
}

void SceneNode::removeChild(SceneNode* aChild)
{
	aChild->orphan();
	auto found = std::find_if(m_children.begin(), m_children.end(), [aChild](const std::shared_ptr<SceneNode>& item)->bool{
		return item.get() == aChild;
	});
	if (m_children.end() != found)
		m_children.erase(found);
}

void SceneNode::orphan()
{
	m_parent = nullptr;
}

void SceneNode::resetParent(SceneNode* newParent)
{
	m_parent = newParent;
}

void SceneNode::childDestroyed(SceneNode* aChild)
{
	auto found = std::find_if(m_children.begin(), m_children.end(), [aChild](const std::shared_ptr<SceneNode>& item)->bool{
		return item.get() == aChild;
	});
	if (m_children.end() != found)
		m_children.erase(found);
}

std::string SceneNode::name()
{
	return m_name;
}

SceneNode* SceneNode::parent() const
{
	return m_parent;
}

glm::mat4 SceneNode::worldTransform() const
{
	if (m_parent)
		return m_nodeTransform * m_parent->worldTransform();
	return m_nodeTransform;
}
