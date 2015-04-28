#define _USE_MATH_DEFINES
#include "meshgenerator.h"
#include <cmath>

std::shared_ptr<EditableMesh> generateSphere()
{
	EditableMesh* newMesh = new EditableMesh;
	const glm::vec3 sphereColor{ 0.2f, 0.2f, 0.2f };
	// small sphere of 660 triangles
	VertexData topVertex;
	topVertex.normal = glm::vec3(0, 0, 1.0f);
	topVertex.position = glm::vec4(topVertex.normal, 1.0f);
	topVertex.color = sphereColor;

	auto vertexFor = [sphereColor](float phi, float theta)->VertexData
	{
		VertexData result;
		result.normal = glm::vec3(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi));
		result.position = glm::vec4(result.normal, 1.0f);
		result.color = sphereColor;
		return result;
	};

	for (unsigned short i = 0; i < 660; ++i)
	{
		unsigned short layer = (i + 30) / 60;
		unsigned short slice = (layer == 0 || layer == 11) ? i % 30 : ((i - 30) / 2) % 30;

		if (layer == 0)
		{
			newMesh->addTriangle(topVertex, vertexFor(M_PI_2 - M_PI / 12.0f, slice*(M_PI / 15.0f)), vertexFor(M_PI_2 - M_PI / 12.0f, (slice + 1)*(M_PI / 15.0f)));
		}
		else if (layer == 11)
		{
			newMesh->addTriangle(vertexFor(-M_PI_2 + M_PI / 12.0f, (slice + 1)*(M_PI / 15.0f)), vertexFor(-M_PI_2 + M_PI / 12.0f, slice*(M_PI / 15.0f)), vertexFor(-M_PI_2, 0));
		}
		else
		{
			if (i % 2 == 0)
			{
				newMesh->addTriangle(vertexFor(M_PI_2 - layer*(M_PI / 12.0f), slice * (M_PI / 15.0f)),
					vertexFor(M_PI_2 - (layer + 1)*(M_PI / 12.0f), slice * (M_PI / 15.0f)),
					vertexFor(M_PI_2 - (layer + 1)*(M_PI / 12.0f), (slice + 1) * (M_PI / 15.0f)));
			}
			else
			{
				newMesh->addTriangle(vertexFor(M_PI_2 - layer*(M_PI / 12.0f), (slice + 1) * (M_PI / 15.0f)),
					vertexFor(M_PI_2 - layer*(M_PI / 12.0f), slice * (M_PI / 15.0f)),
					vertexFor(M_PI_2 - (layer + 1)*(M_PI / 12.0f), (slice+1) * (M_PI / 15.0f)));
			}
		}
	}
	newMesh->updateBuffers();
	return std::shared_ptr<EditableMesh>(newMesh);
}