#include "GUIPortAppearance.h"

//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in som more generic external .txt file in som way
void GUIPortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mIconPath.clear();
    mIconPath.append(QString(PORTICONPATH));
    if (nodetype == "NodeSignal")
    {
        mIconPath.append("SignalPort");
        if ( porttype == "READPORT")
        {
            mIconPath.append("_read");
        }
        else
        {
            mIconPath.append("_write");
        }
    }
    else if (nodetype == "NodeMechanic")
    {
        mIconPath.append("MechanicPort");
        if (CQSType == "C")
        {
            mIconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            mIconPath.append("Q");
        }
    }
    else if (nodetype == "NodeMechanicRotational")
    {
        mIconPath.append("RotationalMechanicPort");
        if (CQSType == "C")
        {
            mIconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            mIconPath.append("Q");
        }
    }
    else if (nodetype == "NodeHydraulic")
    {
        mIconPath.append("HydraulicPort");
        if (CQSType == "C")
        {
            mIconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            mIconPath.append("Q");
        }
    }
    else
    {
        //SystemPort is a blank port (that is why we use it here)
        mIconPath.append("SystemPort");
    }
    mIconPath.append(".svg");
}

