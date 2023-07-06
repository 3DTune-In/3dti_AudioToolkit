#include "ISM.h"

//#define USE_PROFILER_3
#ifdef USE_PROFILER_3
#include <Windows.h>
#include "Common/Profiler.h"
Common::CProfilerDataSet dsProcessAbsortion;
Common::CProfilerDataSet dsProcessImageData;
Common::CProfilerDataSet dsProcessVisible;
#endif

namespace ISM
{
	
	CISM::CISM(Binaural::CCore* _ownerCore) :ownerCore{ _ownerCore }, reflectionOrder{ 1 },  maxDistanceSourcesToListener { 100 } {

		originalSource = make_shared<SourceImages>(this);

#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.InitProfiler();
		Common::PROFILER3DTI.SetAutomaticWrite(dsProcessAbsortion, "PROF_APP_PROCESSAbsortion.txt");
		Common::PROFILER3DTI.StartRelativeSampling(dsProcessAbsortion);
		Common::PROFILER3DTI.SetAutomaticWrite(dsProcessImageData, "PROF_APP_PROCESSImageData.txt");
		Common::PROFILER3DTI.StartRelativeSampling(dsProcessImageData);
		Common::PROFILER3DTI.SetAutomaticWrite(dsProcessVisible, "PROF_APP_PROCESSVisible.txt");
		Common::PROFILER3DTI.StartRelativeSampling(dsProcessVisible);
#endif
	}

	void CISM::SetupShoeBoxRoom(float length, float width, float height)
	{
		mainRoom.setupShoeBox(length, width, height);
		originalSource->createImages(mainRoom, reflectionOrder); 
	}

	void CISM::setupArbitraryRoom(RoomGeometry roomGeometry)
	{
		mainRoom.setupRoomGeometry(roomGeometry);
		originalSource->createImages(mainRoom, reflectionOrder); 
	}

	void CISM::setAbsortion(std::vector<float> absortionPerWall)
	{
		// Check if dimensions of input vctor and walls fit
		if (absortionPerWall.size() != mainRoom.getWalls().size())
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Size of vector of absortions per wall and numbar of walls are different");
			return;
		}

		for (int i = 0; i < mainRoom.getWalls().size(); i++)
		{
			mainRoom.setWallAbsortion(i, absortionPerWall.at(i));
		}
		originalSource->createImages(mainRoom, reflectionOrder);
	}

	void CISM::setAbsortion(std::vector<std::vector<float>> absortionPerBandPerWall)
	{
		// Check the number of bands and the number of walls
		if (absortionPerBandPerWall.size() != mainRoom.getWalls().size())
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Size of vector of absortion profiles per wall and numbar of walls are different");
			return;
		}

		for (int i = 0; i < mainRoom.getWalls().size(); i++)
		{
			mainRoom.setWallAbsortion(i, absortionPerBandPerWall.at(i));
		}
		originalSource->createImages(mainRoom, reflectionOrder); 

	}
	
	Room CISM::getRoom()
	{
		return mainRoom;
	}

	void CISM::enableWall(int wallIndex)
	{
		mainRoom.enableWall(wallIndex);
		originalSource->createImages(mainRoom, reflectionOrder); 
	}

	void CISM::disableWall(int wallIndex)
	{
		mainRoom.disableWall(wallIndex);
		originalSource->createImages(mainRoom, reflectionOrder); 
	}

	void CISM::setReflectionOrder(int _reflectionOrder)
	{
		reflectionOrder = _reflectionOrder;
		originalSource->createImages(mainRoom, reflectionOrder); 
	}

	int CISM::getReflectionOrder()
	{
		return reflectionOrder;
	}

	void CISM::setMaxDistanceImageSources(float _MaxDistanceSourcesToListener)
	{
		maxDistanceSourcesToListener = _MaxDistanceSourcesToListener;	
		originalSource->createImages(mainRoom, reflectionOrder); 
	}

	int CISM::calculateNumOfSilencedFrames(float maxDistanceSourcesToListener)
	{
		float buffersize = (float) ownerCore->GetAudioState().bufferSize;
		float samplerate = (float) ownerCore->GetAudioState().sampleRate;
		float soundSpeed = ownerCore->GetMagnitudes().GetSoundSpeed();

		int numberOfSlilencedFrames = ceil (((maxDistanceSourcesToListener / soundSpeed)*samplerate) / buffersize);

		return numberOfSlilencedFrames;
	}
			
	float CISM::getMaxDistanceImageSources()
	{
		return maxDistanceSourcesToListener;
	}

	void CISM::setSourceLocation(Common::CVector3 location)
	{
		originalSource->setLocation(location);

	}

	Common::CVector3 CISM::getSourceLocation()
	{
		return originalSource->getLocation();
	}

	std::vector<Common::CVector3> CISM::getImageSourceLocations()
	{		
		std::vector<Common::CVector3> imageSourceList;
		originalSource->getImageLocations(imageSourceList);
		return imageSourceList;
	}

	std::vector<ISM::ImageSourceData> CISM::getImageSourceData()
	{
		std::vector<ImageSourceData> imageSourceList;
		originalSource->getImageData(imageSourceList);
		return imageSourceList;
	}

	void CISM::proccess(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation)
	{
#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.RelativeSampleStart(dsProcessAbsortion);
#endif
		originalSource->processAbsortion(inBuffer, imageBuffers, listenerLocation);

#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.RelativeSampleEnd(dsProcessAbsortion);
#endif
		
#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.RelativeSampleStart(dsProcessImageData);
#endif
		std::vector<ImageSourceData> images = getImageSourceData();
#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.RelativeSampleEnd(dsProcessImageData);
#endif
		ASSERT(imageBuffers.size() == images.size(), RESULT_ERROR_BADSIZE, "Vector of buffers to be processed by ISM should be the same size as the number of image sources", "");
#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.RelativeSampleStart(dsProcessVisible);
#endif
		//int fuentes = 0;
		for (int i = 0; i < imageBuffers.size(); i++)
		{
			if (images.at(i).visible)
			{
				//fuentes++;
				for (int j = 0; j < inBuffer.size(); j++)
				{
					imageBuffers.at(i).at(j) = images.at(i).visibility * imageBuffers.at(i).at(j);
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
		//cout << fuentes << endl;
#ifdef USE_PROFILER_3
		Common::PROFILER3DTI.RelativeSampleEnd(dsProcessVisible);
#endif
	
	}

	Binaural::CCore* CISM::GetCore() const{
		return ownerCore;
	}

	float CISM::GetSampleRate() {
		return ownerCore->GetAudioState().sampleRate;
	}

	shared_ptr<Binaural::CListener> CISM::GetListener() const {
		return ownerCore->GetListener();
	}

}//namespace ISM


