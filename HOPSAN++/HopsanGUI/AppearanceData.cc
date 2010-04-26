#include "AppearanceData.h"
#include "qdebug.h"

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
        //! @todo need som error handling here if file stream has incorect data
        is >> command; //Read the command word

        if (command == "NAME")
        {
            rData.mTypeName = is.readLine().trimmed();
        }
        else if (command == "ISOICON")
        {
            rData.mIconPathISO = is.readLine().trimmed();
        }
        else if (command == "USERICON")
        {
            rData.mIconPath = is.readLine().trimmed();
        }
        else if (command == "ICONROTATION")
        {
            rData.mIconRotationBehaviour = is.readLine().trimmed();
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
        else if (command == "BASEPATH")
        {
            rData.mBasePath = is.readLine().trimmed();
        }
        else
        {
            qDebug() << "appearanceData: Incorrect command: " + command;
        }
    }

    return is;
}

QTextStream& operator <<(QTextStream &os, const AppearanceData &rData)
{
    //! @todo maybe write header here (probaly not a good place, better somewhere else)
    //! @todo find out how to make newline in qt instead of "\n"
    os << "NAME " << rData.mTypeName << "\n";
    //os << "BASEPATH " << rData.getBasePath() << "\n"; //Base path is computer dependant
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

size_t AppearanceData::getNumberOfPorts()
{
    return mnPorts;
}

QVector<PortAppearance> &AppearanceData::getPortAppearanceVector()
{
    return mPortAppearanceVector;
}

QString AppearanceData::getBasePath()
{
    return mBasePath;
}

void AppearanceData::setTypeName(QString name)
{
    mTypeName = name;
}

void AppearanceData::setBasePath(QString path)
{
    mBasePath = path;
}

void AppearanceData::setIconPath(QString path)
{
    mIconPath = path;
}

void AppearanceData::setIconPathISO(QString path)
{
    mIconPathISO = path;
}
