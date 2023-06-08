#include "Clock.h"

double Clock::get_frame_time() const
{
	return frame_time;
}

void Clock::set_paused(bool pause)
{
	paused = pause;
}

bool Clock::is_paused() const
{
	return paused;
}

double Clock::get_game_time() const
{
	return t;
}

double Clock::get_starting_physics_time() const
{
	return current_time;
}

double Clock::delta_time() const
{
	return dt;
}

bool Clock::is_tick_ready() const
{
	return accumulator >= dt;
}

void Clock::update_accumulator()
{
	accumulator -= dt;
	t += dt;
}

void Clock::update_time()
{
	current_time = time_in_seconds();
}

void Clock::update_delta_time()
{
	if (paused)
	{
		current_time = time_in_seconds();
		return;
	}

	double new_time = time_in_seconds();
	frame_time = new_time - current_time;

	if (frame_time > dt)
	{
		frame_time = dt;
	}

	current_time = new_time;
	accumulator += frame_time * time_scale;
}

double Clock::time_in_seconds() const
{
	return std::chrono::duration<double>(hires_clock::now() - initial_time).count();
}

double Clock::alpha() const
{
	return accumulator / dt;
}

double Clock::add_time_scale(double t)
{
	time_scale += t;
	return time_scale;
}
