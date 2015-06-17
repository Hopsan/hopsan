/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>
#include <QPen>
#include <QPalette>
#include <QFont>
#include <QDomElement>

class Configuration : public QObject
{
    Q_OBJECT

public:
    Configuration(QWidget *pParent);

    void loadFromXml();
    void loadDefaultsFromXml();

public slots:
    void saveToXml();
    bool getAlwaysSaveBeforeCompiling() const;
    QString getProjectPath() const;
    QString getHopsanPath() const;
    QString getIncludePath() const;
    QString getHopsanCoreLibPath() const;
    QString getCompilerPath() const;
    bool getUseTextWrapping() const;
    QPalette getPalette();
    QString getStyleSheet();

    void setAlwaysSaveBeforeCompiling(const bool &value);
    void setProjectPath(const QString &value);
    void setHopsanPath(const QString &value);
    void setCompilerPath(const QString &value);
    void setUseTextWrapping(const bool &value);

signals:
    void configChanged();

private:
    QWidget *mpParent;

    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);

    bool mAlwaysSaveBeforeCompiling;
    QString mProjectPath;
    QString mHopsanPath;
    QString mCompilerPath;
    bool mUseTextWrapping;
    QPalette mPalette;
    QString mStyleSheet;
};

#endif // CONFIGURATION_H
