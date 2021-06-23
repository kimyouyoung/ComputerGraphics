#pragma once


// origin: (x, z) -> (2,3)
struct Coordinate
{
	int xMin;
	int xMax;

	int yMin;
	int yMax;

	int zMin;
	int zMax;

	Coordinate() {
		xMin = 0;
		xMax = 0;
		
		yMin = 0;
		yMax = 0;

		zMin = 0;
		zMax = 0;
	}
};