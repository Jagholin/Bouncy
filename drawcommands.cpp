#include "drawcommands.h"
#include "drawable.h"

DrawElementsCommand::DrawElementsCommand(Drawable* renderThing) :
m_slave(renderThing)
{

}

void DrawElementsCommand::apply(GraphicsState& state)
{
    m_slave->render();
}

