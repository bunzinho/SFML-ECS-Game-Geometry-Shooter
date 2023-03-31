#pragma once

#include "Common.h"

class CTransform 
{
public:
    Vec2 pos        = {0.0f, 0.0f};
    Vec2 last_pos   = {0.0f, 0.0f};
    Vec2 velocity   = {0.0f, 0.0f};
    float angle     = 0.0f;

	CTransform(const Vec2& p, const Vec2& v, float a)
		: pos(p), velocity(v), angle(a) {}
};

class CShape
{
public:
    sf::CircleShape circle;

    CShape(float radius, int points, const sf::Color& fill, const sf::Color& outline, float thickness)
        : circle(radius, points)
    {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(radius, radius);
    }
};

class CCollision
{
public:
	float radius = 0;
	CCollision(float r)
		: radius(r) {}
};

class CScore
{
public:
	int score = 0;
	CScore(int s)
		: score(s) {}
};

class CLifespan 
{
public:
    double lifespan     = 0.0;   // amount of total lifespan of the entity
    double frameCreated = 0.0;   // when the lifespan component was created
    CLifespan(double duration, double created) 
        : lifespan(duration), frameCreated(created) {}
};

class CInput
{
public:
    bool up     = false;
    bool left   = false;
    bool right  = false;
    bool down   = false;
    bool shoot  = false;

    CInput() {}
};
