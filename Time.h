#pragma once
#include <chrono>
typedef std::chrono::high_resolution_clock hires_clock;
using namespace std::chrono;

class Time
{
	steady_clock::time_point start = hires_clock::now();
	double t = 0.0;
	const double dt = 1.0/20;

	double current_time = time_in_seconds();
	double game_time = 0.0;
	double accumulator = 0.0;

public:
	double get_game_time()
	{
		return t;
	}

	double get_starting_physics_time()
	{
		return current_time;
	}
	double delta_time()
	{
		return dt;
	}

	bool is_tick_ready()
	{
		return accumulator >= dt;
	}

	void update_accumulator()
	{
		accumulator -= dt;
		t += dt;
	}

	void update_time()
	{
		current_time = time_in_seconds();
	}

	void update_delta_time()
	{
		double new_time = time_in_seconds();
		double frame_time = new_time - current_time;
		current_time = new_time;
		accumulator += frame_time;
	}

	double time_in_seconds()
	{
		return duration<double>(hires_clock::now() - start).count();
	}

	double alpha()
	{
		return accumulator / dt;
	}
};
