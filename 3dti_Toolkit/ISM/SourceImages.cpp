#include "SourceImages.h"
#include "ISM.h"
#include <cmath>
#include <Common/BiquadFilter.h>

//#ifndef THRESHOLD_BORDER
//#define THRESHOLD_BORDER 0.2f
//#endif

namespace ISM
{
	SourceImages::SourceImages(ISM::CISM* _ownerISM) : ownerISM{_ownerISM} {
			
	}

	void SourceImages::setLocation(Common::CVector3 _location)
	{
		sourceLocation = _location;
		updateImages();
	}

	Common::CVector3 SourceImages::getLocation()
	{
		return sourceLocation;
	}

	std::vector<weak_ptr <SourceImages>> SourceImages::getImages()
	{
		vector<weak_ptr<SourceImages>> result;
		for (auto i = 0; i < images.size(); ++i) {
			result.push_back(weak_ptr<SourceImages>(images[i]));
		}
		return result;
	}

	void SourceImages::getImageLocations(std::vector<Common::CVector3> &imageSourceList)
	{
			for (int i = 0; i < images.size(); i++)
			{
				if (images.at(i)->reflectionWalls.back().isActive())
				{
					imageSourceList.push_back(images.at(i)->getLocation());
					images.at(i)->getImageLocations(imageSourceList);
				}
			}
	}


	void SourceImages::getImageData(std::vector<ImageSourceData> &imageSourceDataList)
	{
		for (int i = 0; i < images.size(); i++)
		{
			ImageSourceData temp;
			temp.location = images.at(i)->getLocation();
			temp.reflectionWalls = images.at(i)->reflectionWalls;
			temp.reflectionBands = images.at(i)->reflectionBands;
			temp.visibility = images.at(i)->visibility;
			temp.visible = images.at(i)->visible;
			imageSourceDataList.push_back(temp);  //Once created, the image source data is added to the list

			images.at(i)->getImageData(imageSourceDataList); //recurse to the next level
		}
	}

	//ARCADIO: is this needed? it seems it is not used
	void SourceImages::setReflectionWalls(std::vector<Wall> _reflectionWalls)
	{
		reflectionWalls = _reflectionWalls;
	}

	Wall SourceImages::getReflectionWall()
	{
		return reflectionWalls.back();
	}

	void SourceImages::createImages(Room _room, int reflectionOrder)
	{
		images.clear();	
		int equalizerType = ownerISM->getEqualizerType();
		if (equalizerType != 0 && equalizerType != 1)
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");
		createImages(_room, reflectionOrder, reflectionWalls, equalizerType);
		updateImages();
	}

	void SourceImages::createImages(Room _room, int reflectionOrder, std::vector<Wall> reflectionWalls, int equalizerType)
	{
		Common::CVector3 roomCenter = ownerISM->getRoom().getCenter();
		Common::CTransform listenerTransform = ownerISM->GetListener()->GetListenerTransform();
		Common::CVector3 listenerLocation = listenerTransform.GetPosition();

		if (reflectionOrder > 0) //if the reflection order is already 0 no more images should be created. We are at the leaves of the tree
		{
			reflectionOrder--;
			std::vector<Wall> walls = _room.getWalls();
			for (int i = 0; i < walls.size(); i++)  //for each wall an image is created
			{
				if (walls.at(i).isActive()) //if the wall is not active, its image is not created
				{
					shared_ptr<SourceImages> tempSourceImage = make_shared< SourceImages>(ownerISM);
					Common::CVector3 tempImageLocation = walls[i].getImagePoint(sourceLocation);

					// if the image is closer to the room center than the previous original, that reflection is not real and should not be included
					// this is equivalent to determine wether source and room center are on the same side of the wall or not
					if (((roomCenter - sourceLocation).GetDistance() < (roomCenter - tempImageLocation).GetDistance()))
					{
						// If the image room where the candidate image source is far from the original room, so that any location in it is further than
						// the maximum distance to any location in the original room, then the recursive tree has to stop growing

						//if there are no reflection walls, the minimum distance is 0 and it is not necessary to calculate it. This way, -
						//we avoid to get wall from the reflectionWalls vector, which is empty.
						float roomsDistance = 0.0;
						if(reflectionWalls.size()>0) 
						{
							// the distance criterion can be static or dynamic
							if (ownerISM->staticDistanceCriterion == false)
								roomsDistance = walls.at(i).getMinimumDistanceFromWall(reflectionWalls.front());
							else
							    roomsDistance = (listenerLocation - tempImageLocation).GetDistance();
						}

						// the distance criterion can be static or dynamic
						float maxDistanceImageSources;
						if (ownerISM->staticDistanceCriterion == false)
							maxDistanceImageSources = ownerISM->getMaxDistanceImageSources();
						else
							maxDistanceImageSources = ownerISM->getMaxDistanceImageSources() + ownerISM->transitionMeters * 0.5;

						if (roomsDistance <= maxDistanceImageSources)
						{
							//The new candidate meets all requirements and will be a source image. It is therefor finally completed
							tempSourceImage->setLocation(tempImageLocation);
							reflectionWalls.push_back(walls.at(i));
							tempSourceImage->reflectionWalls = reflectionWalls;

							if (equalizerType == CASCADE)        
								tempSourceImage->FilterChain.RemoveFilters();
							else if (equalizerType == PARALLEL)  
								tempSourceImage->FilterBank.RemoveFilters();
							else
								SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");

							////////////////////// Set up an equalisation filterbank to simulate frequency dependent absortion
							float samplingFrec = ownerISM->GetSampleRate();
							float bandFrequency = FIRST_ABSORTION_BAND;						//frecuency of each band. We start with the first band
							float octaveStepPow = 2.0;
							float Q_BPF = std::sqrt(octaveStepPow) / (octaveStepPow - 1.0f); // We are using a constant Q filter bank. Q=sqrt(2)

							std::vector<float> tempReflectionCoefficients(NUM_BAND_ABSORTION, 1.0);	//creates band reflection coeffs and initialise them to 1.0
							tempSourceImage->reflectionBands = tempReflectionCoefficients ;

							float gMeanLinear;           // only for Cascade
							if (equalizerType == CASCADE) 
							{
								// Absorption values are expressed in dBs
								int nf, nc;
								for (nc = 0; nc < NUM_BAND_ABSORTION; nc++) 
								{
									float R = walls.at(i).getAbsortionB().at(nc);
									gdB[nc] = 20 * log10(1-R*R);
								}
								//average gain in dB
								float gMeandB = 0;
								for (nc = 0; nc < NUM_BAND_ABSORTION; nc++) gMeandB += gdB[nc];
								gMeandB /= NUM_BAND_ABSORTION;
								for (nc = 0; nc < NUM_BAND_ABSORTION; nc++) gdB[nc] -= gMeandB;
								
								// Command gains are calculated
								for (nf = 0; nf < NUM_BAND_ABSORTION; nf++) {
									gCmd[nf] = 0;
									for (nc = 0; nc < NUM_BAND_ABSORTION; nc++)
										gCmd[nf] += Room::inverseBmatrix[nf][nc] * gdB[nc];
								}
								// Command gains are expressed in linear mode
								for (nc = 0; nc < NUM_BAND_ABSORTION; nc++)
									gCmd[nc] = pow(10.0, ((gCmd[nc]) / 20.0));
								gMeanLinear = pow(10.0, ((gMeandB) / 20.0));
								//float  gCmd[NUM_BAND_ABSORTION] = { 2.5546,  2.4005, 0.4056, 2.4646, 3.3480, 0.1198, 1.3390, 0.1377, 3.9254 };
							}
							

							for (int k = 0; k < NUM_BAND_ABSORTION; k++)
							{
								shared_ptr<Common::CBiquadFilter> filter;
								if (equalizerType == CASCADE)
									filter = tempSourceImage->FilterChain.AddFilter();
								else if (equalizerType == PARALLEL)
									filter = tempSourceImage->FilterBank.AddFilter();
								else
									SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");

								if (k == 0)
									if (equalizerType == CASCADE)
										filter->Setup(samplingFrec, bandFrequency * Q_BPF, 1 / Q_BPF, Common::T_filterType::LOWSHELF, gCmd[k]);
									else if (equalizerType == PARALLEL)
										filter->Setup(samplingFrec, bandFrequency * Q_BPF, 1 / Q_BPF, Common::T_filterType::LOWPASS);
									else
										SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");
								else if (k < NUM_BAND_ABSORTION - 1)
									if (equalizerType == CASCADE)
										filter->Setup(samplingFrec, bandFrequency, Q_BPF, Common::T_filterType::PEAKNOCH, gCmd[k]);
									else if (equalizerType == PARALLEL)
										filter->Setup(samplingFrec, bandFrequency, Q_BPF, Common::T_filterType::BANDPASS);
									else
										SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");
								else
									if (equalizerType == CASCADE) {
										filter->Setup(samplingFrec, bandFrequency / Q_BPF, 1 / Q_BPF, Common::T_filterType::HIGHSHELF, gCmd[k]);
										filter->SetGeneralGain(gMeanLinear);
									}
									else if (equalizerType == PARALLEL)
										filter->Setup(samplingFrec, bandFrequency / Q_BPF, 1 / Q_BPF, Common::T_filterType::HIGHPASS);
									else
										SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");
								
								CMonoBuffer<float> tempBuffer(1, 0.0);		// A minimal process with a one sample buffer is carried out to make the coeficients stable
								filter->Process(tempBuffer);				// and avoid crossfading at the begining.


								//Set the reflection coefficient of each band according to absortion coeficients of reflectionWalls
								for (int j = 0; j < reflectionWalls.size(); j++)
								{
									tempSourceImage->reflectionBands[k] *= sqrt(1 - reflectionWalls.at(j).getAbsortionB().at(k));
								}
							    if (equalizerType == PARALLEL)
								    filter->SetGeneralGain(tempSourceImage->reflectionBands.at(k));	//FIXME: the gain per band is dulicated (inside the filters and  in reflectionBands attribute
								
								bandFrequency *= octaveStepPow;
							}
							/////////////////////////

							if (reflectionOrder > 0)  //Still higher order reflections: we need to create images of the image just created
							{
								// We need to calculate the image room before asking for all the new images
								Room tempRoom;
								for (int j = 0; j < walls.size(); j++)
								{
									Wall tempWall = walls.at(i).getImageWall(walls.at(j));
									tempRoom.insertWall(tempWall);
								}
								tempSourceImage->createImages(tempRoom, reflectionOrder, reflectionWalls, equalizerType);
							}
							images.push_back(tempSourceImage);
							reflectionWalls.pop_back();
						}
					}
				}
			}
		}
	}

	void SourceImages::updateImages()
	{	
		Common::CTransform listenerTransform = ownerISM->GetListener()->GetListenerTransform();
		Common::CVector3 listenerLocation = listenerTransform.GetPosition();

		float distanceMargin = ownerISM->transitionMeters;
		distanceMargin *= 1; // METERS_OF_MARGIN;
				
		for (int i = 0; i < images.size(); i++)
		{
			images[i]->setLocation(images.at(i)->getReflectionWall().getImagePoint(sourceLocation));
		}

		//Check visibility through all reflection walls and compute a visibility coeficient
		visibility = 1.0;	//We hypothesise that it is fully visible. Otherwise, this will become lower
		visible = true;

		Common::CVector3 roomCenter = ownerISM->getRoom().getCenter();
	
		float distanceImageToLisener, distAux1;
		distanceImageToLisener = (listenerLocation - sourceLocation).GetDistance();
		float maxDistanceSourcesToListener = ownerISM->getMaxDistanceImageSources();
			
		if (distanceImageToLisener > (maxDistanceSourcesToListener + distanceMargin * 0.5))
		{
			visible = false; 
			visibility = 0.0;
		}
		else 
		{
			for (int j = 0; j < reflectionWalls.size(); j++)
			{
				Common::CVector3 reflectionPoint = reflectionWalls.at(j).getIntersectionPointWithLine(sourceLocation, listenerLocation);
				float distanceToBorder, wallVisibility;
				reflectionWalls.at(j).checkPointInsideWall(reflectionPoint, distanceToBorder, wallVisibility);
				visibility *= wallVisibility;
				visible &= (wallVisibility > 0);
			}
			visibility = pow(visibility, (1 / (float)reflectionWalls.size())); 
			if (distanceImageToLisener > (maxDistanceSourcesToListener - distanceMargin * 0.5))
			{
				visibility *= 0.5 + 0.5 * cos(PI * (distanceImageToLisener - (maxDistanceSourcesToListener - distanceMargin * 0.5)) / distanceMargin);
			}
		}
	}

	void SourceImages::processAbsortion(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>>& imageBuffers, Common::CVector3 listenerLocation, int equalizerType)
	{
		for (int i = 0; i < images.size(); i++)  //process buffers for each of the image sources, adding the result to the output vector of buffers
		{
			CMonoBuffer<float> tempBuffer(inBuffer.size(), 0.0);

			if (images.at(i)->visibility > 0.00001)
				if (equalizerType == CASCADE)
					images.at(i)->FilterChain.Process(inBuffer, tempBuffer);
				else if (equalizerType == PARALLEL)
					images.at(i)->FilterBank.Process(inBuffer, tempBuffer);
				else
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "equalizerType");
			imageBuffers.push_back(tempBuffer);
			images.at(i)->processAbsortion(inBuffer, imageBuffers, listenerLocation, equalizerType);
		}
	}


}//namespace ISM
