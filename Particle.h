#pragma once
#include <Windows.h>
class Particle
{
public:
	Particle();
	~Particle();

private:
	double x;
	double y;
	double speedX;
	double speedY;
	int life;
	COLORREF color;
public:
	void update(int elapsed);
	int getX();
	int getY();
	COLORREF getColor();
	void reset(int positionX, int positionY, COLORREF color);
	BOOL isAlive();
};

