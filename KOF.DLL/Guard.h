#pragma once
#include <mutex>

class Guard
{
public:
	Guard(std::recursive_mutex& mutex) : target(mutex) { target.lock(); }
	Guard(std::recursive_mutex* mutex) : target(*mutex) { target.lock(); }
	~Guard() { target.unlock(); }

protected:
	std::recursive_mutex& target;
};
