#include "Wall.h"
#include <cmath>

#ifndef THRESHOLD
#define THRESHOLD 0.00001f
#endif

#ifndef THRESHOLD_BORDER
#define THRESHOLD_BORDER 0.3f
#endif

#define TWOPI 6.283185307179586476925287
#define RTOD 57.2957795

namespace ISM
{
	Wall::Wall() : A(0.0), B(0.0), C(0.0), D(0.0)
	{
		std::vector<float> tempAbsotionBands(NUM_BAND_ABSORTION, 0.0);
		absortionBands = tempAbsotionBands;								//Wall purely reflective by default
		active = true;													//Wall active by default
	}

	bool Wall::insertCorner(const Common::CVector3& _corner)
	{				
		if (polygon.size() < 3)
		{
			polygon.push_back(_corner);
			return true;
		}
		// Now we have 3 corners
		if (polygon.size() == 3)
		{
			calculate_ABCD();
		}
		double diff = _corner.x * A + _corner.y * B + _corner.z * C + D;
		diff = std::fabs(diff);
		if (diff < THRESHOLD) // ¿DBL_EPSILON? ¿THRESHOLD?
		{
			polygon.push_back(_corner);
			return true;
		}
		else
		{
			Common::CVector3 _calculatedCorner = getProjectionPoint(_corner);
			polygon.push_back(_calculatedCorner);
			return false;
		}		
	}


	bool Wall::insertCorner(const float& _x, const float& _y, const float& _z)
	{
		Common::CVector3 tempCorner(_x, _y, _z);
		return insertCorner(tempCorner);		
	}
	
	const std::vector<Common::CVector3>& Wall::getCorners() const
	{
		return polygon;
	}

	void Wall::setAbsortion(const float& _absortion)
	{
		std::vector<float> tempAbsortionBands(NUM_BAND_ABSORTION, _absortion);
		absortionBands = tempAbsortionBands;
	}
	
	void Wall::setAbsortion (const std::vector<float>& _absortionBands)
	{
		absortionBands = _absortionBands;
	}
		
	const std::vector<float>& Wall::getAbsortionB() const
	{
		return absortionBands;
	}
	/*std::vector<float> Wall::getAbsortionB() const {
		return const_cast<Wall*>(this)->getAbsortionB();
	}*/

	Common::CVector3& Wall::getNormal() const
	{
		//Common::CVector3 normal, p1, p2; 
		Common::CVector3 p1, p2, normal;
		float modulus;

		p1 = polygon.at(1) - polygon.at(0);
		p2 = polygon.at(2) - polygon.at(0);

		normal = p1.CrossProduct(p2);

		modulus = normal.GetDistance();

		normal.x = normal.x / modulus;
		normal.y = normal.y / modulus;
		normal.z = normal.z / modulus;

		return normal;
	}

	Common::CVector3& Wall::getCenter() const
	{
		Common::CVector3 center;

		center = Common::CVector3::ZERO;

		for (auto i = 0; i < polygon.size(); i++)
		{
			center.x += polygon.at(i).x;
			center.y += polygon.at(i).y;
			center.z += polygon.at(i).z;
		}
		center.x /= polygon.size();
		center.y /= polygon.size();
		center.z /= polygon.size();

		return center;

	}

	Common::CVector3& Wall::getProjectionPoint(const Common::CVector3& point) const
	{
		// Vectorial Ec. of straight line --> (X,Y,Z) = (x0, y0, z0) + lambda (normalV.x, normalV.y, normalV.z)
		// Plane of the wall              --> AX+BY+CZ+D = 0

		Common::CVector3 normalV;
		double rX1, rY1, rZ1, lambda;
		double rX2, rY2, rZ2;
		double diff1, diff2;
		float rX, rY, rZ;

		//calculate_ABCD();
		normalV = getNormal();
		lambda = (double)getDistanceFromPoint(point);

		// lambda could be positive or negative
		// 
		rX1 = point.x + lambda * normalV.x;
		rY1 = point.y + lambda * normalV.y;
		rZ1 = point.z + lambda * normalV.z;
		diff1 = rX1 * A + rY1 * B + rZ1 * C + D;
		diff1 = fabs(diff1);

		rX2 = point.x - lambda * normalV.x;
		rY2 = point.y - lambda * normalV.y;
		rZ2 = point.z - lambda * normalV.z;
		diff2 = rX2 * A + rY2 * B + rZ2 * C + D;
		diff2 = fabs(diff2);

		if (diff1 < diff2)
		{
			rX = rX1;
			rY = rY1;
			rZ = rZ1;
		}
		else
		{
			rX = rX2;
			rY = rY2;
			rZ = rZ2;
		}
		Common::CVector3 projectionPoint = Common::CVector3(rX, rY, rZ);
		return projectionPoint;		
	}

	Common::CVector3& Wall::getProjectionPoint(float& x0, float& y0, float& z0) const
	{
		Common::CVector3 point(x0, y0, z0);
		return getProjectionPoint(point);
	}

	float Wall::getDistanceFromPoint(const Common::CVector3& point) const
	{
		float distance;
		//calculate_ABCD();
		distance = fabs(A*point.x + B * point.y + C * point.z + D);
		distance = distance / sqrtf(A * A + B * B + C * C);
		return distance;
	}

	/*float Wall::getMinimumDistanceFromWall(ISM::Wall wall) const {
		return const_cast<Wall*>(this)->getMinimumDistanceFromWall(wall);
	}*/

	float Wall::getMinimumDistanceFromWall(const ISM::Wall& wall) const
	{
		Common::CVector3 cornerDistance = polygon.at(0) - wall.polygon.at(0);
		float minimumDistance=cornerDistance.GetDistance();
		for (int i = 0; i < polygon.size(); i++)
		{
			for (int j = 0; j < wall.polygon.size(); j++)
			{
				cornerDistance = polygon.at(i) - wall.polygon.at(j);
				if (minimumDistance > cornerDistance.GetDistance())
				{
					minimumDistance = cornerDistance.GetDistance();
				}
			}
		}
		return minimumDistance;
	}

	Common::CVector3 Wall::getImagePoint(const Common::CVector3& point) const
	{
		float distance;
		Common::CVector3 imagePoint, normalRay;
		distance = getDistanceFromPoint(point);

		normalRay = getNormal();
		normalRay.x *= -(2 * distance);
		normalRay.y *= -(2 * distance);
		normalRay.z *= -(2 * distance);

		imagePoint = point + normalRay;

		return imagePoint;
	}
	/*Common::CVector3 Wall::getImagePoint(const Common::CVector3& point) const {
		return const_cast<Wall*>(this)->getImagePoint(point);
	}*/

	Wall Wall::getImageWall(const Wall& _wall) const
	{
		Wall tempWall;
		std::vector<Common::CVector3> corners = _wall.getCorners();
		for (int i = corners.size() - 1; i >= 0; i--)
		{
			Common::CVector3 tempImageCorner = getImagePoint(corners.at(i));
			tempWall.insertCorner(tempImageCorner);
		}
		tempWall.absortionBands = _wall.absortionBands;
		if (!_wall.isActive()) tempWall.disable();
		return tempWall;
	}

	Common::CVector3 Wall::getIntersectionPointWithLine(const Common::CVector3& p1, const Common::CVector3& p2) const
	{
		Common::CVector3 cutPoint, vecLine;
		float modulus, lambda;

		vecLine = p2 - p1;

		lambda = (-D - (A *p1.x + B * p1.y + C * p1.z));
		lambda = lambda / (A*vecLine.x + B * vecLine.y + C * vecLine.z);

		cutPoint.x = p1.x + lambda * vecLine.x;
		cutPoint.y = p1.y + lambda * vecLine.y;
		cutPoint.z = p1.z + lambda * vecLine.z;

		modulus = getDistanceFromPoint(cutPoint); // must be = ZERO

		return cutPoint;
	}

	int  Wall::checkPointInsideWall(const Common::CVector3& point, float &distanceNearestEdge, float &sharpness) const
	{
		float modulus = getDistanceFromPoint(point);
		if (modulus > 5*THRESHOLD)
		{
			sharpness = 0.0;
			return 0;                           // Point is not in the wall's plane       
		}

		double m1, m2, anglesum = 0, costheta, anglediff;
		Common::CVector3 p1, p2;
		int n = polygon.size();

		for (auto i = 0; i < n; i++)
		{
			p1.x = polygon[i].x - point.x;
			p1.y = polygon[i].y - point.y;
			p1.z = polygon[i].z - point.z;
			p2.x = polygon[(i + 1) % n].x - point.x;
			p2.y = polygon[(i + 1) % n].y - point.y;
			p2.z = polygon[(i + 1) % n].z - point.z;
			m1 = p1.GetDistance();
			m2 = p2.GetDistance();
			if (m1*m2 <= THRESHOLD)
			{
				distanceNearestEdge = 0.0f;
				sharpness = 0.5f;
				return 1;                       // Point is on a corner of the wall,
			}
			else
				costheta = (p1.x*p2.x + p1.y*p2.y + p1.z*p2.z) / (m1*m2);

			anglesum += acos(costheta);
		}

		anglediff = fabs(TWOPI - anglesum);
		if (anglediff < THRESHOLD)
		{   // Point is inside Wall
			distanceNearestEdge = calculateDistanceNearestEdge(point);
			if (fabs(distanceNearestEdge) < THRESHOLD_BORDER)
				sharpness = 0.5 + distanceNearestEdge / (2.0 * THRESHOLD_BORDER);
			else
				sharpness = 1.0;
			return 1;                               // Point is inside the wall,
		}
		else
		{   // Point is outside Wall
			distanceNearestEdge = -calculateDistanceNearestEdge(point);
			if (fabs(distanceNearestEdge) < THRESHOLD_BORDER)
			{
				sharpness = 0.5 + distanceNearestEdge / (2.0 * THRESHOLD_BORDER);
				return 2;                           // Point is coming out of the wall
			}
			else 
			{
				sharpness = 0.0;
				return 0;                           // Point is outside Wall
			}
		}
	}

	float Wall::calculateDistanceNearestEdge(const Common::CVector3& point) const {
		float minDistance = 0.0, distance = 0.0;
		int n = polygon.size();
		for (auto i = 0; i < n; i++)
		{
			distance = calculateDistancePointToLine(point, polygon[i], polygon[(i + 1) % n]);
			if (i == 0) minDistance = distance;
			else
			{
				if (distance < minDistance) minDistance = distance;
			}
		}
		return(minDistance);
	}

	float Wall::calculateDistancePointToLine(const Common::CVector3& point, const Common::CVector3& pointLine1, const Common::CVector3& pointLine2) const
	{
		float distance = 0, vectorModulus;
		Common::CVector3 vector1, vector2, vector3;
		vector1 = pointLine2 - pointLine1;
		vector2 = point - pointLine1;
		vector3 = vector1.CrossProduct(vector2);
		//vectorModulus = vector3.GetDistance();
		distance = vector3.GetDistance() / vector1.GetDistance();
		return distance;
	}

	void Wall::calculate_ABCD()
	{
		Common::CVector3 normal;
		normal = getNormal();
		A = normal.x;
		B = normal.y;
		C = normal.z;
		D = -(A * polygon.at(2).x + B * polygon.at(2).y + C * polygon.at(2).z);
	}

}//namespace ISM
