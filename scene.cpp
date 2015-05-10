#define GLM_SWIZZLE
#define _USE_MATH_DEFINES
#include "scene.h"
#include "mesh.h"
#include "shader.h"
#include "meshgenerator.h"
#include "scenenode.h"
#include "stateelements.h"
#include "state.h"
#include "camera.h"
#include "cameranode.h"

#include <cmath>
#include <glm/glm.hpp>
#include <sstream>
#include <functional>
#include <iostream>

GraphicsScene::GraphicsScene(GraphicsState* state) :m_rootNode(), m_state(state), m_uniformsData(nullptr)
{
	m_rootNode = SceneNode::createSceneNode<SceneNode>(this, "rootNode");
}

GraphicsScene::~GraphicsScene()
{
	m_rootNode.reset();
}

void GraphicsScene::initPyramid()
{
	/*m_program->loadFromFile("shaders/vertex.txt", "shaders/fragment.txt");

	VertexData v1, v2, v3, v4;
	v1.position = glm::vec4(1.0f, 0.0f, 0, 1.0f);
	v2.position = glm::vec4(std::cos(M_PI * 2.0f / 3.0f), std::sin(M_PI * 2.0f / 3.0f), 0, 1.0f);
	v3.position = glm::vec4(std::cos(M_PI * 4.0f / 3.0f), std::sin(M_PI * 4.0f / 3.0f), 0, 1.0f);
	v4.position = glm::vec4(0, 0, -1.0f, 1.0f);

	// Add first triangle
	{
		VertexData a(v1), b(v2), c(v4);
		glm::vec3 norm = glm::normalize(glm::cross((b.position - a.position).xyz(), (c.position - b.position).xyz()));
		glm::vec3 col(1.0f, 0.0f, 0.0f);

		a.normal = norm; a.color = col;
		b.normal = norm; b.color = col;
		c.normal = norm; c.color = col;

		m_pyramid->addTriangle(a, b, c);
	}
	{
		VertexData a(v2), b(v3), c(v4);
		glm::vec3 norm = glm::normalize(glm::cross((b.position - a.position).xyz(), (c.position - b.position).xyz()));
		glm::vec3 col(1.0f, 1.0f, 0.0f);

		a.normal = norm; a.color = col;
		b.normal = norm; b.color = col;
		c.normal = norm; c.color = col;

		m_pyramid->addTriangle(a, b, c);
	}
	{
		VertexData a(v3), b(v1), c(v4);
		glm::vec3 norm = glm::normalize(glm::cross((b.position - a.position).xyz(), (c.position - b.position).xyz()));
		glm::vec3 col(0.0f, 1.0f, 0.0f);

		a.normal = norm; a.color = col;
		b.normal = norm; b.color = col;
		c.normal = norm; c.color = col;

		m_pyramid->addTriangle(a, b, c);
	}
	{
		VertexData a(v2), b(v1), c(v3);
		glm::vec3 norm = glm::normalize(glm::cross((b.position - a.position).xyz(), (c.position - b.position).xyz()));
		glm::vec3 col(0.0f, 1.0f, 1.0f);

		a.normal = norm; a.color = col;
		b.normal = norm; b.color = col;
		c.normal = norm; c.color = col;

		m_pyramid->addTriangle(a, b, c);
	}
	m_pyramid->updateBuffers();*/
}

void GraphicsScene::init()
{
	auto myProgram = make_ref<ShaderProgram>();
	myProgram->loadFromFile("shaders/vertex.txt", "shaders/fragment.txt");
	//m_program->loadFromFile("shaders/vertex.txt", "shaders/fragment.txt");
	//m_pyramid = generateSphere();
	//m_rootNode->addDrawable(generateSphere());
	m_rootNode->stateSet().addElement("shaderProgram", std::make_shared<ProgramStateElement>(myProgram));
	auto &registry = m_state->stateData();

	m_uniformsData = registry.item_at("/globalUniforms");
	if (!m_uniformsData)
	{
		//m_state->setStateData("globalUniforms", std::make_shared<RegistryDataItem>());
		//m_uniformsData = m_state->stateData("globalUniforms");
		m_uniformsData = registry.createItemAt("/", "globalUniforms", NullType{});
	}

	if (m_camera)
	{
		m_projUniform = make_ref<GlmUniform<glm::mat4>>("projectionMatrix", m_camera->projectionMatrix());
		m_viewUniform = make_ref<GlmUniform<glm::mat4>>("viewMatrix", m_camera->viewMatrix());
		registry.createItemAt(m_uniformsData, "projectionMatrix", m_projUniform);
		//m_uniformsData->childData["projectionMatrix"] = std::make_shared<UniformStateData<glm::mat4>>(m_camera->projectionMatrix());
		registry.createItemAt(m_uniformsData, "viewMatrix", m_viewUniform);
		//m_uniformsData->childData["viewMatrix"] = std::make_shared<UniformStateData<glm::mat4>>(m_camera->viewMatrix());
	}
}

void GraphicsScene::render()
{
	//m_program->use();
	//m_pyramid->render();
	//sf::Shader::bind(nullptr);
	//m_uniformsData = m_statex
	//	->stateData("globalUniforms");

	if (m_camera)
	{
		if (! m_activeCameraNode.expired())
			m_camera->setActiveCameraNode(m_activeCameraNode);
		//auto &registry = m_state->stateData();
		//registry.createItemAt(m_uniformsData, "projectionMatrix")
		//m_uniformsData->childData["projectionMatrix"] = std::make_shared<UniformStateData<glm::mat4>>(m_camera->projectionMatrix());
		//m_uniformsData->childData["viewMatrix"] = std::make_shared<UniformStateData<glm::mat4>>(m_camera->viewMatrix());
		m_projUniform->setValue(m_camera->projectionMatrix());
		m_viewUniform->setValue(m_camera->viewMatrix());
	}
	m_rootNode->render(*m_state);
}

void GraphicsScene::setCamera(Camera* cam)
{
	m_camera = cam;
}

enum ReadState{ST_START, ST_VERTS, ST_POLYS, ST_NEWOBJECT_NAME, ST_NEWOBJECT_MATRIX, ST_MESHDATA, ST_LAMPDATA_COLOR, ST_LAMPDATA_ENERGY, ST_CAMERADATA, ST_PARENTS};

void GraphicsScene::loadFromStream(std::istream& aStream)
{
	if (aStream)
	{
		//auto myNode = std::make_shared<SceneNode>();
		//myNode->addDrawable(drawableFromStream(aStream));
		//m_rootNode->addChild(myNode);
		ReadState readState = ST_START;
		std::string currentLine;
		int lineCounter = 0;
		std::istringstream lineStream;
		std::vector<VertexData> verts_vec;
		std::vector<GLushort> polys_vec;
		std::string objectName;
		glm::mat4 objectMatrix;
		glm::vec3 lampColor;
		float lampEnergy = 1.0f;
		float cameraFov{ ((float)M_PI) / 3.0f }, cameraNear{ 0.01f }, cameraFar{ 100000.0f };
		std::vector<std::pair<std::string, std::string>> parentPairs;

		std::vector<std::shared_ptr<SceneNode>> newNodes;

		typedef std::function<void()> transitFunc;
		transitFunc nullTransit;
		transitFunc createMeshNode = [&](){
			auto myMesh = std::make_shared<EditableMesh>(verts_vec, polys_vec);
			auto myNode = SceneNode::createSceneNode<SceneNode> (this, objectName);
			myNode->addDrawable(myMesh);
			myNode->setNodeTransform(objectMatrix);

			newNodes.emplace_back(std::move(myNode));

			verts_vec.clear();
			polys_vec.clear();
			objectName.clear();
			objectMatrix = glm::mat4(1.0f);
		};

		transitFunc createCamera = [&](){
			// todo: create camera node
			auto myNode = SceneNode::createSceneNode<CameraNode>(this, objectName);
			myNode->setNodeTransform(objectMatrix);
			myNode->setFov(cameraFov);
			myNode->setNear(cameraNear);
			myNode->setFar(cameraFar);

			m_activeCameraNode = myNode;
			newNodes.emplace_back(std::move(myNode));

			objectName.clear();
			objectMatrix = glm::mat4(1.0f);
			cameraFov = ((float)M_PI) / 3.0f;
			cameraNear = 0.01f;
			cameraFar = 100000.0f;
		};

		transitFunc createLamp = [&](){
			// todo: create lamp node
			auto myNode = SceneNode::createSceneNode<SceneNode>(this, objectName);
			myNode->setNodeTransform(objectMatrix);

			newNodes.emplace_back(std::move(myNode));

			objectName.clear();
			objectMatrix = glm::mat4(1.0f);
		};

		transitFunc createEmptyNode = [&](){
			auto myNode = SceneNode::createSceneNode<SceneNode>(this, objectName);
			myNode->setNodeTransform(objectMatrix);

			newNodes.emplace_back(std::move(myNode));

			objectName.clear();
			objectMatrix = glm::mat4(1.0f);
		};

		while (std::getline(aStream, currentLine, '\n'))
		{
			++lineCounter;
			const std::unordered_multimap<std::string, std::tuple<ReadState, ReadState, transitFunc>> stateChangeMap = {
				{ "NEWOBJECT", std::make_tuple( ST_START, ST_NEWOBJECT_NAME, nullTransit ) },
				{ "MESHDATA", std::make_tuple( ST_NEWOBJECT_MATRIX, ST_MESHDATA, nullTransit ) },
				{ "LAMPDATA", std::make_tuple( ST_NEWOBJECT_MATRIX, ST_LAMPDATA_COLOR, nullTransit ) },
				{ "CAMERADATA", std::make_tuple( ST_NEWOBJECT_MATRIX, ST_CAMERADATA, nullTransit ) },
				{ "VERTS", std::make_tuple( ST_MESHDATA, ST_VERTS, nullTransit ) },
				{ "POLYS", std::make_tuple( ST_VERTS, ST_POLYS, nullTransit ) },
				{ "NEWOBJECT", std::make_tuple( ST_POLYS, ST_NEWOBJECT_NAME, createMeshNode ) },
				{ "NEWOBJECT", std::make_tuple( ST_CAMERADATA, ST_NEWOBJECT_NAME, createCamera ) },
				{ "NEWOBJECT", std::make_tuple( ST_LAMPDATA_ENERGY, ST_NEWOBJECT_NAME, createLamp ) },
				{ "PARENTS", std::make_tuple( ST_POLYS, ST_PARENTS, createMeshNode ) },
				{ "PARENTS", std::make_tuple( ST_NEWOBJECT_MATRIX, ST_PARENTS, createEmptyNode ) },
				{ "PARENTS", std::make_tuple( ST_LAMPDATA_ENERGY, ST_PARENTS, createLamp ) },
				{ "PARENTS", std::make_tuple( ST_CAMERADATA, ST_PARENTS, createCamera ) }
			};

			if (stateChangeMap.count(currentLine) > 0)
			{
				const auto statemapRange = stateChangeMap.equal_range(currentLine);
				bool stateChanged = false;
				for (auto stateChangeDesc = statemapRange.first; stateChangeDesc != statemapRange.second; ++stateChangeDesc)
				{
					if (std::get<0>(stateChangeDesc->second) == readState)
					{
						readState = std::get<1>(stateChangeDesc->second);
						stateChanged = true;
						auto transFunc = std::get<2>(stateChangeDesc->second);
						if (transFunc)
							transFunc();
						break;
					}
				}
				if (!stateChanged)
				{
					std::cerr << "Cannot change statemachine_state by reading line " << lineCounter << std::endl;
				}
				continue;
			}

			lineStream.str(currentLine);
			lineStream.clear();
			if (readState == ST_VERTS)
			{
				float f1, f2, f3, f4;
				lineStream >> f1 >> f2 >> f3 >> f4;
				if (!(lineStream.fail()))
				{
					verts_vec.push_back(VertexData{ { f1, f2, f3, f4 }, { 0.0f, 0.0f, 0.0f }, { 0.2f, 0.2f, 0.2f } });
				}
				else
				{
					std::cerr << "Couldn't read vertex data \"" << currentLine << "\" on line" << lineCounter << "\n";
				}
			}
			else if (readState == ST_POLYS)
			{
				GLushort f1, f2, f3, f4;
				lineStream >> f1 >> f2 >> f3;
				// read an optional 4th integer
				if (lineStream.fail())
				{
					std::cerr << "Couldn't read vertex indices \"" << currentLine << "\" on line" << lineCounter << "\n";
					goto endline_loc;
				}
				polys_vec.insert(std::end(polys_vec), { f1, f2, f3 });
				if (!lineStream.eof())
				{
					lineStream >> f4;
					if (lineStream.fail())
					{
						std::cerr << "Couldn't read vertex indices \"" << currentLine << "\" on line" << lineCounter << "\n";
						goto endline_loc;
					}
					//polys_vec.emplace_back(f1, f3, f4);
					polys_vec.insert(std::end(polys_vec), { f1, f3, f4 });
				}
			}
			else if (readState == ST_NEWOBJECT_NAME)
			{
				lineStream >> objectName;
				readState = ST_NEWOBJECT_MATRIX;
			}
			else if (readState == ST_NEWOBJECT_MATRIX)
			{
				float mtemp[16];
				for (unsigned int i = 0; i < 16; ++i)
				{
					lineStream >> mtemp[i];
					if (lineStream.fail())
					{
						std::cerr << "Couldn't read matrix element " << i << " on line " << lineCounter << "\n";
						goto endline_loc;
					}
				}
				for (unsigned int i = 0; i < 4; ++i)
					for (unsigned int j = 0; j < 4; ++j)
						objectMatrix[i][j] = mtemp[i+j*4];
			}
			else if (readState == ST_LAMPDATA_COLOR)
			{
				float f1, f2, f3;
				lineStream >> f1 >> f2 >> f3;
				if (lineStream.fail())
				{
					std::cerr << "Couldn't read lamp color \"" << currentLine << "\" on line" << lineCounter << "\n";
					goto endline_loc;
				}
				lampColor = { f1, f2, f3 };
				readState = ST_LAMPDATA_ENERGY;
			}
			else if (readState == ST_LAMPDATA_ENERGY)
			{
				float f1;
				lineStream >> f1;
				if (lineStream.fail())
				{
					std::cerr << "Couldn't read lamp energy \"" << currentLine << "\" on line" << lineCounter << "\n";
					goto endline_loc;
				}
				lampEnergy = f1;
			}
			else if (readState == ST_CAMERADATA)
			{
				float f1, f2, f3;
				lineStream >> f1 >> f2 >> f3;
				if (lineStream.fail())
				{
					std::cerr << "Couldn't read camera data \"" << currentLine << "\" on line" << lineCounter << "\n";
					goto endline_loc;
				}
				cameraNear = f1;
				cameraFar = f2;
				cameraFov = f3;
			}
			else if (readState == ST_PARENTS)
			{
				std::string child, parent;
				lineStream >> child >> parent;
				if (lineStream.fail())
				{
					std::cerr << "Couldn't read parent data \"" << currentLine << "\" on line" << lineCounter << "\n";
					goto endline_loc;
				}
				parentPairs.push_back(std::make_pair(child, parent));
			}
		endline_loc:
			// nothing
			;
		}

		// Go through parentPairs and assign parentship
		for (auto const& parentPair : parentPairs)
		{
			auto parent = m_nodeMap[parentPair.second].lock();
			auto child = m_nodeMap[parentPair.first].lock();
			parent->addChild(child);
		}
		for (auto const& createdNode : newNodes)
		{
			if (!createdNode->parent())
				m_rootNode->addChild(createdNode);
		}
	}
	else
		std::cerr << "Cannot open a file stream for reading" << std::endl;
}

void GraphicsScene::registerSceneNode(std::string const& name, std::shared_ptr<SceneNode> const & node)
{
	m_nodeMap[name] = node;
}

void GraphicsScene::unregisterSceneNode(std::string const& name)
{
	m_nodeMap.erase(name);
}
