#include "ISM.h"

namespace ISM
{

	void ISM::SetupShoeBoxRoom(float length, float width, float height)
	{
		mainRoom.setupShoebox(length, width, height);
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //FIXME:the listener location is fake
	}

	void ISM::setupArbitraryRoom(RoomGeometry roomGeometry)
	{
		mainRoom.setupRoomGeometry(roomGeometry);
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //FIXME:the listener location is fake
	}

	void ISM::setAbsortion(std::vector<float> absortions)
	{
		for (int i = 0; i < mainRoom.getWalls().size(); i++)
		{
			mainRoom.setWallAbsortion(i, absortions.at(i));
		}
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //FIXME:the listener location is fake
	}
	
	Room ISM::getRoom()
	{
		return mainRoom;
	}

	void ISM::enableWall(int wallIndex)
	{
		mainRoom.enableWall(wallIndex);
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //FIXME:the listener location is fake
	}

	void ISM::disableWall(int wallIndex)
	{
		mainRoom.disableWall(wallIndex);
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //FIXME:the listener location is fake
	}

	void ISM::setReflectionOrder(int _reflectionOrder)
	{
		reflectionOrder = _reflectionOrder;
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //(0,0,0) is used instead of listener location. We should consider to change listener location by the center of the room
	}

	int ISM::getReflectionOrder()
	{
		return reflectionOrder;
	}

	void ISM::setSourceLocation(Common::CVector3 location)
	{
		originalSource.setLocation(location);

	}

	Common::CVector3 ISM::getSourceLocation()
	{
		return originalSource.getLocation();
	}


	std::vector<Common::CVector3> ISM::getImageSourceLocations()
	{
		std::vector<Common::CVector3> imageSourceList;
		originalSource.getImageLocations(imageSourceList, reflectionOrder);
		return imageSourceList;
	}


	std::vector<ImageSourceData> ISM::getImageSourceData(Common::CVector3 listenerLocation)
	{
		std::vector<ImageSourceData> imageSourceList;
		originalSource.getImageData(imageSourceList, listenerLocation, reflectionOrder);
		return imageSourceList;
	}

	void ISM::proccess(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation)
	{
		std::vector<ImageSourceData> images = getImageSourceData(listenerLocation);
		for (int i = 0; i < imageBuffers.size(); i++)
		{
			if (images.at(i).visibility)
			{
				for (int j = 0; j < inBuffer.size(); j++)
				{
					imageBuffers.at(i).at(j) = images.at(i).reflection*inBuffer.at(j);
				}
			}
		}
	}

}//namespace ISM


