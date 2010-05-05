//!
//! @file   AppearanceData.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//!
//$Id$

#include "AppearanceData.h"
#include "qdebug.h"

AppearanceData::AppearanceData()
{
    //Assume all strings default to ""
    mnPorts = 0;
//    mPortAppearanceVector.clear(); //! @todo should be deleted everywhere
    mPortAppearanceMap.clear();
}


QTextStream& operator >>(QTextStream &is, AppearanceData &rData)
{
    //! @todo handle returned error indication
    bool sucess = rData.setAppearanceData(is);
    return is;
}

QTextStream& operator <<(QTextStream &os, const AppearanceData &rData)
{
    //! @todo maybe write header here (probaly not a good place, better somewhere else)
    //! @todo find out how to make newline in qt instead of "\n"
    //! @todo do not write emtpy info
    os << "NAME " << rData.mTypeName << "\n";
    //os << "BASEPATH " << rData.getBasePath() << "\n"; //Base path is computer dependant
    os << "ISOICON " << rData.mIconPathISO << "\n";
    os << "USERICON " << rData.mIconPathUser << "\n";
    os << "ICONROTATION " << rData.mIconRotationBehaviour << "\n";
//    os << "PORTS " << rData.mnPorts << "\n";
//    for (size_t i=0; i<rData.mnPorts; ++i)
//    {
//        os << rData.mPortAppearanceVector[i].x << " "
//           << rData.mPortAppearanceVector[i].y << " "
//           << rData.mPortAppearanceVector[i].rot << "\n";
//    }
    QMap<QString, PortAppearance> map;
    map = rData.mPortAppearanceMap;
    QMap<QString, PortAppearance>::iterator i;
    for (i = map.begin(); i != map.end(); ++i)
    {
        os << "PORT " << " \""
           << i.key() << "\" "
           << i.value().x << " "
           << i.value().y << " "
           << i.value().rot << "\n";
    }
    return os;
}

QString AppearanceData::getTypeName()
{
    return mTypeName;
}

QString AppearanceData::getFullIconPath(bool useIso)
{
    if ( !mIconPathUser.isEmpty() && !useIso )
    {
        //Use user icon
        return mBasePath + mIconPathUser;
    }
    else if ( !mIconPathISO.isEmpty() && useIso )
    {
        //use iso icon
        return mBasePath + mIconPathISO;
    }
    else if ( mIconPathUser.isEmpty() && !mIconPathISO.isEmpty() )
    {
        //Want user icon but not available, use iso icon
        return mBasePath + mIconPathISO;
    }
    else if ( !mIconPathUser.isEmpty() && mIconPathISO.isEmpty() )
    {
        //Want ISO icon but not available, Use user icon
        return mBasePath + mIconPathUser;
    }
    else
    {
        //No icon available use som noname icon
        return "som noname file"; //!< @todo Fix this, Empty icon
    }
}

QString AppearanceData::getIconPathUser()
{
    return mIconPathUser;
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

//QVector<PortAppearance> &AppearanceData::getPortAppearanceVector()
//{
//    return mPortAppearanceVector;
//}

QMap<QString, PortAppearance> &AppearanceData::getPortAppearanceMap()
{
    return mPortAppearanceMap;
}


QString AppearanceData::getBasePath()
{
    return mBasePath;
}

bool AppearanceData::setAppearanceData(QTextStream &is)
{
    QString command;
    bool sucess = true;

    while (!is.atEnd())
    {
        //! @todo make sure we dont read any file hader if that exist # in the begining
        //! @todo need som error handling here if file stream has incorect data
        is >> command; //Read the command word

        if (command == "NAME")
        {
            mTypeName = is.readLine().trimmed();
        }
        else if (command == "ISOICON")
        {
            mIconPathISO = is.readLine().trimmed();
        }
        else if (command == "USERICON")
        {
            mIconPathUser = is.readLine().trimmed();
        }
        else if (command == "ICONROTATION")
        {
            mIconRotationBehaviour = is.readLine().trimmed();
        }
        else if (command == "PORT")
            //New style:
        {
            QString tmp;

            is >> tmp;
            tmp=tmp.trimmed();
            qDebug() << tmp;

            //! @todo Fix the \" thing
            QString portName;
            portName = tmp.mid(1);
            portName.chop(1);

            qDebug() << "New style! " << portName;

            PortAppearance portapp;

            is >> portapp.x;
            is >> portapp.y;
            is >> portapp.rot;

            mPortAppearanceMap.insert(portName, portapp);
            //                mPortAppearanceVector.push_back(portapp);
            qDebug() << "Map size: " << mPortAppearanceMap.size();
            //                qDebug() << "Vector size: " << mPortAppearanceVector.size();

        }
        else if (command == "PORTS") //Old style:
            //! @todo delete when component-txt:s are re-done
        {
            QString tmp;

            is >> tmp;
            tmp=tmp.trimmed();
            qDebug() << tmp;

            mnPorts = tmp.toInt();

            for (size_t i=0; i<mnPorts; ++i)
            {
                PortAppearance portapp;

                is >> portapp.x;
                is >> portapp.y;
                is >> portapp.rot;
                //is.readLine(); //Clear the line ending

                mPortAppearanceMap.insert(tmp, portapp);
                //                    mPortAppearanceVector.push_back(portapp);
                //std::cout << qPrintable(componentName) << " x: " << qPrintable(portPosX) << " y: " << qPrintable(portPosY) << " rot: " << qPrintable(portRot) << std::endl;
            }
        }
        else if (command == "BASEPATH")
        {
            mBasePath = is.readLine().trimmed();
        }
        else
        {
            //Ignore empty lines
            if (!command.isEmpty())
            {
                qDebug() << "appearanceData: Incorrect command: " + command;
                sucess = false;
            }
        }
    }
    return sucess;
}

void AppearanceData::setTypeName(QString name)
{
    mTypeName = name;
}

void AppearanceData::setBasePath(QString path)
{
    mBasePath = path;
}

void AppearanceData::setIconPathUser(QString path)
{
    mIconPathUser = path;
}

void AppearanceData::setIconPathISO(QString path)
{
    mIconPathISO = path;
}

bool AppearanceData::haveIsoIcon()
{
    return !mIconPathISO.isEmpty();
}

bool AppearanceData::haveUserIcon()
{
    return !mIconPathUser.isEmpty();
}
