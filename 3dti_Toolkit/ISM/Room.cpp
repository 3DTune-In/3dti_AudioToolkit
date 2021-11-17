#include "Room.h"

void Room::setupShoebox(float length, float width, float height)
{
	Wall front,back,left,right,ceiling,floor;
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

