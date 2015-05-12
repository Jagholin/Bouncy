#pragma once
#include "statecommands.h"
#include "commandqueue.h"

class Drawable;

class DrawCommand : public RenderCommand
{
public:
    bool isDraw() const override {
        return true;
    }
};

class DrawElementsCommand : public RenderCommand
{
public:
    DrawElementsCommand(Drawable* renderThing);

    void apply(GraphicsState&) override;
    //void addToQueue(const GraphicsState& theState, CommandQueue& commandQueue);
    RenderCommand::CommandType type() const override;

protected:
    Drawable* m_slave;
};
