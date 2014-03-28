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

    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();

    QString getProjectPath();

    QPalette getPalette();
    QString getStyleSheet();

    void setProjectPath(QString dir);

private:
    QWidget *mpParent;

    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);

    QString mProjectPath;
    QPalette mPalette;
    QString mStyleSheet;
};

#endif // CONFIGURATION_H
