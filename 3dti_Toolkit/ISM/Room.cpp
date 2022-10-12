#include "Room.h"
#include <cfloat>

namespace ISM
{

	void Room::setupShoeBox(float length, float width, float height)
	{
		//If the room was previously set up as a shoebox, it will keep the wall properties if it is redifined with a new shoeboxSetup
		std::vector<Wall> previousWalls;
		if (shoeBox)
		{
			previousWalls = walls;
		}

		// Now we can clear the walls in case there was a previous definition to set the room up from scratch
		walls.clear();
		Wall front, back, left, right, ceiling, floor;
		front.insertCorner(length / 2, width / 2, height / 2);
		front.insertCorner(length / 2, width / 2, -height / 2);
		front.insertCorner(length / 2, -width / 2, -height / 2);
		front.insertCorner(length / 2, -width / 2, height / 2);
		insertWall(front);
		left.insertCorner(-length / 2, width / 2, height / 2);
		left.insertCorner(-length / 2, width / 2, -height / 2);
		left.insertCorner(length / 2, width / 2, -height / 2);
		left.insertCorner(length / 2, width / 2, height / 2);
		insertWall(left);
		right.insertCorner(length / 2, -width / 2, height / 2);
		right.insertCorner(length / 2, -width / 2, -height / 2);
		right.insertCorner(-length / 2, -width / 2, -height / 2);
		right.insertCorner(-length / 2, -width / 2, height / 2);
		insertWall(right);
		back.insertCorner(-length / 2, -width / 2, height / 2);
		back.insertCorner(-length / 2, -width / 2, -height / 2);
		back.insertCorner(-length / 2, width / 2, -height / 2);
		back.insertCorner(-length / 2, width / 2, height / 2);
		insertWall(back);
		floor.insertCorner(length / 2, width / 2, -height / 2);
		floor.insertCorner(-length / 2, width / 2, -height / 2);
		floor.insertCorner(-length / 2, -width / 2, -height / 2);
		floor.insertCorner(length / 2, -width / 2, -height / 2);
		insertWall(floor);
		ceiling.insertCorner(length / 2, -width / 2, height / 2);
		ceiling.insertCorner(-length / 2, -width / 2, height / 2);
		ceiling.insertCorner(-length / 2, width / 2, height / 2);
		ceiling.insertCorner(length / 2, width / 2, height / 2);
		insertWall(ceiling);

		if (shoeBox)
		{
			for (int i = 0; i < previousWalls.size(); i++)
			{
				if (!previousWalls.at(i).isActive())
				{
					walls.at(i).disable();
				}
			}
		}

		shoeBox = true;
	}

	void Room::setupRoomGeometry(RoomGeometry roomGeometry)
	{
		walls.clear();
		for (int i = 0; i < roomGeometry.walls.size(); i++)
		{
			Wall tempWall;
			for (int j = 0; j < roomGeometry.walls.at(i).size(); j++)
			{
				tempWall.insertCorner(roomGeometry.corners.at(roomGeometry.walls.at(i).at(j)));
			}
			insertWall(tempWall);
		}
		shoeBox = false;
	}
		
	void Room::insertWall(Wall _newWall)
	{
		walls.push_back(_newWall);
	}

	void Room::enableWall(int wallIndex)
	{
		if (walls.size() > wallIndex)
		{
			walls.at(wallIndex).enable();
		}
	}

	void Room::disableWall(int wallIndex)
	{
		if (walls.size() > wallIndex)
		{
			walls.at(wallIndex).disable();
		}
	}

	void Room::setWallAbsortion(int wallIndex, float absortion)
	{
		walls.at(wallIndex).setAbsortion(absortion);
	}

	void Room::setWallAbsortion(int wallIndex, std::vector<float> absortionPerBand)
	{
		walls.at(wallIndex).setAbsortion(absortionPerBand);
	}

	std::vector<Wall> Room::getWalls()
	{
		return walls;
	}

	std::vector<Room> Room::getImageRooms()
	{
		std::vector<Room> roomList;
		for (int i = 0; i < walls.size(); i++)
		{
			if (walls.at(i).isActive())
			{
				Room tempRoom;
				for (int j = 0; j < walls.size(); j++)
				{
					Wall tempWall = walls.at(i).getImageWall(walls.at(j));
					tempRoom.insertWall(tempWall);
				}
				roomList.push_back(tempRoom);
			}
		}
		return roomList;
	}

	bool Room::checkPointInsideRoom(Common::CVector3 point, float &distanceNearestWall)
	{
		float distanceToPlane = FLT_MAX;
		bool inside;

		inside = true;
		for (int i = 0; i < walls.size(); i++)
		{
			if (walls.at(i).isActive())
			{

				Common::CVector3 normal, farthestCorner, center, p, p1, p2;
				center = walls.at(i).getCenter();
				farthestCorner = center;
				normal = walls.at(i).getNormal();

				std::vector<Common::CVector3> corners;
				Wall tWall = walls.at(i);
				corners = tWall.getCorners();

				float tempDistanceToPlane = walls.at(i).getDistanceFromPoint(point);
				if (tempDistanceToPlane < distanceToPlane) distanceToPlane = tempDistanceToPlane;
				
				double distance = 0.0, d;
				for (int j = 0; j < corners.size(); j++)
				{
					p.x = point.x - corners[j].x;
					p.y = point.y - corners[j].y;
					p.z = point.z - corners[j].z;
					d = p.GetDistance();
					if (d > distance)
					{
						distance = d;
						farthestCorner = corners[j];
					}
				}
				p = farthestCorner;
				//p = center;

				p1.x = p.x - point.x;
				p1.y = p.y - point.y;
				p1.z = p.z - point.z;

				p2.x = - normal.x;
				p2.y = - normal.y;
				p2.z = - normal.z;

				float dP;
				dP = p2.DotProduct(p1);
				if (dP < 0.0f)
     				inside = false;
			}
		}

		distanceNearestWall = distanceToPlane;
		return inside;
	}

	Common::CVector3 Room::getCenter()
	{
		Common::CVector3 center = Common::CVector3::ZERO;

		for (auto i = 0; i < walls.size(); i++)
		{
			center = center + walls.at(i).getCenter();
		}
		center.x /= walls.size();
		center.y /= walls.size();
		center.z /= walls.size();

		return center;
	}

} //namespace ISM
