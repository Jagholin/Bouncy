#pragma once
#include "statecommands.h"

class Drawable;

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
