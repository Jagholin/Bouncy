#pragma once

class CommandQueue;
class GraphicsState;

class Drawable
{
public:
	virtual ~Drawable(){};
	virtual void render() = 0;
	virtual void addToQueue(GraphicsState&, CommandQueue& out) = 0;
};