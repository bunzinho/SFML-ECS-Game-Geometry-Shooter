#pragma once
#include <chrono>

typedef std::chrono::high_resolution_clock hires_clock;

class Clock
{
	hires_clock::time_point initial_time = hires_clock::now();
	double t = 0.0;
	const double dt = 1.0/30;
	double time_scale = 1.0;

	bool paused = false;
	double current_time = time_in_seconds();
	double accumulator = 0.0;

public:

	void set_paused(bool pause)
	{
		paused = pause;
	}

	bool is_paused() const
	{
		return paused;
	}

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
		if (paused)
		{
			current_time = time_in_seconds();
			return;
		}

		double new_time = time_in_seconds();
		double frame_time = time_scale * (new_time - current_time);

		if (frame_time > 2.0)
		{
			frame_time = dt;
		}

		current_time = new_time;
		accumulator += frame_time;
	}

	double time_in_seconds()
	{
		return std::chrono::duration<double>(hires_clock::now() - initial_time).count();
	}

	double alpha()
	{
		return accumulator / dt;
	}
};
