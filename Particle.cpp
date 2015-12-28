#include "Particle.h"
 


Particle::Particle():life(-1)
{
}


Particle::~Particle()
{
}

static double gravity = 0.4;

void Particle::update(int elapsed)
{
	life -= elapsed;
	x += speedX * elapsed;
	y += speedY * elapsed;

	speedY += gravity * elapsed / 1000;
}
 

int Particle::getX()
{
	return x;
}


int Particle::getY()
{
	return y;
}


COLORREF Particle::getColor()
{
	return color;
}

#include <stdlib.h>
#include <math.h>
#define PI 3.1415926
void Particle::reset(int positionX, int positionY, COLORREF color)
{
	x = positionX;
	y = positionY;
	this->color = color;

	life = rand() % 200 + 800;

	int velocity = rand()%30 + 150;

	int angle = rand() % 21+70;

	speedY = -sin(PI / 180 * angle) * velocity / 1000;
	speedX = cos(PI / 180 * angle) * velocity / 1000;

	int sign = rand()%2 ? 1 : -1;
	speedX = speedX * sign;
}


BOOL Particle::isAlive()
{
	return life > 0;
}
