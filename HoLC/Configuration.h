/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
    QStringList getRecentLibraries();

    void setAlwaysSaveBeforeCompiling(const bool &value);
    void setProjectPath(const QString &value);
    void setHopsanPath(const QString &value);
    void setCompilerPath(const QString &value);
    void setUseTextWrapping(const bool &value);
    void addRecentLibrary(const QString &value);

signals:
    void configChanged();

private:
    QWidget *mpParent;

    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);
    void loadRecentLibraries(QDomElement &rDomElement);

    bool mAlwaysSaveBeforeCompiling;
    QString mProjectPath;
    QString mHopsanPath;
    QString mCompilerPath;
    bool mUseTextWrapping;
    QPalette mPalette;
    QString mStyleSheet;
    QStringList mRecentLibraries;
};

#endif // CONFIGURATION_H
