//!
//! @file   GUIModelObjectAppearance.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//!
//$Id$

#ifndef GUIMODELOBJECTAPPEARANCE_H
#define GUIMODELOBJECTAPPEARANCE_H

#include <QString>
#include <QPointF>
#include "../common.h"

#include "../Utilities/XMLUtilities.h"
#include "../GUIPortAppearance.h"


class GUIModelObjectAppearance
{
public:
    GUIModelObjectAppearance();
    void setTypeName(QString name);
    void setName(QString name);
    void setHelpText(QString text);
    void setBaseIconPath(QString path);
    void setIconPathUser(QString path);
    void setIconPathISO(QString path);

    QString getTypeName();
    QString getName();
    QString getNonEmptyName();
    QString getHelpPicture();
    QString getHelpText();
    QString getFullIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getIconPathUser();
    QString getIconPathISO();
    QString getIconRotationBehaviour();
    QPointF getNameTextPos();
    PortAppearanceMapT &getPortAppearanceMap();

    bool haveIsoIcon();
    bool haveUserIcon();

    QString getBaseIconPath();

    void readFromDomElement(QDomElement domElement);
    void saveToDomElement(QDomElement &rDomElement);
    void saveToXML(QString filename);

private:
    QString mTypeName;
    QString mName;
    QString mHelpPicture;
    QString mHelpText;
    QString mIconPathUser;
    QString mIconPathISO;
    //! @todo In the future we should store file info separately for iso and user icons, and not use one common base path
    //QFileInfo mUserIconInfo;
    //QFileInfo mISOIconInfo;
    QString mIconRotationBehaviour;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings, mayb should not be stored in here
    QString mBasePath;

};

#endif // APPEARANCEDATA_H
