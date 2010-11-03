#include "GUIPortAppearance.h"
#include "common.h"

//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in som more generic external .txt file in som way
void GUIPortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mIconPath.clear();
    mIconOverlayPath.clear();

    mIconPath = QString(PORTICONPATH);
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
    else
    {
        if (nodetype == "NodeMechanic")
        {
            mIconPath.append("MechanicPort");
    //        if (CQSType == "C")
    //        {
    //            mIconPath.append("C");
    //        }
    //        else if (CQSType == "Q")
    //        {
    //            mIconPath.append("Q");
    //        }
        }
        else if (nodetype == "NodeMechanicRotational")
        {
            mIconPath.append("RotationalMechanicPort");
    //        if (CQSType == "C")
    //        {
    //            mIconPath.append("C");
    //        }
    //        else if (CQSType == "Q")
    //        {
    //            mIconPath.append("Q");
    //        }
        }
        else if (nodetype == "NodeHydraulic")
        {
            mIconPath.append("HydraulicPort");
    //        if (CQSType == "C")
    //        {
    //            mIconPath.append("C");
    //        }
    //        else if (CQSType == "Q")
    //        {
    //            mIconPath.append("Q");
    //        }
        }
        else
        {
            //SystemPort is a blank port (that is why we use it here)
            mIconPath.append("SystemPort");
        }

        //Select overlay icon depending on cqs type
        mIconOverlayPath = QString(PORTICONPATH);
        //! @todo maybe should be able to select bassed on other things than cqs type
        if (CQSType == "C")
        {
            mIconOverlayPath.append("PortOverlayC.svg");
        }
        else if (CQSType == "Q")
        {
            mIconOverlayPath.append("PortOverlayQ.svg");
        }
        else
        {
            mIconOverlayPath.clear();
        }
    }
    mIconPath.append(".svg");
}

