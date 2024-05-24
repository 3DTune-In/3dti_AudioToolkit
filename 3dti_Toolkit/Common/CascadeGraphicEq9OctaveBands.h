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

    public:
        CascadeGraphicEq9OctaveBands();


        void Process(CMonoBuffer<float> &buffer) {
            return CFiltersChain::Process(buffer);
        }

        void SetEffectiveGains(std::vector<float> gains) {
            std:vector<float> commandGains = CalculateCommandGains(gains);

        }

    };
}
#endif