#ifndef ELECTRICVOLTAGESOURCEMULTIPORTC_HPP
#define ELECTRICVOLTAGESOURCEMULTIPORTC_HPP

#include <vector>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup ElectricComponents
    //!
    class ElectricVoltageSourceMultiPortC : public ComponentC
    {
    private:
        Port *mpMP;
        size_t mNumPorts;
        std::vector<double*> mvpMP_u;
        std::vector<double*> mvpMP_i;
        std::vector<double*> mvpMP_c;
        std::vector<double*> mvpMP_Zc;
        double *mpU;

    public:
        static Component *Creator()
        {
            return new ElectricVoltageSourceMultiPortC();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("P1", "NodeElectric");
            addInputVariable("U", "Voltage", "V", 12, &mpU);

            disableStartValue(mpMP, NodeElectric::Voltage);
            disableStartValue(mpMP, NodeElectric::WaveVariable);
            disableStartValue(mpMP, NodeElectric::CharImpedance);
        }


        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();
            mvpMP_u.resize(mNumPorts);
            mvpMP_i.resize(mNumPorts);
            mvpMP_c.resize(mNumPorts);
            mvpMP_Zc.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpMP_u[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::Voltage);
                mvpMP_i[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::Current);
                mvpMP_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::WaveVariable);
                mvpMP_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::CharImpedance);

                *(mvpMP_u[i]) = (*mpU);    //Override the startvalue for the pressure
                *(mvpMP_i[i]) = getDefaultStartValue(mpMP, NodeElectric::Current);
                *(mvpMP_c[i]) = (*mpU);
                *(mvpMP_Zc[i]) = 0.0;
            }
        }


        void simulateOneTimestep()
        {
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *(mvpMP_c[i]) = (*mpU);
                *(mvpMP_Zc[i]) = 0.0;
            }
        }
    };
}

#endif // ELECTRICVOLTAGESOURCEMULTIPORTC_HPP
