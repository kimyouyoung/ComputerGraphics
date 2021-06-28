#pragma once

struct Coordinate
{
	int xMax;
	int xMin;
	int yMax;
	int yMin;
	int zMax;
	int zMin;

	Coordinate() {
		xMax = 0;
		xMin = 0;

		yMax = 0;
		yMin = 0;

		zMax = 0;
		zMin = 0;
	}
};