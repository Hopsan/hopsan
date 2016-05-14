#ifndef MECHANICALSEPARATOR_HPP
#define MECHANICALSEPARATOR_HPP

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

class MechanicalSeparator : public ComponentQ
{

private:
    // Port pointers
    Port *mpP1, *mpP2;
    // Node data pointers
    double *mpOutContact;
    MechanicNodeDataPointerStructT mP1, mP2;

    // Member variables
    bool mIsContact;
    int mMethod;
    int mCCounter, mNCCounter, mCCounterThresh, mNCCounterThresh;
    double mDisconnectThresh;

    // Other member variables
    Integrator mFreeIntegrator1;
    Integrator mFreeIntegrator2;

public:
    static Component *Creator()
    {
        return new MechanicalSeparator();
    }

    void configure()
    {
        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeMechanic");
        mpP2 = addPowerPort("P2", "NodeMechanic");

        addOutputVariable("contact", "Contact indication", "", &mpOutContact);
        std::vector<HString> conditions;
        conditions.push_back("Standard");
        conditions.push_back("RecalcNC");
        conditions.push_back("StepsizeRelax");
        addConditionalConstant("method", "", conditions, 2, mMethod);
        addConstant("dc_thresh", "Disconnect force threshold", "", 0, mDisconnectThresh);
        addConstant("cctr_thresh", "Nr steps before contact can be broken", "", 0, mCCounterThresh);
        addConstant("nctr_thresh", "Nr steps before contact can be reestablished", "", 0, mNCCounterThresh);
    }


    void initialize()
    {
        //Assign node data pointers
        getMechanicPortNodeDataPointers(mpP1, mP1);
        getMechanicPortNodeDataPointers(mpP2, mP2);

        //Initialization
        mFreeIntegrator1.initialize(mTimestep,mP1.v(),mP1.x());
        mFreeIntegrator2.initialize(mTimestep,mP2.v(),mP2.x());
        mIsContact = ((mP2.x()+mP1.x())<=0.0);
        mCCounter = 0;
        mNCCounter = 0;

        addInfoMessage("Method is: "+to_hstring(mMethod));

        simulateOneTimestep();
    }

    inline void calcNonContactVF(double &rV1, double &rV2, double &rF1, double &rF2)
    {
        rV1 = -mP1.c()/std::max(mP1.Zc(), 1e-12);
        rV2 = -mP2.c()/std::max(mP2.Zc(), 1e-12);
        rF1 = 0;
        rF2 = 0;
    }

    inline void calcContactVF(double &rV1, double &rV2, double &rF1, double &rF2)
    {
        rV2 = (mP1.c()-mP2.c())/std::max(mP1.Zc()+mP2.Zc(), 1e-12);
        rV1 = -rV2;
        rF1 = mP1.c() + mP1.Zc()*rV1;
        rF2 = mP2.c() + mP2.Zc()*rV2;

//        if (rF1<0 || rF2<0)
//        {
//            HString msg = "c1: "+to_hstring(mP1.c())+ " c2: "+to_hstring(mP2.c())+" Zc1: "+to_hstring(mP1.Zc())+" Zc2: "+to_hstring(mP2.Zc())+" v2: "+to_hstring(rV2);
//            addWarningMessage(msg);
//        }
    }


    void simulateOneTimestep()
    {
        double x1,x2,v1,v2,f1,f2;

        if (mMethod == 0)
        {
            if (mIsContact)
            {
                // OK we have contact
                v2 = (mP1.c()-mP2.c())/std::max(mP1.Zc()+mP2.Zc(), 1e-12);
                v1 = -v2;
                f1 = mP1.c() + mP1.Zc()*v1;
                f2 = mP2.c() + mP2.Zc()*v2;

                x1 = mFreeIntegrator1.update(v1);
                x2 = mFreeIntegrator2.update(v2);

                // Positive forces means that there are forces acting into the Q-ports
                // If any of the forces are negative it means it is pulling away
                // (not to be confused with the positive direction for velocity)
                // If one of the forces becomes negative, that means that the contact is broken.
                mIsContact = !(f1<mDisconnectThresh || f2<mDisconnectThresh);
                f1 = lowerLimit(f1,0);
                f2 = lowerLimit(f2,0);

                writeOutputVariable(mpOutContact, 1);
            }
            else
            {
                // First calculate the non-contact behavior
                v1 = -mP1.c()/std::max(mP1.Zc(), 1e-12);
                v2 = -mP2.c()/std::max(mP2.Zc(), 1e-12);
                f1 = 0;
                f2 = 0;

                x1 = mFreeIntegrator1.update(v1);
                x2 = mFreeIntegrator2.update(v2);

                // Contact if positions becomes the same or x2 >= x1
                // Note! Since we have discrete time steps, the actual contact may have been missed
                // x_diff = x2-x1 but since positive directions are mirrored => x_diff = x2+x1
                mIsContact = x2+x1<=0;

                writeOutputVariable(mpOutContact, 0);
            }
        }
        else if (mMethod == 1)
        {
            if (mIsContact)
            {
                // OK we have contact
                calcContactVF(v1,v2,f1,f2);

                // Positive forces means that there are forces acting into the Q-ports
                // If any of the forces are negative it means it is pulling away
                // (not to be confused with the positive direction for velocity)
                // If one of the forces becomes negative, that means that the contact is broken.
                mIsContact = !(f1<mDisconnectThresh || f2<mDisconnectThresh);
                // If not contact then use non-contact calculation instead
                if (!mIsContact)
                {
//                    addWarningMessage("Recalc: "+to_hstring(f1)+" "+to_hstring(f2));
                    calcNonContactVF(v1,v2,f1,f2);
                }
                f1 = lowerLimit(f1,0);
                f2 = lowerLimit(f2,0);

                x1 = mFreeIntegrator1.update(v1);
                x2 = mFreeIntegrator2.update(v2);

                writeOutputVariable(mpOutContact, boolToDouble(mIsContact));
            }
            else
            {
                // Calculate the non-contact behavior
                calcNonContactVF(v1,v2,f1,f2);

                x1 = mFreeIntegrator1.update(v1);
                x2 = mFreeIntegrator2.update(v2);

                // Contact if positions becomes the same or x2 >= x1
                // Note! Since we have discrete time steps, the actual contact may have been missed
                // x_diff = x2-x1 but since positive directions are mirrored => x_diff = x2+x1
                mIsContact = x2+x1<=0;

                writeOutputVariable(mpOutContact, boolToDouble(mIsContact));
            }
        }
        else if (mMethod == 2)
        {
            if (mIsContact)
            {
                // OK we have contact
                calcContactVF(v1,v2,f1,f2);

                // Positive forces means that there are forces acting into the Q-ports
                // If any of the forces are negative it means it is pulling away
                // (not to be confused with the positive direction for velocity)
                // If one of the forces becomes negative, that means that the contact is broken.
                if (mCCounter < mCCounterThresh)
                {
                    mIsContact = true;
                }
                else
                {
                    mIsContact = !(f1<mDisconnectThresh || f2<mDisconnectThresh);
                }

                f1 = lowerLimit(f1,0);
                f2 = lowerLimit(f2,0);

                x1 = mFreeIntegrator1.update(v1);
                x2 = mFreeIntegrator2.update(v2);

                writeOutputVariable(mpOutContact, boolToDouble(mIsContact));

                if (!mIsContact)
                {
                    mCCounter = 0;
                    mNCCounter = 0;
                }
                else
                {
                    mCCounter++;
                }
            }
            else
            {
                // Calculate the non-contact behavior
                calcNonContactVF(v1,v2,f1,f2);

                x1 = mFreeIntegrator1.update(v1);
                x2 = mFreeIntegrator2.update(v2);

                if (mNCCounter < mNCCounterThresh)
                {
                    mIsContact = false;
                }
                else
                {
                    // Contact if positions becomes the same or x2 >= x1
                    // Note! Since we have discrete time steps, the actual contact may have been missed
                    // x_diff = x2-x1 but since positive directions are mirrored => x_diff = x2+x1
                    mIsContact = x2+x1<=0;
                }

                writeOutputVariable(mpOutContact, boolToDouble(mIsContact));

                if (mIsContact)
                {
                    mCCounter = 0;
                    mNCCounter = 0;
                }
                else
                {
                    mNCCounter++;
                }
            }
        }

        // Write to nodes
        mP1.rf() = f1;
        mP1.rv() = v1;
        mP1.rx() = x1;

        mP2.rf() = f2;
        mP2.rv() = v2;
        mP2.rx() = x2;
    }
};
}

#endif // MECHANICALSEPARATOR_HPP

