#include "../inc/Creature.hpp"

float hypotenuse(agl::Vec<float, 2> xy)
{
	return sqrt((xy.x * xy.x) + (xy.y * xy.y));
}

Creature::Creature()
{
	Connection connection[TOTAL_CONNECTIONS];

	connection[0].startNode = 0;
	connection[0].endNode	= TOTAL_INPUT + 0;
	connection[0].weight	= 0;

	// INPUT
	// constant
	// x pos
	// y pos
	// rotation
	// speed
	// Ray[i] distance to object
	// Ray[i] object type (-1 = creature, 0 = nothing, 1 = food)
	//
	// OUTPUT
	// Move foward
	// Move backward
	// Turn right
	// Turn left
	// Eat
	// Lay egg
	network = new NeuralNetwork(TOTAL_NODES, 5 + (RAY_TOTAL * 2), connection, TOTAL_CONNECTIONS);

	return;
}

void Creature::setPosition(agl::Vec<float, 2> position)
{
	this->position = position;

	return;
}

void Creature::setWorldSize(agl::Vec<float, 2> worldSize)
{
	this->worldSize = worldSize;

	return;
}

float vectorAngle(agl::Vec<float, 2> vec)
{
	float angle = atan(vec.x / vec.y);

	if (vec.y < 0)
	{
		angle *= -1;

		if (vec.x > 0)
		{
			angle = PI - angle;
		}
		else
		{
			angle = -(PI + angle);
		}
	}

	return angle;
}

void Creature::updateNetwork(Food *food, int totalFood)
{
	for (int x = 0; x < RAY_TOTAL; x++)
	{
		float nearestDistance = RAY_LENGTH;

		for (int i = 0; i < totalFood; i++)
		{
			agl::Vec<float, 2> offset	= position - food[i].position;
			float			   distance = offset.length();

			if (distance > nearestDistance)
			{
				continue;
			}

			float foodRotation = vectorAngle(offset);
			float creatureAngle = rotation + (PI / 2);
			float rayAngle = (((float)x / RAY_TOTAL) * PI);

			foodRotation += creatureAngle;
			rayAngle += creatureAngle;

			float angleDifference = foodRotation - rayAngle;
			float maxAngleDifference = (PI / RAY_TOTAL) / 2;

			if(angleDifference < maxAngleDifference && angleDifference > -maxAngleDifference)
			{
				nearestDistance = distance;
			}
		}

		network->setInputNode((x + 5), (RAY_LENGTH - nearestDistance) / RAY_LENGTH);
	}

	network->setInputNode(0, 1);

	network->update();

	return;
}

void Creature::updateActions(Food *food)
{
	velocity = {0, 0};

	float speed = 0;

	if (network->getNode(TOTAL_INPUT + 0).value > 0)
	{
		speed += network->getNode(0).value * 2.5;
	}

	if (network->getNode(TOTAL_INPUT + 1).value > 0)
	{
		speed -= network->getNode(0).value * 2.5;
	}

	if (network->getNode(TOTAL_INPUT + 2).value > 0)
	{
		rotation += 0.05 * network->getNode(13).value;
	}

	if (network->getNode(TOTAL_INPUT + 3).value > 0)
	{
		rotation -= 0.05 * network->getNode(14).value;
	}

	if (network->getNode(TOTAL_INPUT + 4).value > 0)
	{
		eating = true;
	}

	if (network->getNode(TOTAL_INPUT + 5).value > 0)
	{
		layingEgg = true;
	}

	velocity.x = cos(rotation) * speed;
	velocity.y = sin(rotation) * speed;

	position.x += velocity.x;
	position.y += velocity.y;
	return;
}

NeuralNetwork Creature::getNeuralNetwork()
{
	return *network;
}

agl::Vec<float, 2> Creature::getPosition()
{
	return position;
}

float Creature::getRotation()
{
	return -rotation * 180 / 3.14159;
}

bool Creature::getEating()
{
	return eating;
}

bool Creature::getLayingEgg()
{
	return layingEgg;
}
