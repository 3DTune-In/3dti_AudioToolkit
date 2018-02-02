/**
* \class CLowHighSplitFilter
*
* \brief Implementation of CHighOrderButterworthFilter class. This class implements different types of high order filters which coefficients are in hardcoded tables.
*
* \date	October 2017
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2018
*
* \b Licence: This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#include <HAHLSimulation/HighOrderButterworthFilter.h>
#include <Common/BiquadFilter.h>

namespace HAHLSimulation {

	void CHighOrderButterworthFilter::Setup(int samplingRate, int splitFrequency, Common::T_filterType filterType, int filterOrder)
	{
		// TO DO: check errors... (bad size of coefficients in hardcoded table...)

		// 1. Choose table and fill it with hardcoded coefficients (if not already done)	
		coefficientsTable = nullptr;
		if ((filterType == Common::T_filterType::LOWPASS) && (filterOrder == 6))
		{
			coefficientsTable = &lowPassFilterOrder6Table;
			FillLowPassFilterOrder6CoefficientsTable();
		}
		if ((filterType == Common::T_filterType::LOWPASS) && (filterOrder == 4))
		{
			coefficientsTable = &lowPassFilterOrder4Table;
			FillLowPassFilterOrder4CoefficientsTable();
		}
		if ((filterType == Common::T_filterType::HIGHPASS) && (filterOrder == 4))
		{
			coefficientsTable = &highPassFilterOrder4Table;
			FillHighPassFilterOrder4CoefficientsTable();
		}

		ASSERT(coefficientsTable != nullptr, RESULT_ERROR_CASENOTDEFINED, "Attempt to setup a high order butterworth filter type and order for which there are no hardcoded coefficients", "");

		// 2. Get the coefficients from the hardcoded table and set filters
		SetFilterCoefficients(samplingRate, splitFrequency);
	}

	/////////////////////////////////////////////////////
	void CHighOrderButterworthFilter::Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		outputBuffer = inputBuffer;
		filtersChain.Process(outputBuffer);
	}

	/////////////////////////////////////////////////////
	// Set split frequency
	void CHighOrderButterworthFilter::SetFilterCoefficients(int samplingRate, int cutoffFrequency)
	{
		Common::TFiltersChainCoefficients coefficients;

		// Find coefficients
		auto it = coefficientsTable->find(THighOrderFilterParameters(samplingRate, cutoffFrequency));
		if (it != coefficientsTable->end())
		{
			coefficients = it->second;
		}
		else
		{
			SET_RESULT(RESULT_ERROR_NOTSET, "High order filter coefficients not set for this sampling rate and cutoff frequency");
			return;
		}

		// Setup filters if coefficients were found
		filtersChain.SetFromCoefficientsVector(coefficients);
	}

	/////////////////////////////////////////////////////
	// Fill hardcoded coefficients table for 4th order LPF		
	static void FillLowPassFilterOrder4CoefficientsTable()
	{
		// TO DO: check return values of calls to "emplace"

		// 1. Check if table was already filled
		if (isLowPassFilterOrder4TableFilled)
			return;
		else
			isLowPassFilterOrder4TableFilled = true;

		// 2. Clear table
		lowPassFilterOrder4Table.clear();

		// 3. Create coefficients vectors

		// sr = 44100; sf = 200
		Common::TFiltersChainCoefficients coefs_44100_200;
		coefs_44100_200.clear();
		coefs_44100_200.push_back({ 0.00020079f, 0.00040158f, 0.00020079f, -1.97762572f, 0.97842888f });
		coefs_44100_200.push_back({ 0.00019777f, 0.00039555f, 0.00019777f, -1.94791403f, 0.94870513f });

		// sr = 44100; sf = 400
		Common::TFiltersChainCoefficients coefs_44100_400;
		coefs_44100_400.clear();
		coefs_44100_400.push_back({ 0.00079444f, 0.00158888f, 0.00079444f, -1.95415732f, 0.95733507f });
		coefs_44100_400.push_back({ 0.00077117f, 0.00154235f, 0.00077117f, -1.89692950f, 0.90001420f });

		// sr = 44100; sf = 800
		Common::TFiltersChainCoefficients coefs_44100_800;
		coefs_44100_800.clear();
		coefs_44100_800.push_back({ 0.00310907f, 0.00621813f, 0.00310907f, -1.90414615f, 0.91658241f });
		coefs_44100_800.push_back({ 0.00293589f, 0.00587178f, 0.00293589f, -1.79808579f, 0.80982936f });

		// sr = 44100; sf = 1600
		Common::TFiltersChainCoefficients coefs_44100_1600;
		coefs_44100_1600.clear();
		coefs_44100_1600.push_back({ 0.01190578f, 0.02381156f, 0.01190578f, -1.79317807f, 0.84080119f });
		coefs_44100_1600.push_back({ 0.01070115f, 0.02140229f, 0.01070115f, -1.61174323f, 0.65454782f });

		// sr = 44100; sf = 3200
		Common::TFiltersChainCoefficients coefs_44100_3200;
		coefs_44100_3200.clear();
		coefs_44100_3200.push_back({ 0.04370798f, 0.08741597f, 0.04370798f, -1.53677575f, 0.71160768f });
		coefs_44100_3200.push_back({ 0.03630460f, 0.07260920f, 0.03630460f, -1.27647222f, 0.42169061f });

		// sr = 44100; sf = 6400
		Common::TFiltersChainCoefficients coefs_44100_6400;
		coefs_44100_6400.clear();
		coefs_44100_6400.push_back({ 0.14882671f, 0.29765342f, 0.14882671f, -0.94012760f, 0.53543445f });
		coefs_44100_6400.push_back({ 0.11202634f, 0.22405268f, 0.11202634f, -0.70766233f, 0.15576770f });

		// sr = 44100; sf = 12800
		Common::TFiltersChainCoefficients coefs_44100_12800;
		coefs_44100_12800.clear();
		coefs_44100_12800.push_back({ 0.45610997f, 0.91221994f, 0.45610997f, 0.36513031f, 0.45930957f });
		coefs_44100_12800.push_back({ 0.32995836f, 0.65991671f, 0.32995836f, 0.26414200f, 0.05569143f });

		// sr = 48000; sf = 200
		Common::TFiltersChainCoefficients coefs_48000_200;
		coefs_48000_200.clear();
		coefs_48000_200.push_back({ 0.00016964f, 0.00033928f, 0.00016964f, -1.97948519f, 0.98016374f });
		coefs_48000_200.push_back({ 0.00016729f, 0.00033458f, 0.00016729f, -1.95210428f, 0.95277345f });

		// sr = 48000; sf = 400
		Common::TFiltersChainCoefficients coefs_48000_400;
		coefs_48000_400.clear();
		coefs_48000_400.push_back({ 0.00067178f, 0.00134356f, 0.00067178f, -1.95804318f, 0.96073029f });
		coefs_48000_400.push_back({ 0.00065363f, 0.00130726f, 0.00065363f, -1.90514144f, 0.90775596f });

		// sr = 48000; sf = 800
		Common::TFiltersChainCoefficients coefs_48000_800;
		coefs_48000_800.clear();
		coefs_48000_800.push_back({ 0.00263370f, 0.00526740f, 0.00263370f, -1.91253970f, 0.92307450f });
		coefs_48000_800.push_back({ 0.00249783f, 0.00499566f, 0.00249783f, -1.81387480f, 0.82386613f });

		// sr = 48000; sf = 1600
		Common::TFiltersChainCoefficients coefs_48000_1600;
		coefs_48000_1600.clear();
		coefs_48000_1600.push_back({ 0.01012093f, 0.02024187f, 0.01012093f, -1.81211540f, 0.85259914f });
		coefs_48000_1600.push_back({ 0.00916562f, 0.01833124f, 0.00916562f, -1.64106974f, 0.67773221f });

		// sr = 48000; sf = 3200
		Common::TFiltersChainCoefficients coefs_48000_3200;
		coefs_48000_3200.clear();
		coefs_48000_3200.push_back({ 0.03740511f, 0.07481023f, 0.03740511f, -1.58100527f, 0.73062573f });
		coefs_48000_3200.push_back({ 0.03142029f, 0.06284058f, 0.03142029f, -1.32804422f, 0.45372538f });

		// sr = 48000; sf = 6400
		Common::TFiltersChainCoefficients coefs_48000_6400;
		coefs_48000_6400.clear();
		coefs_48000_6400.push_back({ 0.12880418f, 0.25760836f, 0.12880418f, -1.04194367f, 0.55716038f });
		coefs_48000_6400.push_back({ 0.09808907f, 0.19617814f, 0.09808907f, -0.79347802f, 0.18583429f });

		// sr = 48000; sf = 12800
		Common::TFiltersChainCoefficients coefs_48000_12800;
		coefs_48000_12800.clear();
		coefs_48000_12800.push_back({ 0.40002130f, 0.80004261f, 0.40002130f, 0.15142611f, 0.44865910f });
		coefs_48000_12800.push_back({ 0.28781474f, 0.57562949f, 0.28781474f, 0.10895086f, 0.04230811f });

		// sr = 96000; sf = 200
		Common::TFiltersChainCoefficients coefs_96000_200;
		coefs_96000_200.clear();
		coefs_96000_200.push_back({ 0.00004262f, 0.00008525f, 0.00004262f, -1.98986110f, 0.99003159f });
		coefs_96000_200.push_back({ 0.00004232f, 0.00008465f, 0.00004232f, -1.97593328f, 0.97610258f });

		// sr = 96000; sf = 400
		Common::TFiltersChainCoefficients coefs_96000_400;
		coefs_96000_400.clear();
		coefs_96000_400.push_back({ 0.00016964f, 0.00033928f, 0.00016964f, -1.97948519f, 0.98016374f });
		coefs_96000_400.push_back({ 0.00016729f, 0.00033458f, 0.00016729f, -1.95210428f, 0.95277345f });

		// sr = 96000; sf = 800
		Common::TFiltersChainCoefficients coefs_96000_800;
		coefs_96000_800.clear();
		coefs_96000_800.push_back({ 0.00067178f, 0.00134356f, 0.00067178f, -1.95804318f, 0.96073029f });
		coefs_96000_800.push_back({ 0.00065363f, 0.00130726f, 0.00065363f, -1.90514144f, 0.90775596f });

		// sr = 96000; sf = 1600
		Common::TFiltersChainCoefficients coefs_96000_1600;
		coefs_96000_1600.clear();
		coefs_96000_1600.push_back({ 0.00263370f, 0.00526740f, 0.00263370f, -1.91253970f, 0.92307450f });
		coefs_96000_1600.push_back({ 0.00249783f, 0.00499566f, 0.00249783f, -1.81387480f, 0.82386613f });

		// sr = 96000; sf = 3200
		Common::TFiltersChainCoefficients coefs_96000_3200;
		coefs_96000_3200.clear();
		coefs_96000_3200.push_back({ 0.01012093f, 0.02024187f, 0.01012093f, -1.81211540f, 0.85259914f });
		coefs_96000_3200.push_back({ 0.00916562f, 0.01833124f, 0.00916562f, -1.64106974f, 0.67773221f });

		// sr = 96000; sf = 6400
		Common::TFiltersChainCoefficients coefs_96000_6400;
		coefs_96000_6400.clear();
		coefs_96000_6400.push_back({ 0.03740511f, 0.07481023f, 0.03740511f, -1.58100527f, 0.73062573f });
		coefs_96000_6400.push_back({ 0.03142029f, 0.06284058f, 0.03142029f, -1.32804422f, 0.45372538f });

		// sr = 96000; sf = 12800
		Common::TFiltersChainCoefficients coefs_96000_12800;
		coefs_96000_12800.clear();
		coefs_96000_12800.push_back({ 0.12880418f, 0.25760836f, 0.12880418f, -1.04194367f, 0.55716038f });
		coefs_96000_12800.push_back({ 0.09808907f, 0.19617814f, 0.09808907f, -0.79347802f, 0.18583429f });

		// 4. Emplace coefficient vectors in table 				
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 200), coefs_44100_200);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 400), coefs_44100_400);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 800), coefs_44100_800);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 1600), coefs_44100_1600);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 3200), coefs_44100_3200);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 6400), coefs_44100_6400);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 12800), coefs_44100_12800);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 200), coefs_48000_200);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 400), coefs_48000_400);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 800), coefs_48000_800);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 1600), coefs_48000_1600);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 3200), coefs_48000_3200);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 6400), coefs_48000_6400);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 12800), coefs_48000_12800);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 200), coefs_96000_200);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 400), coefs_96000_400);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 800), coefs_96000_800);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 1600), coefs_96000_1600);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 3200), coefs_96000_3200);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 6400), coefs_96000_6400);
		lowPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 12800), coefs_96000_12800);
	}

	/////////////////////////////////////////////////////
	// Fill hardcoded coefficients table for 4th order HPF		
	static void FillHighPassFilterOrder4CoefficientsTable()
	{
		// TO DO: check return values of calls to "emplace"

		// 1. Check if table was already filled
		if (isHighPassFilterOrder4TableFilled)
			return;
		else
			isHighPassFilterOrder4TableFilled = true;

		// 2. Clear table
		highPassFilterOrder4Table.clear();

		// 3. Create coefficients vectors

		// sr = 44100; sf = 200
		Common::TFiltersChainCoefficients coefs_44100_200;
		coefs_44100_200.clear();
		coefs_44100_200.push_back({ 0.98901365f, -1.97802730f, 0.98901365f, -1.97762572f, 0.97842888f });
		coefs_44100_200.push_back({ 0.97415479f, -1.94830958f, 0.97415479f, -1.94791403f, 0.94870513f });

		// sr = 44100; sf = 400
		Common::TFiltersChainCoefficients coefs_44100_400;
		coefs_44100_400.clear();
		coefs_44100_400.push_back({ 0.97787310f, -1.95574620f, 0.97787310f, -1.95415732f, 0.95733507f });
		coefs_44100_400.push_back({ 0.94923592f, -1.89847185f, 0.94923592f, -1.89692950f, 0.90001420f });

		// sr = 44100; sf = 800
		Common::TFiltersChainCoefficients coefs_44100_800;
		coefs_44100_800.clear();
		coefs_44100_800.push_back({ 0.95518214f, -1.91036428f, 0.95518214f, -1.90414615f, 0.91658241f });
		coefs_44100_800.push_back({ 0.90197879f, -1.80395758f, 0.90197879f, -1.79808579f, 0.80982936f });

		// sr = 44100; sf = 1600
		Common::TFiltersChainCoefficients coefs_44100_1600;
		coefs_44100_1600.clear();
		coefs_44100_1600.push_back({ 0.90849481f, -1.81698963f, 0.90849481f, -1.79317807f, 0.84080119f });
		coefs_44100_1600.push_back({ 0.81657276f, -1.63314552f, 0.81657276f, -1.61174323f, 0.65454782f });

		// sr = 44100; sf = 3200
		Common::TFiltersChainCoefficients coefs_44100_3200;
		coefs_44100_3200.clear();
		coefs_44100_3200.push_back({ 0.81209586f, -1.62419171f, 0.81209586f, -1.53677575f, 0.71160768f });
		coefs_44100_3200.push_back({ 0.67454071f, -1.34908142f, 0.67454071f, -1.27647222f, 0.42169061f });

		// sr = 44100; sf = 6400
		Common::TFiltersChainCoefficients coefs_44100_6400;
		coefs_44100_6400.clear();
		coefs_44100_6400.push_back({ 0.61889051f, -1.23778103f, 0.61889051f, -0.94012760f, 0.53543445f });
		coefs_44100_6400.push_back({ 0.46585751f, -0.93171501f, 0.46585751f, -0.70766233f, 0.15576770f });

		// sr = 44100; sf = 12800
		Common::TFiltersChainCoefficients coefs_44100_12800;
		coefs_44100_12800.clear();
		coefs_44100_12800.push_back({ 0.27354482f, -0.54708963f, 0.27354482f, 0.36513031f, 0.45930957f });
		coefs_44100_12800.push_back({ 0.19788736f, -0.39577472f, 0.19788736f, 0.26414200f, 0.05569143f });

		// sr = 48000; sf = 200
		Common::TFiltersChainCoefficients coefs_48000_200;
		coefs_48000_200.clear();
		coefs_48000_200.push_back({ 0.98991223f, -1.97982446f, 0.98991223f, -1.97948519f, 0.98016374f });
		coefs_48000_200.push_back({ 0.97621943f, -1.95243887f, 0.97621943f, -1.95210428f, 0.95277345f });

		// sr = 48000; sf = 400
		Common::TFiltersChainCoefficients coefs_48000_400;
		coefs_48000_400.clear();
		coefs_48000_400.push_back({ 0.97969337f, -1.95938673f, 0.97969337f, -1.95804318f, 0.96073029f });
		coefs_48000_400.push_back({ 0.95322435f, -1.90644870f, 0.95322435f, -1.90514144f, 0.90775596f });

		// sr = 48000; sf = 800
		Common::TFiltersChainCoefficients coefs_48000_800;
		coefs_48000_800.clear();
		coefs_48000_800.push_back({ 0.95890355f, -1.91780710f, 0.95890355f, -1.91253970f, 0.92307450f });
		coefs_48000_800.push_back({ 0.90943523f, -1.81887047f, 0.90943523f, -1.81387480f, 0.82386613f });

		// sr = 48000; sf = 1600
		Common::TFiltersChainCoefficients coefs_48000_1600;
		coefs_48000_1600.clear();
		coefs_48000_1600.push_back({ 0.91617863f, -1.83235727f, 0.91617863f, -1.81211540f, 0.85259914f });
		coefs_48000_1600.push_back({ 0.82970049f, -1.65940097f, 0.82970049f, -1.64106974f, 0.67773221f });

		// sr = 48000; sf = 3200
		Common::TFiltersChainCoefficients coefs_48000_3200;
		coefs_48000_3200.clear();
		coefs_48000_3200.push_back({ 0.82790775f, -1.65581550f, 0.82790775f, -1.58100527f, 0.73062573f });
		coefs_48000_3200.push_back({ 0.69544240f, -1.39088480f, 0.69544240f, -1.32804422f, 0.45372538f });

		// sr = 48000; sf = 6400
		Common::TFiltersChainCoefficients coefs_48000_6400;
		coefs_48000_6400.clear();
		coefs_48000_6400.push_back({ 0.64977601f, -1.29955203f, 0.64977601f, -1.04194367f, 0.55716038f });
		coefs_48000_6400.push_back({ 0.49482808f, -0.98965615f, 0.49482808f, -0.79347802f, 0.18583429f });

		// sr = 48000; sf = 12800
		Common::TFiltersChainCoefficients coefs_48000_12800;
		coefs_48000_12800.clear();
		coefs_48000_12800.push_back({ 0.32430825f, -0.64861650f, 0.32430825f, 0.15142611f, 0.44865910f });
		coefs_48000_12800.push_back({ 0.23333931f, -0.46667862f, 0.23333931f, 0.10895086f, 0.04230811f });

		// sr = 96000; sf = 200
		Common::TFiltersChainCoefficients coefs_96000_200;
		coefs_96000_200.clear();
		coefs_96000_200.push_back({ 0.99497317f, -1.98994635f, 0.99497317f, -1.98986110f, 0.99003159f });
		coefs_96000_200.push_back({ 0.98800896f, -1.97601793f, 0.98800896f, -1.97593328f, 0.97610258f });

		// sr = 96000; sf = 400
		Common::TFiltersChainCoefficients coefs_96000_400;
		coefs_96000_400.clear();
		coefs_96000_400.push_back({ 0.98991223f, -1.97982446f, 0.98991223f, -1.97948519f, 0.98016374f });
		coefs_96000_400.push_back({ 0.97621943f, -1.95243887f, 0.97621943f, -1.95210428f, 0.95277345f });

		// sr = 96000; sf = 800
		Common::TFiltersChainCoefficients coefs_96000_800;
		coefs_96000_800.clear();
		coefs_96000_800.push_back({ 0.97969337f, -1.95938673f, 0.97969337f, -1.95804318f, 0.96073029f });
		coefs_96000_800.push_back({ 0.95322435f, -1.90644870f, 0.95322435f, -1.90514144f, 0.90775596f });

		// sr = 96000; sf = 1600
		Common::TFiltersChainCoefficients coefs_96000_1600;
		coefs_96000_1600.clear();
		coefs_96000_1600.push_back({ 0.95890355f, -1.91780710f, 0.95890355f, -1.91253970f, 0.92307450f });
		coefs_96000_1600.push_back({ 0.90943523f, -1.81887047f, 0.90943523f, -1.81387480f, 0.82386613f });

		// sr = 96000; sf = 3200
		Common::TFiltersChainCoefficients coefs_96000_3200;
		coefs_96000_3200.clear();
		coefs_96000_3200.push_back({ 0.91617863f, -1.83235727f, 0.91617863f, -1.81211540f, 0.85259914f });
		coefs_96000_3200.push_back({ 0.82970049f, -1.65940097f, 0.82970049f, -1.64106974f, 0.67773221f });

		// sr = 96000; sf = 6400
		Common::TFiltersChainCoefficients coefs_96000_6400;
		coefs_96000_6400.clear();
		coefs_96000_6400.push_back({ 0.82790775f, -1.65581550f, 0.82790775f, -1.58100527f, 0.73062573f });
		coefs_96000_6400.push_back({ 0.69544240f, -1.39088480f, 0.69544240f, -1.32804422f, 0.45372538f });

		// sr = 96000; sf = 12800
		Common::TFiltersChainCoefficients coefs_96000_12800;
		coefs_96000_12800.clear();
		coefs_96000_12800.push_back({ 0.64977601f, -1.29955203f, 0.64977601f, -1.04194367f, 0.55716038f });
		coefs_96000_12800.push_back({ 0.49482808f, -0.98965615f, 0.49482808f, -0.79347802f, 0.18583429f });

		// 4. Emplace coefficient vectors in table 				
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 200), coefs_44100_200);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 400), coefs_44100_400);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 800), coefs_44100_800);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 1600), coefs_44100_1600);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 3200), coefs_44100_3200);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 6400), coefs_44100_6400);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(44100, 12800), coefs_44100_12800);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 200), coefs_48000_200);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 400), coefs_48000_400);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 800), coefs_48000_800);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 1600), coefs_48000_1600);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 3200), coefs_48000_3200);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 6400), coefs_48000_6400);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(48000, 12800), coefs_48000_12800);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 200), coefs_96000_200);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 400), coefs_96000_400);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 800), coefs_96000_800);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 1600), coefs_96000_1600);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 3200), coefs_96000_3200);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 6400), coefs_96000_6400);
		highPassFilterOrder4Table.emplace(THighOrderFilterParameters(96000, 12800), coefs_96000_12800);
	}

	/////////////////////////////////////////////////////
	// Fill hardcoded coefficients table for 6th order LPF		
	static void FillLowPassFilterOrder6CoefficientsTable()
	{		
		// TO DO: check return values of calls to "emplace"

		// 1. Check if table was already filled
		if (isLowPassFilterOrder6TableFilled)
			return;
		else
			isLowPassFilterOrder6TableFilled = true;

		// 2. Clear table
		lowPassFilterOrder6Table.clear();

		// 3. Create coefficients vectors

		// sr = 44100; sf = 1200
		Common::TFiltersChainCoefficients coefs_44100_1200;
		coefs_44100_1200.clear();
		coefs_44100_1200.push_back({ 0.006982f, 0.013964f, 0.006982f, -1.887714f, 0.915644f });	// LPF coefficients for 1st biquad. 44100Hz sample rate and 1200Hz split frequency
		coefs_44100_1200.push_back({ 0.006507f, 0.013014f, 0.006507f, -1.759197f, 0.785225f });	// LPF coefficients for 2nd biquad. 44100Hz sample rate and 1200Hz split frequency
		coefs_44100_1200.push_back({ 0.006261f, 0.012522f, 0.006261f, -1.692664f, 0.717708f });	// LPF coefficients for 3rd biquad. 44100Hz sample rate and 1200Hz split frequency

		// sr = 44100; sf = 200
		Common::TFiltersChainCoefficients coefs_44100_200;
		coefs_44100_200.clear();		
		coefs_44100_200.push_back({ 0.00020149f, 0.00040299f, 0.00020149f, -1.98455379f, 0.98535977f });
		coefs_44100_200.push_back({ 0.00019897f, 0.00039794f, 0.00019897f, -1.95970703f, 0.96050292f });
		coefs_44100_200.push_back({ 0.00019754f, 0.00039509f, 0.00019754f, -1.94564302f, 0.94643319f });

		// sr = 44100; sf = 400
		Common::TFiltersChainCoefficients coefs_44100_400;
		coefs_44100_400.clear();
		coefs_44100_400.push_back({ 0.00079996f, 0.00159992f, 0.00079996f, -1.96774410f, 0.97094394f });
		coefs_44100_400.push_back({ 0.00078033f, 0.00156065f, 0.00078033f, -1.91944457f, 0.92256588f });
		coefs_44100_400.push_back({ 0.00076942f, 0.00153884f, 0.00076942f, -1.89262340f, 0.89570108f });

		// sr = 44100; sf = 800
		Common::TFiltersChainCoefficients coefs_44100_800;
		coefs_44100_800.clear();
		coefs_44100_800.push_back({ 0.00315161f, 0.00630322f, 0.00315161f, -1.93020394f, 0.94281039f });
		coefs_44100_800.push_back({ 0.00300289f, 0.00600577f, 0.00300289f, -1.83911680f, 0.85112834f });
		coefs_44100_800.push_back({ 0.00292324f, 0.00584648f, 0.00292324f, -1.79033831f, 0.80203127f });

		// sr = 44100; sf = 1600
		Common::TFiltersChainCoefficients coefs_44100_1600;
		coefs_44100_1600.clear();
		coefs_44100_1600.push_back({ 0.01222064f, 0.02444127f, 0.01222064f, -1.84059964f, 0.88948218f });
		coefs_44100_1600.push_back({ 0.01115315f, 0.02230631f, 0.01115315f, -1.67982181f, 0.72443443f });
		coefs_44100_1600.push_back({ 0.01061768f, 0.02123537f, 0.01061768f, -1.59917235f, 0.64164308f });

		// sr = 44100; sf = 3200
		Common::TFiltersChainCoefficients coefs_44100_3200;
		coefs_44100_3200.clear();
		coefs_44100_3200.push_back({ 0.04584781f, 0.09169563f, 0.04584781f, -1.61201229f, 0.79540354f });
		coefs_44100_3200.push_back({ 0.03894697f, 0.07789393f, 0.03894697f, -1.36937804f, 0.52516591f });
		coefs_44100_3200.push_back({ 0.03583305f, 0.07166610f, 0.03583305f, -1.25989254f, 0.40322474f });

		// sr = 44100; sf = 6400
		Common::TFiltersChainCoefficients coefs_44100_6400;
		coefs_44100_6400.clear();
		coefs_44100_6400.push_back({ 0.16092570f, 0.32185141f, 0.16092570f, -1.01655607f, 0.66025888f });
		coefs_44100_6400.push_back({ 0.12434142f, 0.24868284f, 0.12434142f, -0.78545578f, 0.28282146f });
		coefs_44100_6400.push_back({ 0.10991480f, 0.21982959f, 0.10991480f, -0.69432385f, 0.13398303f });

		// sr = 48000; sf = 200
		Common::TFiltersChainCoefficients coefs_48000_200;
		coefs_48000_200.clear();
		coefs_48000_200.push_back({ 0.00017018f, 0.00034037f, 0.00017018f, -1.98586026f, 0.98654100f });
		coefs_48000_200.push_back({ 0.00016822f, 0.00033645f, 0.00016822f, -1.96298009f, 0.96365298f });
		coefs_48000_200.push_back({ 0.00016711f, 0.00033422f, 0.00016711f, -1.95000870f, 0.95067715f });

		// sr = 48000; sf = 400
		Common::TFiltersChainCoefficients coefs_48000_400;
		coefs_48000_400.clear();
		coefs_48000_400.push_back({ 0.00067607f, 0.00135215f, 0.00067607f, -1.97056668f, 0.97327097f });
		coefs_48000_400.push_back({ 0.00066078f, 0.00132156f, 0.00066078f, -1.92598397f, 0.92862709f });
		coefs_48000_400.push_back({ 0.00065226f, 0.00130452f, 0.00065226f, -1.90115085f, 0.90375989f });

		// sr = 48000; sf = 800
		Common::TFiltersChainCoefficients coefs_48000_800;
		coefs_48000_800.clear();
		coefs_48000_800.push_back({ 0.00266690f, 0.00533380f, 0.00266690f, -1.93664975f, 0.94731736f });
		coefs_48000_800.push_back({ 0.00255054f, 0.00510107f, 0.00255054f, -1.85214649f, 0.86234863f });
		coefs_48000_800.push_back({ 0.00248786f, 0.00497572f, 0.00248786f, -1.80663386f, 0.81658530f });

		// sr = 48000; sf = 1600
		Common::TFiltersChainCoefficients coefs_48000_1600;
		coefs_48000_1600.clear();
		coefs_48000_1600.push_back({ 0.01036827f, 0.02073654f, 0.01036827f, -1.85639955f, 0.89787262f });
		coefs_48000_1600.push_back({ 0.00952576f, 0.01905152f, 0.00952576f, -1.70555215f, 0.74365520f });
		coefs_48000_1600.push_back({ 0.00909889f, 0.01819779f, 0.00909889f, -1.62912290f, 0.66551847f });

		// sr = 48000; sf = 3200
		Common::TFiltersChainCoefficients coefs_48000_3200;
		coefs_48000_3200.clear();
		coefs_48000_3200.push_back({ 0.03911010f, 0.07822021f, 0.03911010f, -1.65307024f, 0.80951066f });
		coefs_48000_3200.push_back({ 0.03357181f, 0.06714362f, 0.03357181f, -1.41898265f, 0.55326989f });
		coefs_48000_3200.push_back({ 0.03103451f, 0.06206902f, 0.03103451f, -1.31173848f, 0.43587652f });

		// sr = 48000; sf = 6400
		Common::TFiltersChainCoefficients coefs_48000_6400;
		coefs_48000_6400.clear();
		coefs_48000_6400.push_back({ 0.13874792f, 0.27749584f, 0.13874792f, -1.12238218f, 0.67737386f });
		coefs_48000_6400.push_back({ 0.10844744f, 0.21689488f, 0.10844744f, -0.87727063f, 0.31106039f });
		coefs_48000_6400.push_back({ 0.09630487f, 0.19260974f, 0.09630487f, -0.77904498f, 0.16426445f });

		// sr = 96000; sf = 200
		Common::TFiltersChainCoefficients coefs_96000_200;
		coefs_96000_200.clear();
		coefs_96000_200.push_back({ 0.00004269f, 0.00008538f, 0.00004269f, -1.99307644f, 0.99324720f });
		coefs_96000_200.push_back({ 0.00004244f, 0.00008489f, 0.00004244f, -1.98148851f, 0.98165828f });
		coefs_96000_200.push_back({ 0.00004230f, 0.00008460f, 0.00004230f, -1.97485937f, 0.97502857f });

		// sr = 96000; sf = 400
		Common::TFiltersChainCoefficients coefs_96000_400;
		coefs_96000_400.clear();
		coefs_96000_400.push_back({ 0.00017018f, 0.00034037f, 0.00017018f, -1.98586026f, 0.98654100f });
		coefs_96000_400.push_back({ 0.00016822f, 0.00033645f, 0.00016822f, -1.96298009f, 0.96365298f });
		coefs_96000_400.push_back({ 0.00016711f, 0.00033422f, 0.00016711f, -1.95000870f, 0.95067715f });

		// sr = 96000; sf = 800
		Common::TFiltersChainCoefficients coefs_96000_800;
		coefs_96000_800.clear();
		coefs_96000_800.push_back({ 0.00067607f, 0.00135215f, 0.00067607f, -1.97056668f, 0.97327097f });
		coefs_96000_800.push_back({ 0.00066078f, 0.00132156f, 0.00066078f, -1.92598397f, 0.92862709f });
		coefs_96000_800.push_back({ 0.00065226f, 0.00130452f, 0.00065226f, -1.90115085f, 0.90375989f });

		// sr = 96000; sf = 1600
		Common::TFiltersChainCoefficients coefs_96000_1600;
		coefs_96000_1600.clear();
		coefs_96000_1600.push_back({ 0.00266690f, 0.00533380f, 0.00266690f, -1.93664975f, 0.94731736f });
		coefs_96000_1600.push_back({ 0.00255054f, 0.00510107f, 0.00255054f, -1.85214649f, 0.86234863f });
		coefs_96000_1600.push_back({ 0.00248786f, 0.00497572f, 0.00248786f, -1.80663386f, 0.81658530f });

		// sr = 96000; sf = 3200
		Common::TFiltersChainCoefficients coefs_96000_3200;
		coefs_96000_3200.clear();
		coefs_96000_3200.push_back({ 0.01036827f, 0.02073654f, 0.01036827f, -1.85639955f, 0.89787262f });
		coefs_96000_3200.push_back({ 0.00952576f, 0.01905152f, 0.00952576f, -1.70555215f, 0.74365520f });
		coefs_96000_3200.push_back({ 0.00909889f, 0.01819779f, 0.00909889f, -1.62912290f, 0.66551847f });

		// sr = 96000; sf = 6400
		Common::TFiltersChainCoefficients coefs_96000_6400;
		coefs_96000_6400.clear();
		coefs_96000_6400.push_back({ 0.03911010f, 0.07822021f, 0.03911010f, -1.65307024f, 0.80951066f });
		coefs_96000_6400.push_back({ 0.03357181f, 0.06714362f, 0.03357181f, -1.41898265f, 0.55326989f });
		coefs_96000_6400.push_back({ 0.03103451f, 0.06206902f, 0.03103451f, -1.31173848f, 0.43587652f });


		// 4. Emplace coefficient vectors in table 		
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 1200), coefs_44100_1200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 200),  coefs_44100_200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 400),  coefs_44100_400);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 800),  coefs_44100_800);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 1600), coefs_44100_1600);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 3200), coefs_44100_3200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(44100, 6400), coefs_44100_6400);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(48000, 200),  coefs_48000_200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(48000, 400),  coefs_48000_400);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(48000, 800),  coefs_48000_800);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(48000, 1600), coefs_48000_1600);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(48000, 3200), coefs_48000_3200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(48000, 6400), coefs_48000_6400);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(96000, 200),  coefs_96000_200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(96000, 400),  coefs_96000_400);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(96000, 800),  coefs_96000_800);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(96000, 1600), coefs_96000_1600);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(96000, 3200), coefs_96000_3200);
		lowPassFilterOrder6Table.emplace(THighOrderFilterParameters(96000, 6400), coefs_96000_6400);
	}
}// end namespace HAHLSimulation
