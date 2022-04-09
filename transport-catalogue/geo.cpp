#include "geo.h"
#include <cmath>

namespace geo {
	double ComputeDistance(Coordinates from, Coordinates to) {
		//if from point is same as to point then return 0
		if (from.lat == to.lat && from.lng == to.lng) { return 0; };

		const double dr = 3.1415926535 / 180.0;
		const double earth_radius = 6371000.0;
		return std::acos(std::sin(from.lat * dr) * std::sin(to.lat * dr)
			+ std::cos(from.lat * dr) * std::cos(to.lat * dr) * std::cos(std::abs(from.lng - to.lng) * dr))
			* earth_radius;
	}
} //end of namespace geo