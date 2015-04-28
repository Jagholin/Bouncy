#pragma once
#include <memory>
#include <glm/mat4x4.hpp>
#include <istream>
#include <unordered_map>
class ShaderProgram;

class SceneNode;
class GraphicsState;
class StateData;
class Camera;
class Drawable;
class CameraNode;
class GraphicsScene
{
public:
	GraphicsScene(GraphicsState* state);
	~GraphicsScene();

	void initPyramid();
	void init();
	void render();

	void setCamera(Camera*);

	void loadFromStream(std::istream& aStream);

	// Functions to be called from SceneNode
	void registerSceneNode(std::string const& name, std::shared_ptr<SceneNode> const & node);
	void unregisterSceneNode(std::string const& name);
protected:
	//std::shared_ptr<EditableMesh> m_pyramid;
	//std::shared_ptr<ShaderProgram> m_program;

	std::shared_ptr<SceneNode> m_rootNode;
	std::weak_ptr<CameraNode> m_activeCameraNode;
	std::unordered_map<std::string, std::weak_ptr<SceneNode>> m_nodeMap;

	glm::mat4 m_projection, m_view;
	GraphicsState* m_state;

	// GraphicsState registry entry on global uniforms
	StateData* m_uniformsData;
	Camera* m_camera;
};