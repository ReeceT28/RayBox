#include <SFML/Graphics.hpp>
#include "opencl.hpp"
#include "Simulation.h"


int main() 
{
	Simulation s;
	s.run();
	wait();
	return 0;
}