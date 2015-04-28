#include <GL/glew.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <iostream>
#include <fstream>

#include "scene.h"
#include "state.h"
#include "camera.h"
#include "osdialogs.h"

int main()
{
	sf::Window window(sf::VideoMode(768, 768), "Bouncy!");
	window.setActive();
	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		std::cout << "Error while initializing GleW: " << glewGetErrorString(err) << std::endl;
	}
	glClearColor(0, 0, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	std::unique_ptr<GraphicsState> glState{ new GraphicsState() };
	Camera myCamera{};
	GraphicsScene myScene(glState.get());
	myScene.setCamera(&myCamera);
	myScene.init();
	std::string fileName = getOpenFileName();
	myScene.loadFromStream(std::ifstream(fileName.c_str()));

	// camera tracking variables
	float currPhi = 0.0f, currTheta = 0.0f;
	bool mousePositionValid = false;
	int lastMouseX = 0, lastMouseY = 0;
	float rollfactor = 0.005f;
	float speed = 0.5f;
	sf::Clock myClock;

	while (window.isOpen())
	{
		sf::Time timeElapsed = myClock.restart();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::Resized)
			{
				//window.setSize(sf::Vector2u(event.size.width, event.size.height));
				glViewport(0, 0, event.size.width, event.size.height);
				myCamera.setSideRatio(event.size.width / (float)event.size.height);
			}
			else if (event.type == sf::Event::MouseMoved)
			{
				int xcoord = event.mouseMove.x;
				int ycoord = event.mouseMove.y;
				if (!mousePositionValid)
				{
					lastMouseX = xcoord;
					lastMouseY = ycoord;
					mousePositionValid = true;
					continue;
				}
				currPhi = myCamera.setPhi(currPhi + (lastMouseY - ycoord)*rollfactor);
				currTheta = myCamera.setTheta(currTheta + (lastMouseX - xcoord)*rollfactor);
				lastMouseX = xcoord;
				lastMouseY = ycoord;
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Escape)
					window.close();
			}
		}

		bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

		if (upPressed && !downPressed)
		{
			myCamera.goforward(speed * timeElapsed.asSeconds());
		}
		else if (downPressed && !upPressed)
		{
			myCamera.gobackward(speed * timeElapsed.asSeconds());
		}

		if (window.isOpen())
		{
			window.setActive();
			//window.clear(sf::Color::Blue);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			myScene.render();

			window.display();
		}
	}

	return 0;
}