#pragma once

class Drawable
{
public:
	virtual ~Drawable(){};
	virtual void render() = 0;
};