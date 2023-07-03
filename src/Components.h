#pragma once

#include <SFML/Graphics.hpp>
#include "Vec2.h"

class Component
{
public:
    bool has = false;
};

class CTransform : public Component
{
public:
    Vec2 pos        = {0.0f, 0.0f};
    Vec2 last_pos   = {0.0f, 0.0f};
    Vec2 velocity   = {0.0f, 0.0f};
    float angle     = 0.0f;

	CTransform(const Vec2& p, const Vec2& v, float a)
		: pos(p), last_pos(p), velocity(v), angle(a) {}

    CTransform() = default;
};

class CShape : public Component
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

    CShape() = default;
};

class CCollision : public Component
{
public:
	float radius = 0;
	CCollision(float r)
		: radius(r) {}

    CCollision() = default;
};

class CScore : public Component
{
public:
	int score = 0;
	CScore(int s)
		: score(s) {}

    CScore() = default;
};

class CLifespan : public Component
{
public:
    double lifespan     = 0.0;
    double frameCreated = 0.0;
    CLifespan(double duration, double created) 
        : lifespan(duration), frameCreated(created) {}

    CLifespan() = default;
};

class CInput : public Component
{
public:
    bool up      = false;
    bool left    = false;
    bool right   = false;
    bool down    = false;
    bool shoot   = false;
    bool special = false;
    int x_mouse = 0;
    int y_mouse = 0;

    CInput() = default;
};
