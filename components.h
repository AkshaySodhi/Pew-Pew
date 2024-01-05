#pragma once

#include "Vec2.h"
#include<SFML/Graphics.hpp>

class CTransform
{
public:
	Vec2 pos = { 0.f,0.f };
	Vec2 velocity = { 0.f,0.f };
	float angle = 0;

	CTransform(const Vec2& p, const Vec2& v, float a)
		:pos(p), velocity(v), angle(a) {};
};

class CShape
{
public:
	sf::CircleShape circle;

	CShape(float radius, int points, const sf::Color& fill, const sf::Color& outline, float thickness)
		:circle(radius, points)
	{
		circle.setFillColor(fill);
		circle.setOutlineThickness(thickness);
		circle.setOutlineColor(outline);
		circle.setOrigin(radius, radius);
	};
};

class CCollision
{
public:
	float radius = 0;
	CCollision(float r)
		:radius(r) {};
};

class CScore
{
public:
	int score;
	CScore(int s)
		:score(s) {}
};

class CLifespan
{
public:
	float remaining = 0;
	float total = 0;
	CLifespan(float tspan)
		:total(tspan), remaining(tspan) {};
};

class CInput
{
public:
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;
	bool shoot = false;

	CInput() {};
};