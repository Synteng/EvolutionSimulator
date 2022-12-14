#include "../inc/Creature.hpp"

Creature::Creature()
{
	return;
}

void Creature::setup(CreatureData &creatureData, SimulationRules *simulationRules)
{
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

	this->simulationRules = simulationRules;

	this->gridPosition = {0, 0};

	this->creatureData = creatureData;

	sight = creatureData.getSight();
	speed = creatureData.getSpeed();
	size  = creatureData.getSize();

	hue = creatureData.getHue();

	// sight = 1;
	// speed = 1;
	// size = 1;

	this->rayLength = RAY_LENGTH * sight;

	this->maxForce	  = 1.5 * speed;
	this->maxRotation = 0.05 * speed;

	this->energy = 50 * size;
	this->health = 100 * size;
	this->life	 = 60 * 60 * size;

	this->maxEnergy = 100 * size;
	this->maxHealth = 100 * size;
	this->maxLife	= 60 * 60 * size;

	this->radius = 12.5 * size;

	int xOffset = (roundUp(rayLength / ((float)simulationRules->size.x / simulationRules->gridResolution.x), 2) / 2);
	int yOffset = (roundUp(rayLength / ((float)simulationRules->size.y / simulationRules->gridResolution.y), 2) / 2);

	startGridOffset.x = -xOffset;
	startGridOffset.y = -yOffset;
	endGridOffset.x	  = xOffset;
	endGridOffset.y	  = yOffset;

	network = new NeuralNetwork(TOTAL_NODES, TOTAL_INPUT, this->creatureData.getConnection(),
								this->creatureData.getTotalConnections());
}

void Creature::clear()
{
	delete network;
	network = nullptr;

	position	 = {0, 0};
	velocity	 = {0, 0};
	acceleration = {0, 0};
	rotation	 = 0;
	radius		 = 0;
	network		 = nullptr;
	eating		 = false;
	layingEgg	 = false;
	sight		 = 0;
	speed		 = 0;
	size		 = 0;
	energy		 = 0;
	health		 = 0;

	return;
}

Creature::~Creature()
{
	this->clear();
}

void Creature::setPosition(agl::Vec<float, 2> position)
{
	this->position = position;

	return;
}

void Creature::setVelocity(agl::Vec<float, 2> velocity)
{
	this->velocity = velocity;

	return;
}

void Creature::setRotation(float rotation)
{
	this->rotation = rotation;

	return;
}

void Creature::setHealth(float health)
{
	this->health = health;

	return;
}

void Creature::setEnergy(float energy)
{
	this->energy = energy;

	return;
}

void Creature::setGridPosition(agl::Vec<int, 2> gridPosition)
{
	this->gridPosition = gridPosition;
}

bool isVisible()
{
	return false;
}

float closerObject(agl::Vec<float, 2> offset, float nearestDistance)
{
	return nearestDistance;
}

void Creature::updateNetwork(Grid<Food *> *foodGrid, Grid<Creature *> *creatureGrid)
{
	network->setInputNode(CONSTANT_INPUT, 1);

	network->setInputNode(X_INPUT, ((position.x / simulationRules->size.x) * 2) - 1);
	network->setInputNode(Y_INPUT, ((position.y / simulationRules->size.y) * 2) - 1);

	network->setInputNode(ROTATION_INPUT, rotation / PI);

	network->setInputNode(SPEED_INPUT, velocity.length());

	// for (int x = 0; x < RAY_TOTAL; x++)
	// {
	// 	float nearestDistance = RAY_LENGTH;
	// 	int	  type			  = 0;
	//
	// 	for (int i = 0; i < existingFood->getLength(); i++)
	// 	{
	// 		agl::Vec<float, 2> offset	= position -
	// existingFood->get(i)->position; 		float distance =
	// offset.length();
	//
	// 		if (distance > nearestDistance)
	// 		{
	// 			continue;
	// 		}
	//
	// 		float foodRotation	= vectorAngle(offset);
	// 		float creatureAngle = rotation;
	// 		float rayAngle		= (((float)x / (RAY_TOTAL - 1)) * PI) -
	// (PI / 2);
	//
	// 		rayAngle -= creatureAngle;
	//
	// 		float angleDifference	 = loop(-PI, PI, foodRotation -
	// rayAngle); 		float maxAngleDifference = (PI / RAY_TOTAL) / 2;
	//
	// 		if (angleDifference < maxAngleDifference && angleDifference >
	// -maxAngleDifference)
	// 		{
	// 			nearestDistance = distance;
	// 			type			= 1;
	// 		}
	// 	}
	//
	// 	for (int i = 0; i < existingCreature->getLength(); i++)
	// 	{
	// 		if (existingCreature->get(i) == this)
	// 		{
	// 			continue;
	// 		}
	//
	// 		agl::Vec<float, 2> offset	= position -
	// existingCreature->get(i)->getPosition();
	// 		float			   distance = offset.length();
	//
	// 		if (distance > nearestDistance)
	// 		{
	// 			continue;
	// 		}
	//
	// 		float creatureRotation = vectorAngle(offset);
	// 		float creatureAngle	   = rotation;
	// 		float rayAngle		   = (((float)x / (RAY_TOTAL - 1)) * PI)
	// - (PI / 2);
	//
	// 		rayAngle -= creatureAngle;
	//
	// 		float angleDifference	 = loop(-PI, PI, creatureRotation -
	// rayAngle); 		float maxAngleDifference = (PI / RAY_TOTAL) / 2;
	//
	// 		if (angleDifference < maxAngleDifference && angleDifference >
	// -maxAngleDifference)
	// 		{
	// 			nearestDistance = distance;
	// 			type			= -1;
	// 		}
	// 	}
	//
	// 	network->setInputNode((x + 5), (RAY_LENGTH - nearestDistance) /
	// RAY_LENGTH); 	network->setInputNode((x + 5) + RAY_TOTAL, type);
	// }

	float creatureDistance = rayLength;
	float creatureRotation = 0;
	float foodDistance	   = rayLength;
	float foodRotation	   = 0;

	for (int x = startGridOffset.x; x < endGridOffset.x; x++)
	{
		if (gridPosition.x + x < 0 || gridPosition.x + x > (creatureGrid->getSize().x - 1))
		{
			continue;
		}

		for (int y = -1; y < 2; y++)
		{
			if (gridPosition.y + y < 0 || gridPosition.y + y > (creatureGrid->getSize().y - 1))
			{
				continue;
			}

			List<Creature *> *existingCreatures = creatureGrid->getList({gridPosition.x + x, gridPosition.y + y});

			for (int i = 0; i < existingCreatures->getLength(); i++)
			{
				if (existingCreatures->get(i) == this)
				{
					continue;
				}

				agl::Vec<float, 2> offset	= position - existingCreatures->get(i)->position;
				float			   distance = offset.length();

				if (distance > creatureDistance)
				{
					continue;
				}

				creatureRotation = vectorAngle(offset) + rotation;
				creatureDistance = distance;
			}
		}
	}

	network->setInputNode(CREATURE_DISTANCE, 1 - (creatureDistance / rayLength));
	network->setInputNode(CREATURE_ROTATION, loop(-PI, PI, creatureRotation) / PI);

	for (int x = startGridOffset.x; x < endGridOffset.x; x++)
	{
		if (gridPosition.x + x < 0 || gridPosition.x + x > (foodGrid->getSize().x - 1))
		{
			continue;
		}

		for (int y = startGridOffset.x; y < endGridOffset.y; y++)
		{
			if (gridPosition.y + y < 0 || gridPosition.y + y > (foodGrid->getSize().y - 1))
			{
				continue;
			}

			List<Food *> *existingFood = foodGrid->getList({gridPosition.x + x, gridPosition.y + y});

			for (int i = 0; i < existingFood->getLength(); i++)
			{
				agl::Vec<float, 2> offset	= position - existingFood->get(i)->position;
				float			   distance = offset.length();

				if (distance > foodDistance)
				{
					continue;
				}

				foodRotation = vectorAngle(offset) + rotation;
				foodDistance = distance;
			}
		}
	}

	network->setInputNode(FOOD_DISTANCE, 1 - (foodDistance / rayLength));
	network->setInputNode(FOOD_ROTATION, loop(-PI, PI, foodRotation) / PI);

	network->setInputNode(ENERGY_INPUT, energy / maxEnergy);
	network->setInputNode(HEALTH_INPUT, health / maxHealth);
	network->setInputNode(LIFE_INPUT, life / maxLife);

	network->update();

	return;
}

void Creature::updateActions()
{
	acceleration = {0, 0};

	float force = 0;

	if (network->getNode(FOWARD_OUTPUT).value > 0)
	{
		force += network->getNode(FOWARD_OUTPUT).value * maxForce;
	}

	if (network->getNode(RIGHT_OUTPUT).value > 0)
	{
		rotation += maxRotation * network->getNode(RIGHT_OUTPUT).value;
	}

	if (network->getNode(LEFT_OUTPUT).value > 0)
	{
		rotation -= maxRotation * network->getNode(LEFT_OUTPUT).value;
	}

	if (network->getNode(EAT_OUTPUT).value > 0)
	{
		eating = true;
	}
	else
	{
		eating = false;
	}

	if (network->getNode(LAYEGG_OUTPUT).value > 0)
	{
		layingEgg = true;
	}
	else
	{
		layingEgg = false;
	}

	rotation = loop(-PI, PI, rotation);

	energy -= (sight + (abs(force * force) * (size * size * size))) / 200;

	life--;

	float			   density		 = 2;
	agl::Vec<float, 2> airResistance = {velocity.x / density, //
										velocity.y / density};

	acceleration.x = cos(rotation - (PI / 2)) * force;
	acceleration.y = sin(rotation - (PI / 2)) * force;

	acceleration = acceleration - airResistance;

	velocity.x += acceleration.x;
	velocity.y += acceleration.y;

	position.x += velocity.x;
	position.y += velocity.y;

	return;
}

CreatureData Creature::getCreatureData()
{
	return creatureData;
}

NeuralNetwork Creature::getNeuralNetwork()
{
	return *network;
}

agl::Vec<float, 2> Creature::getPosition()
{
	return position;
}

agl::Vec<float, 2> Creature::getVelocity()
{
	return velocity;
}

agl::Vec<float, 2> Creature::getAcceleration()
{
	return acceleration;
}

float Creature::getRotation()
{
	return rotation;
}

bool Creature::getEating()
{
	return eating;
}

bool Creature::getLayingEgg()
{
	return layingEgg;
}

float Creature::getHealth()
{
	return health;
}

float Creature::getEnergy()
{
	return energy;
}

int Creature::getLifeLeft()
{
	return life;
}

float Creature::getSight()
{
	return sight;
}

float Creature::getSpeed()
{
	return speed;
}

float Creature::getSize()
{
	return size;
}

float Creature::getRadius()
{
	return radius;
}

int Creature::getHue()
{
	return hue;
}

agl::Vec<int, 2> Creature::getGridPosition()
{
	return gridPosition;
}
