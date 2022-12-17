#include "../lib/AGL/agl.hpp"

#include "../inc/Simulation.hpp"

#include <X11/X.h>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <unistd.h>

#define TOTAL_FOOD 10

#define BASE_B_VALUE 63

#define WIDTH  1920
#define HEIGHT 1080

agl::Vec<float, 3> Vec2fVec3f(agl::Vec<float, 2> vec)
{
	agl::Vec<float, 3> newVec;

	newVec.x = vec.x;
	newVec.y = vec.y;

	return newVec;
}

agl::Vec<float, 2> Vec2iVec2f(agl::Vec<int, 2> vec)
{
	return {(float)vec.x, (float)vec.y};
}

void printBits(unsigned char buffer[TOTAL_CONNECTIONS * 3])
{
	for (int x = 0; x < TOTAL_CONNECTIONS * 3; x++)
	{
		for (int i = 0; i < 8; i++)
		{
			printf("%d", !!((buffer[x] << i) & 0x80));
		}

		printf(" ");
	}

	printf("\n");

	return;
}

int main()
{
	agl::RenderWindow window;
	window.setup({WIDTH, HEIGHT}, "EvolutionSimulator");
	window.setClearColor(agl::Color::Black);
	window.setFPS(60);

	agl::Event event;
	event.setWindow(window);

	agl::Shader shader;
	shader.loadFromFile("./vert.glsl", "./frag.glsl");
	window.getShaderUniforms(shader);
	shader.use();

	agl::Camera camera;
	camera.setOrthographicProjection(0, WIDTH, HEIGHT, 0, 0.1, 100);
	agl::Vec<float, 2> cameraPosition = {0, 0};
	camera.setView({cameraPosition.x, cameraPosition.y, 50}, {0, 0, 0}, {0, 1, 0});

	agl::Texture blank;
	blank.setBlank();

	agl::Circle nodeShape(10);
	nodeShape.setTexture(&blank);
	nodeShape.setSize(agl::Vec<float, 3>{10, 10, 0});
	nodeShape.setPosition(agl::Vec<float, 3>{500, 500, 3});

	agl::Rectangle connectionShape;
	connectionShape.setTexture(&blank);
	connectionShape.setColor(agl::Color::Red);
	connectionShape.setSize(agl::Vec<float, 3>{1, 50, 2});

	agl::Circle background(6);
	background.setTexture(&blank);
	background.setColor({15, 15, 15});
	background.setPosition(agl::Vec<float, 3>{150, 150, -1});
	background.setSize(agl::Vec<float, 3>{150, 150, 1});

	agl::Circle foodShape(10);
	foodShape.setTexture(&blank);
	foodShape.setColor(agl::Color::Green);
	foodShape.setSize(agl::Vec<float, 3>{10, 10, 0});

	agl::Rectangle creatureShape;
	creatureShape.setTexture(&blank);
	creatureShape.setColor(agl::Color::White);
	creatureShape.setSize(agl::Vec<float, 3>{25, 25, 0});
	creatureShape.setOffset(agl::Vec<float, 3>{-12.5, -12.5, 0});

	agl::Circle eggShape(10);
	eggShape.setTexture(&blank);
	eggShape.setColor(agl::Color::White);
	eggShape.setSize(agl::Vec<float, 3>{10, 10, 0});

	agl::Rectangle rayShape;
	rayShape.setTexture(&blank);
	rayShape.setColor(agl::Color::White);
	rayShape.setSize(agl::Vec<float, 3>{1, RAY_LENGTH, -1});
	rayShape.setOffset(agl::Vec<float, 3>{-0.5, 0, 0});

	Simulation simulation({WIDTH / 2, HEIGHT / 2}, 4, 300, 2);

	Creature		 *creature			= simulation.getCreatureBuffer();
	List<Creature *> *existingCreatures = simulation.getExistingCreatures();
	Egg				 *egg				= simulation.getEggBuffer();
	List<Egg *>		 *existingEggs		= simulation.getExistingEggs();
	Food			 *food				= simulation.getFoodBuffer();

	Creature *focusCreature;

	CreatureData creatureData = creature->saveData();

	Buffer buffer = Simulation::creatureDataToBuffer(creatureData);

	printBits(buffer.data);

	for (int i = 0; i < TOTAL_CONNECTIONS; i++)
	{
		printf("%d ", buffer.data[(i * 3) + 0]);
		printf("%d ", buffer.data[(i * 3) + 1]);
		printf("%d\n", buffer.data[(i * 3) + 2]);
	}

	bool mHeld	= false;
	bool b1Held = false;

	while (!event.windowClose())
	{
		window.updateMvp(camera);

		event.pollWindow();
		event.pollKeyboard();
		event.pollPointer();

		if (!event.isKeyPressed(XK_space))
		{
			simulation.update();
		}

		if (event.isKeyPressed(XK_m))
		{
			mHeld = true;
		}
		else if (mHeld)
		{
			mHeld = false;

			existingCreatures->pop(0);
		}

		window.clear();

		// AGL rendering

		// draw creature
		for (int i = 0; i < existingCreatures->getLength(); i++)
		{
			creatureShape.setPosition(existingCreatures->get(i)->getPosition());
			creatureShape.setRotation(
				agl::Vec<float, 3>{0, 0, -float(existingCreatures->get(i)->getRotation() * 180 / PI)});
			window.drawShape(creatureShape);
		}

		// Draw food
		for (int i = 0; i < simulation.getExistingFood()->getLength(); i++)
		{
			foodShape.setColor(agl::Color::Green);

			agl::Vec<float, 2> position = simulation.getExistingFood()->get(i)->position;
			foodShape.setPosition(agl::Vec<float, 3>{position.x, position.y, -1});
			window.drawShape(foodShape);
		}

		// draw eggs
		for (int i = 0; i < existingEggs->getLength(); i++)
		{
			eggShape.setPosition(existingEggs->get(i)->getPosition());
			window.drawShape(eggShape);
		}

		// draw background
		window.drawShape(background);

		if (existingCreatures->find(focusCreature) != -1)
		{
			int index = existingCreatures->find(focusCreature);

			// draw rays
			for (int i = 0; i < RAY_TOTAL; i++)
			{
				float angleOffset = ((i / ((float)RAY_TOTAL - 1)) * 180) + 90;

				float weight = existingCreatures->get(index)->getNeuralNetwork().getNode(i + 5).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setPosition(existingCreatures->get(index)->getPosition());
				rayShape.setRotation(agl::Vec<float, 3>{
					0, 0, -float(existingCreatures->get(index)->getRotation() * 180 / PI) + angleOffset});
				window.drawShape(rayShape);
			}

			// draw node connections
			for (int i = 0; i < existingCreatures->get(existingCreatures->find(focusCreature))
									->getNeuralNetwork()
									.getTotalConnections();
				 i++)
			{
				float startAngle = (360. / existingCreatures->get(index)->getNeuralNetwork().getTotalNodes()) *
								   (existingCreatures->get(index)->getNeuralNetwork().getConnection(i).startNode + 1);
				agl::Vec<float, 3> start = Vec2fVec3f(agl::pointOnCircle(agl::degreeToRadian(startAngle)));
				start.x					 = (start.x * 100) + 150;
				start.y					 = (start.y * 100) + 150;

				float endAngle = (360. / existingCreatures->get(index)->getNeuralNetwork().getTotalNodes()) *
								 (existingCreatures->get(index)->getNeuralNetwork().getConnection(i).endNode + 1);
				agl::Vec<float, 3> end = Vec2fVec3f(agl::pointOnCircle(agl::degreeToRadian(endAngle)));
				end.x				   = (end.x * 100) + 150;
				end.y				   = (end.y * 100) + 150;

				agl::Vec<float, 3> offset = end - start;

				float length = offset.length();
				connectionShape.setSize(agl::Vec<float, 3>{2, length, 0});

				float angle = acos(offset.x / length) * (180 / 3.14159);
				connectionShape.setRotation(agl::Vec<float, 3>{0, 0, angle + 90});

				float weight = existingCreatures->get(index)->getNeuralNetwork().getConnection(i).weight;

				if (weight > 0)
				{
					connectionShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});
				}
				else
				{
					connectionShape.setColor({(unsigned char)(-weight * 255), 0, BASE_B_VALUE});
				}

				connectionShape.setPosition(start);
				window.drawShape(connectionShape);
			}

			// draw nodes
			for (int i = 0;
				 i < existingCreatures->get(existingCreatures->find(focusCreature))->getNeuralNetwork().getTotalNodes();
				 i++)
			{
				float angle = (360. / existingCreatures->get(index)->getNeuralNetwork().getTotalNodes()) * (i + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				agl::Vec<float, 3> pos;
				pos.x = x * 100;
				pos.y = y * 100;

				pos.x += 150;
				pos.y += 150;

				pos.z = 3;

				nodeShape.setPosition(pos);

				float nodeValue = existingCreatures->get(index)->getNeuralNetwork().getNode(i).value;

				if (nodeValue > 0)
				{
					nodeShape.setColor({0, (unsigned char)(nodeValue * 255), BASE_B_VALUE});
				}
				else
				{
					nodeShape.setColor({(unsigned char)(-nodeValue * 255), 0, BASE_B_VALUE});
				}

				window.drawShape(nodeShape);
			}
		}

		window.display();

		if (event.isKeyPressed(XK_r))
		{
			focusCreature = nullptr;
		}

		static float sizeMultiplier = 1;

		if (event.isPointerButtonPressed(Button1Mask))
		{
			for (int i = 0; i < existingCreatures->getLength(); i++)
			{
				agl::Vec<float, 2> mouse;
				mouse.x = event.getPointerWindowPosition().x + (cameraPosition.x * sizeMultiplier) - (WIDTH / 2.);
				mouse.y = event.getPointerWindowPosition().y + (cameraPosition.y * sizeMultiplier) - (HEIGHT / 2.);

				float distance = (mouse - existingCreatures->get(i)->getPosition()).length();

				printf("%x %f\n", existingCreatures->get(i), distance);

				if (distance < 25)
				{
					focusCreature = existingCreatures->get(i);

					break;
				}
			}
		}

		printf("\n");

		if (event.isPointerButtonPressed(Button3Mask))
		{
			for (int i = 0; i < existingCreatures->getLength(); i++)
			{
				agl::Vec<float, 2> mouse;
				mouse.x = event.getPointerWindowPosition().x;
				mouse.y = event.getPointerWindowPosition().y;

				float distance = (mouse - existingCreatures->get(i)->getPosition()).length();

				if (distance < 25)
				{
					existingCreatures->pop(i);

					break;
				}
			}
		}

		static agl::Vec<float, 2> cameraOffset;
		static agl::Vec<float, 2> startPos;

		if (event.isPointerButtonPressed(Button1Mask))
		{
			if (b1Held) // holding click
			{
				cameraPosition = cameraPosition - cameraOffset;

				cameraOffset = startPos - Vec2iVec2f(event.getPointerWindowPosition());
				cameraOffset.x *= sizeMultiplier;
				cameraOffset.y *= sizeMultiplier;

				cameraPosition.x += cameraOffset.x;
				cameraPosition.y += cameraOffset.y;
			}
			else // first click
			{
				startPos = Vec2iVec2f(event.getPointerWindowPosition());
				b1Held	 = true;
			}
		}
		else if (b1Held) // let go
		{
			cameraOffset = {0, 0};
			b1Held		 = false;
		}

		static float cameraSpeed = 4;

		if (event.isKeyPressed(XK_Down))
		{
			sizeMultiplier += .1;
		}
		if (event.isKeyPressed(XK_Up))
		{
			sizeMultiplier -= .1;
			if (sizeMultiplier < 0)
			{
				sizeMultiplier = 0;
			}
		}

		camera.setOrthographicProjection(-((WIDTH / 2.) * sizeMultiplier), ((WIDTH / 2.) * sizeMultiplier),
										 ((HEIGHT / 2.) * sizeMultiplier), -((HEIGHT / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});
	}

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
