#include "ISM.h"

namespace ISM
{
	
	ISM::ISM(Binaural::CCore* _ownerCore):ownerCore{ _ownerCore } {

	}

	void ISM::SetupShoeBoxRoom(float length, float width, float height)
	{
		mainRoom.setupShoeBox(length, width, height);
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

	void ISM::setAbsortion(std::vector<std::vector<float>> absortionsBands)
	{
		for (int i = 0; i < mainRoom.getWalls().size(); i++)
		{
			mainRoom.setWallAbsortion(i, absortionsBands.at(i));
		}
		originalSource.createImages(mainRoom, Common::CVector3(0, 0, 0), reflectionOrder); //FIXME:the listener location is fake

	}
	
	Room ISM::getRoom()
	{
		return mainRoom;
	}

	void ISM::enableWall(int wallIndex, Common::CVector3 _lisenerLocation)
	{
		mainRoom.enableWall(wallIndex);
		originalSource.createImages(mainRoom, _lisenerLocation, reflectionOrder); //FIXME:the listener location is fake
	}

	void ISM::disableWall(int wallIndex, Common::CVector3 _lisenerLocation)
	{
		mainRoom.disableWall(wallIndex);
		originalSource.createImages(mainRoom, _lisenerLocation, reflectionOrder); //FIXME:the listener location is fake
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

	void ISM::setSourceLocation(Common::CVector3 location,Common::CVector3 listenerLocation)
	{
		originalSource.setLocation(location,listenerLocation);

	}

	Common::CVector3 ISM::getSourceLocation()
	{
		return originalSource.getLocation();
	}


	std::vector<Common::CVector3> ISM::getImageSourceLocations()
	{		
		std::vector<Common::CVector3> imageSourceList;
		originalSource.getImageLocations(imageSourceList);
		return imageSourceList;
	}


	std::vector<ImageSourceData> ISM::getImageSourceData(Common::CVector3 listenerLocation)
	{
		std::vector<ImageSourceData> imageSourceList;
		originalSource.getImageData(imageSourceList, listenerLocation);
		return imageSourceList;
	}

	void ISM::proccess(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation)
	{
		originalSource.processAbsortion(inBuffer, imageBuffers, listenerLocation);

		
		std::vector<ImageSourceData> images = getImageSourceData(listenerLocation);
		ASSERT(imageBuffers.size() == images.size(), RESULT_ERROR_BADSIZE, "Vector of buffers to be processed by ISM should be the same size as the number of image sources", "");
		
		for (int i = 0; i < imageBuffers.size(); i++)
		{
			if (images.at(i).visible)
			{
				for (int j = 0; j < inBuffer.size(); j++)
				{
					imageBuffers.at(i).at(j) = images.at(i).visibility*imageBuffers.at(i).at(j);
				}
			}
			else
			{
				for (int j = 0; j < inBuffer.size(); j++)
				{
					imageBuffers.at(i).at(j) = 0.0f;
				}
			}
		}
		
	}

	Binaural::CCore* ISM::GetCore() const{
		return ownerCore;
	}

	float ISM::GetSampleRate() {
		return ownerCore->GetAudioState().sampleRate;
	}

	shared_ptr<Binaural::CListener> ISM::GetListener() const {
		return ownerCore->GetListener();
	}

}//namespace ISM


