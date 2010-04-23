#include "AppearanceData.h"

AppearanceData::AppearanceData()
{
    //Assume all strings default to ""
    mnPorts = 0;
    mPortAppearanceVector.clear();

}


QTextStream& operator >>(QTextStream &is, AppearanceData &rData)
{
    QString command;

    while (!is.atEnd())
    {
        //! @todo make sure we dont read any file hader if that exist # in the begining
        is >> command; //Read the command word

        if (command == "NAME")
        {
            is >> rData.mTypeName;
        }
        else if (command == "ISOICON")
        {
            is >> rData.mIconPathISO;
        }
        else if (command == "USERICON")
        {
            is >> rData.mIconPath;
            //userIconPath = libDirObject.absolutePath() + "/" + line.mid(9);
        }
        else if (command == "ICONROTATION")
        {
            is >> rData.mIconRotationBehaviour;
        }
        else if (command == "PORTS")
        {
            is >> rData.mnPorts;

            for (size_t i=0; i<rData.mnPorts; ++i)
            {
                PortAppearance portapp;

                is >> portapp.x;
                is >> portapp.y;
                is >> portapp.rot;

                rData.mPortAppearanceVector.push_back(portapp);
                //std::cout << qPrintable(componentName) << " x: " << qPrintable(portPosX) << " y: " << qPrintable(portPosY) << " rot: " << qPrintable(portRot) << std::endl;
            }
        }
    }

    return is;
}

QTextStream& operator <<(QTextStream &os, const AppearanceData &rData)
{
    //! @todo maybe write header here (probaly not a good place, better somewhere else)
    //! @todo find out how to make newline in qt instead of "\n"
    os << "NAME " << rData.mTypeName << "\n";
    os << "ISOICON " << rData.mIconPathISO << "\n";
    os << "USERICON " << rData.mIconPath << "\n";
    os << "ICONROTATION " << rData.mIconRotationBehaviour << "\n";
    os << "PORTS " << rData.mnPorts << "\n";
    for (size_t i=0; i<rData.mnPorts; ++i)
    {
        os << rData.mPortAppearanceVector[i].x << " "
           << rData.mPortAppearanceVector[i].y << " "
           << rData.mPortAppearanceVector[i].rot << "\n";
    }

    return os;
}

QString AppearanceData::getTypeName()
{
    return mTypeName;
}

QString AppearanceData::getIconPath()
{
    return mIconPath;
}

QString AppearanceData::getIconPathISO()
{
    return mIconPathISO;
}

QString AppearanceData::getIconRotationBehaviour()
{
    return mIconRotationBehaviour;
}

QPointF AppearanceData::getNameTextPos()
{
    return mNameTextPos;
}

QString AppearanceData::getBasePath()
{
    return mBasePath;
}

void AppearanceData::setBasePath(QString path)
{
    mBasePath = path;
}
