#pragma once

#include <glm/mat4x4.hpp>
#include <memory>
#include <deque>

#include "drawable.h"

class GraphicsStateSet;
class GraphicsState;
class GraphicsScene;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
	virtual ~SceneNode();
	SceneNode(const SceneNode&) = delete;
	SceneNode(SceneNode&&) = delete;

	template <typename T>
	static std::shared_ptr<T> createSceneNode(GraphicsScene *scene, std::string const& name)
	{
		auto res = std::shared_ptr < T > {new T{ scene, name }};
		scene->registerSceneNode(name, res);
		return res;
	}

	SceneNode& operator=(const SceneNode&)=delete;
	SceneNode& operator=(SceneNode&&)=delete;

	void render(GraphicsState& aState, bool recursive = true);
	void setNodeTransform(const glm::mat4& newTransform);

	void addDrawable(const std::shared_ptr<Drawable>& aDrawable);
	GraphicsStateSet& stateSet();

	void addChild(const std::shared_ptr<SceneNode>& aChild);
	void removeChild(SceneNode* aChild);

	SceneNode* parent() const;

	glm::mat4 worldTransform() const;

	std::string name();
protected:
	glm::mat4 m_nodeTransform;
	std::shared_ptr<GraphicsStateSet> m_nodeState;
	std::deque<std::shared_ptr<Drawable>> m_drawables;
	std::deque<std::shared_ptr<SceneNode>> m_children;
	SceneNode* m_parent;
	std::string m_name;
	GraphicsScene* m_scene;

	void orphan();
	void resetParent(SceneNode* newParent);
	void childDestroyed(SceneNode* aChild);

	SceneNode(GraphicsScene* scene, std::string const& name);
};
