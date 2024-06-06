/** 
 * \class CascadeGraphicEq9OctaveBands
 * 
 * \brief This is a chain of Low Shelf, Peak Notch and High Shelf filters that are used to implement a 9 octave band graphic equalizer.
 * \date May 2024
 * 
 * 
*/
#ifndef _CASCADE_GRAPHIC_EQ_9_OCTAVE_BANDS_H_
#define _CASCADE_GRAPHIC_EQ_9_OCTAVE_BANDS_H_
#include <Common/FiltersChain.h>



namespace Common {
    class CascadeGraphicEq9OctaveBands : private CFiltersChain {

        constexpr static int NUM_BANDS = 9;
        constexpr static float Q = 1.414213562373095; // sqrt(2)
        constexpr static float BANDS_CENTERS[NUM_BANDS] = {62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
        // Inverse matrix in 
        // Abel, J.S.; Berners, D.P. Filter design using second-order peaking and shelving sections. 
        // In Proceedings of the International Computer Music Conference, Coral Gables, FL, USA, 1–6 November 2004.

        constexpr static float inverseBmatrix[NUM_BANDS][NUM_BANDS] = 
        { { 1.3617, -0.3280,   0.0403,  0.0043,  0.0007,  0.0001,  0.0000,  0.0000,  0.0000},
        {-0.2750,  1.1128,  -0.2298, -0.0009, -0.0014, -0.0001, -0.0000, -0.0000, -0.0000 },
	    {-0.0023, -0.2138,   1.0915, -0.2207,  0.0001, -0.0012, -0.0001, -0.0000, -0.0000 },
	    {-0.0016, -0.0007,  -0.2172,  1.0919, -0.2196,  0.0000, -0.0012, -0.0001, -0.0000 },
	    {-0.0002, -0.0012,  -0.0006, -0.2184,  1.0922, -0.2187,  0.0004, -0.0010, -0.0000 },
	    {-0.0000, -0.0001,  -0.0012, -0.0005, -0.2209,  1.0932, -0.2158,  0.0020, -0.0005 },
	    {-0.0000, -0.0000,  -0.0001, -0.0013, -0.0008, -0.2277,  1.0969, -0.2028,  0.0064 },
	    {-0.0000, -0.0000,  -0.0000, -0.0002, -0.0018, -0.0036, -0.2632,  1.0738, -0.1416 },
	    { 0.0000,  0.0000,   0.0000,  0.0001,  0.0004,  0.0024,  0.0356, -0.1909,  1.1250 } };

    public:

        /** 
         * \brief Default constructor which creates a chain of 9 octave bands graphic equalizer filters.
         * \param commandGains Vector of command gains at each band (note that these are not the peak gains of each inidividual filter)
        */
        CascadeGraphicEq9OctaveBands(const std::vector<float> & _commandGains) {
            if (_commandGains.size() != NUM_BANDS) {
                throw std::invalid_argument("CascadeGraphicEq9OctaveBands: gains vector must have 9 elements");
            }
            SetCommandGains(_commandGains);
        }

        /** 
         * \brief Process an buffer through the whole set of filters
         * \param buffer Buffer to be processed
        */
        void Process(CMonoBuffer<float> &buffer) {
            return CFiltersChain::Process(buffer);
        }

        /** 
         * \brief Set the command gains of the filter at each band
         * \param gains Vector of command gains to be obtained at each band. Note that these are not the peak gains of each inidividual filter. 
         * \return True if the gains were set correctly, false otherwise
        */
        bool SetCommandGains(const std::vector<float> &  _gains) {
            if (_gains.size() != NUM_BANDS) {
                SET_RESULT(RESULT_ERROR_INVALID_PARAM, "CascadeGraphicEq9OctaveBands: gains vector must have 9 elements");
                return false;
            }
            commandGains = _gains;
            ResetFiltersChain(CalculatePeakGains());
        }

        /** 
         * \brief Get the effective gains of the filters in the chain
        */
        std::vector<float> GetCommandGains() {
            return commandGains;
        }

#ifndef NDEBUG
        /**
         * @brief Get the filters in the chain 
         * 
         */
        shared_ptr<CBiquadFilter> GetFilter(int index) {
            return CFiltersChain::GetFilter(index);
        }
#endif
    private:

        /** 
         * \brief Vector of effective gains of the filters in the chain
        */
        std::vector<float> commandGains;



        /** 
         * \brief Calculate the command gains for the filters in the chain
         * \param gains Vector of gains to be set
        */
        std::vector<float> CalculatePeakGains() {

            // Convert effective gains to dB
            std::vector<float> commandGainsDB(NUM_BANDS, 0.0);
            for (int i = 0; i < NUM_BANDS; i++) {
                commandGainsDB[i] = 20.0 * std::log10(commandGains[i]);
            }

            std::vector<float> peakGainsDB(NUM_BANDS, 0.0);
            for (int i = 0; i < NUM_BANDS; i++) {
                    peakGainsDB[i] = 0.0;
                for (int j = 0; j < NUM_BANDS; j++) {
                    peakGainsDB[i] += inverseBmatrix[i][j] * commandGainsDB[j];
                }
            }

            // Convert command gains to linear scale
            std::vector<float> peakGains(NUM_BANDS, 0.0);
            for (int i = 0; i < NUM_BANDS; i++) {
                peakGains[i] = std::pow(10.0, peakGainsDB[i] / 20.0);
            }

            return peakGains;
        } 

        /** 
         * \brief Reset the filters chain with the new command gains
         * \param gains Vector of gains to be set
        */
       void ResetFiltersChain(const std::vector<float> & peakGains) {
            RemoveFilters();

            // Create first low shelf filter
            auto filter = AddFilter();
            constexpr float dummyQ = 1.0; // Not used inside the low shelf filter nor the high shelf filter
            filter->Setup(48000, 62.5 * Q, dummyQ, T_filterType::LOWSHELF, peakGains[0]);

            // Create 7 peak notch filters
            for (int i = 1; i < 8; i++) {
                filter = AddFilter();
                filter->Setup(48000, BANDS_CENTERS[i], Q, T_filterType::PEAKNOTCH, peakGains[i]);
            }

            // Create last high shelf filter
            filter = AddFilter();
            filter->Setup(48000, 16000 / Q, dummyQ, T_filterType::HIGHSHELF, peakGains[8]); 
            
       }

    };
}
#endif
