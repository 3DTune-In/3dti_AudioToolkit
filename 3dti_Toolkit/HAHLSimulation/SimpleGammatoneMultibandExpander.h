/**
* \class CSimpleGammatoneMultibandExpander
*
* \brief Declaration of CGammatoneMultibandExpander class interface. 
*
* \date	January 2025
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

#ifndef _CSIMPLEGAMMATONEMULTIBAND_EXPANDER_H_
#define _CSIMPLEGAMMATONEMULTIBAND_EXPANDER_H_

#include <HAHLSimulation/MultibandExpander.h>
#include <Common/GammatoneFilterBank.h>
#include <Common/CommonDefinitions.h>
#include <Common/DynamicExpanderMono.h>
#include <Common/ErrorHandler.h>
#include <vector>
#include <memory>

// Defaulta values for initialization of expanders
#define DEFAULT_RATIO 1
#define DEFAULT_THRESHOLD 0
#define DEFAULT_ATTACK 20
#define DEFAULT_RELEASE 100
#define LINEAR_GAIN_CORRECTION_GAMMATONE 4					// equivalent to 12dB gain

namespace HAHLSimulation 
{

 /** \details This class implements a multiband 
  *  equalizer where each band has an independent envelope follower and expander
  *  This is used for simulation of non-linear attenuation in hearing loss. It is meant as a replacement 
  *  of the CGammatoneMultibandExpander class, which has been deprecated.
  */    
class CSimpleGammatoneMultibandExpander 
{
public: 

    /** \brief Setup the multiband expander 
     * \details Specifies the bands. 
     * \param [in] samplingRate sampling rate in samples per second
     * \param [in] vector of band centers. 
    */
    void Setup(int samplingRate, const std::vector<float> & _bandCenters_Hz){
        SetupImpl(samplingRate, _bandCenters_Hz.begin(), _bandCenters_Hz.end());
    }

    /** \brief Setup the multiband expander with initializer list for bands 
     * \details Specifies the bands as initializer list. For example, Setup(44100, {250, 500, 1000, 2000, 3000, 4000, 6000, 8000});
     * \param [in] samplingRate sampling rate in samples per second
     * \param [in] initializer list of band centers.
    */
    void Setup(int samplingRate, std::initializer_list<float> _bandCenters_Hz){
        SetupImpl(samplingRate, _bandCenters_Hz.begin(), _bandCenters_Hz.end());
    }

    /** \brief Setup using octave bands and specifying grouping
     * \details Specifies the bands as octave bands. For example, Setup(44100, 125, 8); 
     * \param [in] samplingRate sampling rate in samples per second
     * \param [in] initial frequency in Hz
     * \param [in] number of octave bands
     */
    void Setup(int samplingRate, float initialFrequency_Hz, int octaveBands, bool filterGrouping = false){
        std::vector<float> _bandCenters_Hz;
        for (int i = 0; i < octaveBands; i++){
            _bandCenters_Hz.push_back(initialFrequency_Hz * std::pow(2, i));
        }
        SetupImpl(samplingRate, _bandCenters_Hz.begin(), _bandCenters_Hz.end());
    }

    /** \brief Process an input buffer
     * \details The input buffer is processed by the multiband expander. The result is returned in the output buffer
     * \param [in] inputBuffer input buffer
     * \param [out] outputBuffer output buffer
     */
    void Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer){
        ASSERT(setupDone, RESULT_ERROR_NOTALLOWED, "Setup not done", "");
        ASSERT(inputBuffer.size() == outputBuffer.size(), RESULT_ERROR_BADSIZE, "Input and output buffer sizes do not match", "");

    }

    /** \brief Get the frequency in Hertzs of the band whose index is passed
     * \param [in] bandIndex index of the band whose frequency is requiered
     * \retval frequency centre frequency in Hzs of the band
     */
    float GetBandFrequency(int bandIndex){
        ASSERT(setupDone, RESULT_ERROR_NOTALLOWED, "Setup not done", "");
        ASSERT(bandIndex >= 0 && bandIndex < bands.size(), RESULT_ERROR_OUTOFRANGE, "Invalid band index", "");
        return bands[bandIndex].centerFrequency_Hz;
    }

    /** \brief Get the number of bands */
    int GetNumBands(){
        ASSERT(setupDone, RESULT_ERROR_NOTALLOWED, "Setup not done", "");
        return bands.size();
    }

private: 

    /** \brief Private function doing the real work of Setup */
    template <typename Iterator>
    void SetupImpl(int samplingRate, Iterator begin, Iterator end)q{

        // Check input parameters
        ASSERT((samplingRate > 0), RESULT_ERROR_INVALID_PARAM, "Invalid sampling rate", "");
        ASSERT((std::distance(begin, end) > 0), RESULT_ERROR_INVALID_PARAM, "No bands specified", "");

        // Check that bands are in ascending order 
        for (auto it = begin; it != end; ++it){
            if (it != begin){
                ASSERT((*it > *(it - 1)), RESULT_ERROR_INVALID_PARAM, "Bands should be in ascending order", "");
            }
        }

        // Reset the setup
        ResetSetup();

        // Create the filter bank and per-filter expanders 
        Common::CGammatoneFilterBank gammatoneFilterBank(samplingRate);
        gammatoneFilterBank.InitWithFreqRangeOverlap(20, 20000, 0.0, Common::CGammatoneFilterBank::EAR_MODEL_DEFAULT);

        // Store filter bank and expanders. With respect to CGammatoneMultibandExpander, we are not storing the expanders in a vector, but in the bands vector  
        // Also we are notestoring the lower and upper indices for the octave bands, as we are not using them
        filters.resize(gammatoneFilterBank.GetNumFilters());
        for (int filterIndex = 0; filterIndex < gammatoneFilterBank.GetNumFilters(); filterIndex++){
            filters[filterIndex].centerFrequency_Hz = gammatoneFilterBank.GetFilter(filterIndex)->GetCenterFrequency();
            filters[filterIndex].filter = gammatoneFilterBank.GetFilter(filterIndex);
            filters[filterIndex].expander = std::make_unique<Common::CDynamicExpanderMono>();
            filters[filterIndex].expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
        }


        // Create the bands and per-band expanders. This would be equivalent to set groups
        for (auto it = begin; it != end; ++it){
            BandInfo band;
            band.centerFrequency_Hz = *it;
            band.expander = std::make_unique<Common::CDynamicExpanderMono>();
            band.expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
            band.gain_dB = 0.0f;
            bands.push_back(std::move(band));
        }   

        // Compute the lower and upper limits for the bands (geometric means between adjacent bands, except for first and last bands)
        for (int i = 0; i < bands.size(); i++){
            if (i == 0){
                bands[i].lowerLimit_Hz = 0.0f;
            } else {
                bands[i].lowerLimit_Hz = std::sqrt(bands[i].centerFrequency_Hz * bands[i - 1].centerFrequency_Hz);
            }
            if (i == bands.size() - 1){
                bands[i].upperLimit_Hz = 30000.0f;
            } else {
                bands[i].upperLimit_Hz = std::sqrt(bands[i].centerFrequency_Hz * bands[i + 1].centerFrequency_Hz);
            }
        }

        // Compute the lower and upper filter indices for the bands, i.e what is the first and last filter that belongs to the band
        for (int i = 0; i < bands.size(); i++){
            for (int j = 0; j < filters.size(); j++){
                if (filters[j].centerFrequency_Hz >= bands[i].lowerLimit_Hz){
                    bands[i].lowerIndex = j;
                    break;
                }
            }
            for (int j = filters.size() - 1; j >= 0; j--){
                if (filters[j].centerFrequency_Hz <= bands[i].upperLimit_Hz){
                    bands[i].upperIndex = j;
                    break;
                }
            }
        }

        // Setup done
        setupDone = true;

    }
public: 

    // Reset the setup
    void ResetSetup(){
        setupDone = false;
        bands.clear();
        filters.clear();
    }

    // Filter information 
    struct FilterInfo {

        // Center frequency of the filter
        float centerFrequency_Hz{0.0f};

        // Filter
        std::shared_ptr<Common::CGammatoneFilter> filter;

        // Expander for the filter
        std::unique_ptr<Common::CDynamicExpanderMono> expander;

        // Gain in dB for the filter
        float gain_dB{0.0f};

    };
    std::vector<FilterInfo> filters;
    

    // Band information
    struct BandInfo {
        // Center frequency of the band
        float centerFrequency_Hz{0.0f};

        // Expander for the band
        std::unique_ptr<Common::CDynamicExpanderMono> expander;

        // Gain in dB for the band
        float gain_dB{0.0f};

        // Lower and upper limits for the band
        float lowerLimit_Hz{0.0f};
        float upperLimit_Hz{0.0f};

        // Lower and upper indices for the band
        int lowerIndex{-1};
        int upperIndex{-1};

    };
    std::vector<BandInfo> bands;

    // Setup done flag
    bool setupDone{false};
    


};

} // namespace HAHLSimulation

#endif

 


