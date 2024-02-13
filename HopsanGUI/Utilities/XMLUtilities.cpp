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

//!
//! @file   XMLUtilities.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-xx
//!
//! @brief Contains XML DOM help functions that are more or less Hopsan specific
//!
//$Id$

#include "version_gui.h"
#include "XMLUtilities.h"
#include "GUIUtilities.h"
#include <QMessageBox>
#include <QLocale>

#include "GUIObjects/GUIModelObjectAppearance.h"
#include "MessageHandler.h"
#include "global.h"

//! @brief Help function to get "correct" string representation of bool
QString bool2str(const bool in)
{
    if (in)
    {
        return hmf::truetag;
    }
    else
    {
        return hmf::falsetag;
    }
}

//! @brief Function for loading an XML DOM Document from file
//! @param[in] rFile The file to load from
//! @param[in] rDomDocument The DOM Document to load into
//! @param[in] rootTagName The expected root tag name to extract from the Dom Document
//! @returns The extracted DOM root element from the loaded DOM document
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName)
{
    QString errorStr;
    int errorLine, errorColumn;
    if (!rDomDocument.setContent(&rFile, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(0, "Hopsan GUI",
                                 QString(rFile.fileName() + ": Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement xmlRoot = rDomDocument.documentElement();
        if (xmlRoot.tagName() != rootTagName)
        {
            QMessageBox::information(0, "Hopsan GUI",
                                     QString("The file has the wrong Root Tag Name: ")
                                     + xmlRoot.tagName() + "!=" + rootTagName);
        }
        else
        {
            return xmlRoot;
        }
    }
    return QDomElement(); //NULL
}

//! @brief Appends xml processing instructions before the root tag in a DOM Document, run this ONCE before writing to file
//! @param[in] rDomDocument The DOM Document to append ProcessingInstructions to
void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument)
{
    QDomNode xmlProcessingInstruction = rDomDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    rDomDocument.insertBefore(xmlProcessingInstruction, rDomDocument.firstChild());
}

//! @brief This function adds the root element including version info to a HMF xml document
//! @param[in] rDomDocument The XML DOM document to append to
//! @param[in] hmfVersion The version string of the hmf file
//! @param[in] hopsanGuiVersion The version string of the Hopsan GUI Application
//! @param[in] hopsanCoreVersion The version string of the Hopsan Core Library
//! @returns The created root DOM element
QDomElement appendHMFRootElement(QDomDocument &rDomDocument, QString hmfVersion, QString hopsanGuiVersion, QString hopsanCoreVersion)
{
    QDomElement hmfRoot = rDomDocument.createElement(hmf::root);
    rDomDocument.appendChild(hmfRoot);
    hmfRoot.setAttribute(hmf::version::hmf, hmfVersion);
    hmfRoot.setAttribute(hmf::version::hopsangui, hopsanGuiVersion);
    hmfRoot.setAttribute(hmf::version::hopsancore, hopsanCoreVersion);
    return hmfRoot;
}

//! @brief Function for adding one initially empty Dom Element to extingd Element
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] element_name The name of the new DOM element
//! @returns The new sub element dom node
QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name)
{
    QDomElement subDomElement = rDomElement.ownerDocument().createElement(element_name);
    rDomElement.appendChild(subDomElement);
    return subDomElement;
}

//! @brief Function to get a sub dom element, if it does not exist it is first added
QDomElement getOrAppendNewDomElement(QDomElement &rDomElement, const QString element_name)
{
    QDomElement elem = rDomElement.firstChildElement(element_name);
    if (elem.isNull())
    {
        return appendDomElement(rDomElement, element_name);
    }
    else
    {
        return elem;
    }
}

//! @brief Function for adding Dom elements containing one text node
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] text The text contents of the text node
QDomElement appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text)
{
    //Only write tag if both name and value is non empty
    QDomElement subDomElement;
    if (!element_name.isEmpty() && !text.isEmpty())
    {
        QDomDocument ownerDomDocument = rDomElement.ownerDocument();
        subDomElement = ownerDomDocument.createElement(element_name);
        subDomElement.appendChild(ownerDomDocument.createTextNode(text));
        rDomElement.appendChild(subDomElement);
    }
    return subDomElement;
}

//! @brief Function for adding Dom elements containing one boolean text node
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] value The boolean value
QDomElement appendDomBooleanNode(QDomElement &rDomElement, const QString element_name, const bool value)
{
    if(value)
    {
        return appendDomTextNode(rDomElement, element_name, hmf::truetag);
    }
    else
    {
        return appendDomTextNode(rDomElement, element_name, hmf::falsetag);
    }
}

//! @brief Function for adding Dom elements containing one text node (based on an integer value)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] val The double value
QDomElement appendDomIntegerNode(QDomElement &rDomElement, const QString element_name, const int val)
{
    QString tmp_string;
    tmp_string.setNum(val);
    return appendDomTextNode(rDomElement, element_name, tmp_string);
}

//! @brief Function for adding Dom elements containing one text node (based on a double value)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] val The double value
//! @param[in] format The format to use when converting the number to text
//! @param[in] precision The precision of the converted value according to format
QDomElement appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val, const char format, const int precision)
{
    return appendDomTextNode(rDomElement, element_name, QString("%1").arg(val, 0, format, precision));
}

//! @brief Function for adding Dom elements containing one text node (based on two double values)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] a The first double value
//! @param[in] b The second double value
//! @param[in] format The format to use when converting the number to text
//! @param[in] precision The precision of the converted value according to format
QDomElement appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b, const char format, const int precision)
{
    return appendDomTextNode(rDomElement, element_name, QString("%1 %2").arg(a,0,format,precision).arg(b,0,format,precision));
}

//! @brief Function for adding Dom elements containing one text node (based on three double values)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] a The first double value
//! @param[in] b The second double value
//! @param[in] c The third double value
//! @param[in] format The format to use when converting the number to text
//! @param[in] precision The precision of the converted value according to format
QDomElement appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c, const char format, const int precision)
{
    return appendDomTextNode(rDomElement, element_name, QString("%1 %2 %3").arg(a,0,format,precision).arg(b,0,format,precision).arg(c,0,format,precision));
}

//! @brief Function for adding Dom elements containing one text node (based on N double values)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] rValues A QVector containing all of the values to add
//! @param[in] format The format to use when converting the number to text
//! @param[in] precision The precision of the converted value according to format
QDomElement appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<double> &rValues, const char format, const int precision)
{
    QString num,str;
    for (int i=0; i<rValues.size(); ++i)
    {
        num.setNum(rValues[i], format, precision);
        str.append(num);
        str.append(" ");
    }
    str.chop(1); //Remove last space
    return appendDomTextNode(rDomElement, element_name, str);
}

//! @brief Helpfunction for adding a double (float or double) attribute to a Dom element while making sure that decimal point is . and not ,
//! @param domElement The DOM element to add the attribute to
//! @param[in] attrName The name of the attribute
//! @param[in] attrValue The value of the attribute
void setQrealAttribute(QDomElement domElement, const QString attrName, const double attrValue, const int precision, const char format)
{
    QString str;
    domElement.setAttribute(attrName, str.setNum(attrValue, format, precision));
}


//! @brief Function that parses one DOM elements containing one text node (based on three double values)
//! @param[in] domElement The DOM Element to parse
//! @param[out] rA The first extracted double value
//! @param[out] rB The second extracted double value
//! @param[out] rC The third extracted double value
void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC)
{
    QStringList poseList = domElement.text().split(" ");
    rA = poseList[0].toDouble();
    rB = poseList[1].toDouble();
    rC = poseList[2].toDouble();
}

//! @brief Function that parses one DOM elements containing one text node (based on two double values)
//! @param[in] domElement The DOM Element to parse
//! @param[out] rA The first extracted double value
//! @param[out] rB The second extracted double value
void parseDomValueNode2(QDomElement domElement, double &rA, double &rB)
{
    QStringList poseList = domElement.text().split(" ");
    if(poseList.size() < 2) return;
    rA = poseList[0].toDouble();
    rB = poseList[1].toDouble();
}

//! @brief Function that parses one DOM elements containing one text node (based on a double value)
//! @param[in] domElement The DOM Element to parse
//! @returns The extracted value
double parseDomValueNode(QDomElement domElement, const double defaultValue)
{
    if ( !domElement.isNull() )
    {
        bool isOk;
        double val = domElement.text().toDouble(&isOk);
        if (isOk)
        {
            return val;
        }
    }
    // If dom was null or if parsing failed we return the default value
    return defaultValue;
}

//! @brief Function that parses one DOM elements containing one text node (based on an integer value)
//! @param[in] domElement The DOM Element to parse
//! @param[in] defaultVal The default value to use if DOM element is null or if parsing fails
//! @returns The extracted value
int parseDomIntegerNode(QDomElement domElement, const int defaultValue)
{
    if ( !domElement.isNull() )
    {
        bool isOk;
        int val = domElement.text().toInt(&isOk);
        if (isOk)
        {
            return val;
        }
    }
    // If dom was null or if parsing failed we return the default value
    return defaultValue;
}


//! @brief Function that parses one DOM elements containing one text node (based on a boolean value)
//! @param[in] domElement The DOM Element to parse
//! @param[in] defaultVal The default value to use if DOM element is null
//! @returns The extracted boolean value
bool parseDomBooleanNode(QDomElement domElement, const bool defaultValue)
{
    // If dom element is null then return default value
    if ( domElement.isNull() )
    {
        return defaultValue;
    }
    // else check if it is true or not (something else = false)
    return (domElement.text() == hmf::truetag);
}

//! @brief Special purpose function for adding a Hopsan specific XML tag containing Object Pose information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
//! @param[in] th The orientation (angle)
//! @param[in] flipped isFlipped status of the object
void appendPoseTag(QDomElement &rDomElement, const double x, const double y, const double th, const bool flipped, const int precision)
{
    QDomElement pose = appendDomElement(rDomElement, hmf::appearance::pose);

    setQrealAttribute(pose, "x", x, precision, 'g');
    setQrealAttribute(pose, "y", y, precision, 'g');
    setQrealAttribute(pose, "a", th, precision, 'g');
    pose.setAttribute("flipped", bool2str(flipped));
}

//! @brief Special purpose function for adding a Hopsan specific XML tag containing a coordinate
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
void appendCoordinateTag(QDomElement &rDomElement, const double x, const double y, const int precision)
{
    QDomElement pose = appendDomElement(rDomElement, hmf::connector::coordinate);
    setQrealAttribute(pose, "x", x, precision);
    setQrealAttribute(pose, "y", y, precision);
}

//! @brief Special purpose help function for adding a Hopsan specific XML tag containing viewport information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
//! @param[in] zoom The zoom factor
void appendViewPortTag(QDomElement &rDomElement, const double x, const double y, const double zoom)
{
    //qDebug() << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << "DecimalPoint: " << QLocale().decimalPoint();
    QDomElement pose = appendDomElement(rDomElement, hmf::appearance::viewport);

    setQrealAttribute(pose, "x", x, 6, 'g');
    setQrealAttribute(pose, "y", y, 6, 'g');
    setQrealAttribute(pose, "zoom", zoom, 6, 'g');

    //qDebug() << "zoom: " << pose.attribute("zoom");
}

//! @brief Special purpose help function for adding a Hopsan specific XML tag containing simulationtime information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] start The starttime
//! @param[in] step The timestep size
//! @param[in] stop The stoptime
void appendSimulationTimeTag(QDomElement &rDomElement, const double start, const double step, const double stop, const bool inheritTs)
{
    QDomElement simu = appendDomElement(rDomElement, hmf::simulationtime);
    setQrealAttribute(simu, "start", start, 10, 'g');
    setQrealAttribute(simu, "timestep", step, 10, 'g');
    setQrealAttribute(simu, "stop", stop, 10, 'g');
    simu.setAttribute("inherit_timestep", bool2str(inheritTs));

}

//! @brief Special purpose help function for adding a Hopsan specific XML tag containing simulationtime information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] start The starttime
//! @param[in] step The timestep size
//! @param[in] stop The stoptime
void appendLogSettingsTag(QDomElement &rDomElement, const double logStartTime, const unsigned int numLogSamples)
{
    QDomElement log = appendDomElement(rDomElement, hmf::simulationlogsettings);
    setQrealAttribute(log, "starttime", logStartTime, 10, 'g');
    log.setAttribute("numsamples", numLogSamples);
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing Object Pose information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rTheta The orientation (angle)
//! @param[out] rFlipped isFlipped status of the object
void parsePoseTag(QDomElement domElement, double &rX, double &rY, double &rTheta, bool &rFlipped)
{
    rX = domElement.attribute("x").toDouble();
    rY = domElement.attribute("y").toDouble();
    rTheta = domElement.attribute("a").toDouble();
    rFlipped = parseAttributeBool(domElement, "flipped", false);
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing a coordinate
//! @param[in] domElement The DOM Element to parse
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
void parseCoordinateTag(QDomElement domElement, double &rX, double &rY)
{
    rX = domElement.attribute("x").toDouble();
    rY = domElement.attribute("y").toDouble();
}

//! @brief Special purpose help function for parsing a Hopsan specific XML tag containing viewport information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rZoom The zoom factor
void parseViewPortTag(QDomElement domElement, double &rX, double &rY, double &rZoom)
{
    rX = domElement.attribute("x").toDouble();
    rY = domElement.attribute("y").toDouble();
    rZoom = domElement.attribute("zoom").toDouble();
}

//! @brief Special purpose help function for parsing a Hopsan specific XML tag containing simulationtime information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rStart The starttime
//! @param[out] rStep The timestep size
//! @param[out] rStop The stoptime
void parseSimulationTimeTag(QDomElement domElement, QString &rStart, QString &rStep, QString &rStop, bool &rInheritTs)
{
    rStart = domElement.attribute("start");
    rStep = domElement.attribute("timestep");
    rStop = domElement.attribute("stop");
    rInheritTs = parseAttributeBool(domElement, "inherit_timestep", true);
}

double parseAttributeQreal(const QDomElement domElement, const QString attributeName, const double defaultValue)
{
    if (domElement.hasAttribute(attributeName))
    {
        bool isOK=false;
        const double val = domElement.attribute(attributeName).toDouble(&isOK);
        if (isOK)
        {
            return val;
        }
    }
    return defaultValue;
}

bool parseAttributeBool(const QDomElement domElement, const QString attributeName, const bool defaultValue)
{
    QString attr = domElement.attribute(attributeName, bool2str(defaultValue));
    if ( (attr==hmf::truetag) || (attr=="True") || (attr=="1"))
    {
        return true;
    }

    return false;
}


//! @brief Converts a color to a string of syntax "R,G,B".
QString makeRgbString(QColor color)
{
    QString red, green,blue;
    red.setNum(color.red());
    green.setNum(color.green());
    blue.setNum(color.blue());
    return QString(red+","+green+","+blue);
}

//! @brief Converts a string of syntax "R,G,B" into three numbers for red, green and blue.

void parseRgbString(QString rgb, double &red, double &green, double &blue)
{
    QStringList split = rgb.split(",");
    red = split[0].toDouble();
    green = split[1].toDouble();
    blue = split[2].toDouble();
}

//! @brief Make sure hmf version is compatible
bool verifyHmfFormatVersion(const QString hmfVersion)
{
    return (hmfVersion >= HMF_REQUIREDVERSIONNUM);
}

//! @brief Handles compatibility issues for elements loaded from hmf files
//! @todo Add check for separate orifice areas in the rest of the valves
//! @todo coreVersion check will not work later when core can save some data by itself, need to use guiversion also
void updateHmfComponentProperties(QDomElement &element, const QString /*hmfVersion*/, QString coreVersion)
{
    updateRenamedComponentType(element, "MechanicRotationalInertiaWithCoulumbFriction", "MechanicRotationalInertiaWithCoulombFriction");

    // For all versions older then 0.6.0 and all 0.6.x DEV versions run the following
    if (isVersionGreaterThan("0.6.0", coreVersion) || (coreVersion.contains("0.6.x_r")))
    {
        // Typos
        updateRenamedComponentType(element, "MechanicTranslationalMassWithCoulumbFriction", "MechanicTranslationalMassWithCoulombFriction");

        updateRenamedComponentType(element, "SignalSource", "SignalConstant");
        updateRenamedPort(element, "SignalConstant", "out", "y");

        updateRenamedPort(element, "MechanicForceTransformer", "in", "F");

        updateRenamedPort(element, "MechanicTranslationalMassWithCoulombFriction", "Pfs", "f_s");
        updateRenamedPort(element, "MechanicTranslationalMassWithCoulombFriction", "Pfk", "f_k");

        updateRenamedPort(element, "MechanicVelocityTransformer", "vin", "v");
        updateRenamedPort(element, "MechanicVelocityTransformer", "xin", "x");

        updateRenamedPort(element, "MechanicTorqueTransformer", "in", "T");

        updateRenamedParameter(element, "HydraulicVariableDisplacementMotorQ", "epsilon_m", "eps");
        updateRenamedParameter(element, "HydraulicVariableDisplacementPump", "epsilon_p", "eps");
        updateRenamedPort(element, "HydraulicVariableDisplacementMotorQ", "in", "eps");
        updateRenamedPort(element, "HydraulicVariableDisplacementPump", "in", "eps");



        // Fix changed parameter names, after introduction of readVariables
        updateRenamedParameter(element, "HydraulicLaminarOrifice", "K_c", "Kc");
        updateRenamedPort(element, "HydraulicPressureSourceC", "In", "p");
        updateRenamedPort(element, "HydraulicPressureSourceQ", "in", "p");
        updateRenamedPort(element, "HydraulicMultiPressureSourceC", "In", "p");
        updateRenamedPort(element, "HydraulicFlowSourceQ", "in", "q");

        // Fix incorrect parameter names
        QDomElement xmlParameter = element.firstChildElement(hmf::parameters).firstChildElement(hmf::parameter::root);
        while (!xmlParameter.isNull())
        {
            // Fix renamed node data variables
            //! @todo :: is used to upconvert old formats, can be removed later after 0.7 release (if 0.7 requires models to be saved in quite new hopsan version)
            if (xmlParameter.attribute("name").contains("#") || xmlParameter.attribute("name").contains("::"))
            {
                QStringList parts;
                if (xmlParameter.attribute("name").contains("#"))
                {
                    parts = xmlParameter.attribute("name").split("#");
                }
                else
                {
                    parts = xmlParameter.attribute("name").split("::");
                }

                if (parts[1] == "Angular Velocity")
                {
                    parts[1] = "AngularVelocity";
                }
                else if (parts[1] == "Equivalent Inertia")
                {
                    parts[1] = "EquivalentInertia";
                }
                else if (parts[1] == "CharImp")
                {
                    parts[1] = "CharImpedance";
                }
                xmlParameter.setAttribute("name", parts[0]+"#"+parts[1]);
            }

            // Fix parameter names with illegal chars
            if (!isNameValid(xmlParameter.attribute("name")))
            {
                QString name = xmlParameter.attribute("name");
                if (name == "sigma^2")
                {
                    name = "std_dev";
                }
                name.replace(',',"");
                name.replace('.',"");
                name.replace(' ',"_");

                xmlParameter.setAttribute("name", name);
            }
            xmlParameter = xmlParameter.nextSiblingElement(hmf::parameter::root);
        }
    }
}


//! @brief Handles compatibility issues for xml data loaded from configuration file
void verifyConfigurationCompatibility(QDomElement &rConfigElement)
{
    qDebug() << "Current version = " << HOPSANGUIVERSION << ", config version = " << rConfigElement.attribute(hmf::version::hopsangui);

    QDomElement unitsElement = rConfigElement.firstChildElement("units");
    if(!unitsElement.isNull())
    {
        QDomElement customUnitElement = unitsElement.firstChildElement("customunit");
        while(!customUnitElement.isNull())
        {
            if(customUnitElement.attribute("name") == "Angular Velocity")
            {
                customUnitElement.setAttribute("name", "AngularVelocity");
            }
            customUnitElement = customUnitElement.nextSiblingElement("customunit");
        }
    }
}

void updateRenamedComponentType(QDomElement &rDomElement, const QString oldType, const QString newType)
{
    if(rDomElement.attribute(hmf::typenametag) == oldType)
    {
        rDomElement.setAttribute(hmf::typenametag, newType);
        QDomElement guiElement = rDomElement.firstChildElement(hmf::hopsangui);
        if(!guiElement.isNull())
        {
            QDomElement cafElement = guiElement.firstChildElement(caf::root);
            if(!cafElement.isNull())
            {
                QDomElement objectElement = cafElement.firstChildElement(caf::modelobject);
                objectElement.setAttribute(hmf::typenametag, newType);
            }
        }
    }
}

void updateRenamedComponentName(QDomElement &rDomElement, const QString oldName, const QString newName)
{
    if((rDomElement.tagName() == hmf::component) && rDomElement.attribute(hmf::name) == oldName)
    {
        rDomElement.setAttribute(hmf::name, newName);
        gpMessageHandler->addWarningMessage("Renamed component: "+oldName+" to "+newName);
        QDomElement guiElement = rDomElement.firstChildElement(hmf::hopsangui);
        if(!guiElement.isNull())
        {
            QDomElement cafElement = guiElement.firstChildElement(caf::root);
            if(!cafElement.isNull())
            {
                QDomElement objectElement = cafElement.firstChildElement(caf::modelobject);
                objectElement.setAttribute("displayname", newName);
            }
        }
    }
    else if ((rDomElement.tagName() == hmf::connector::root) && (rDomElement.attribute(hmf::connector::startcomponent) == oldName)) {
        rDomElement.setAttribute(hmf::connector::startcomponent, newName);
    }
    else if ((rDomElement.tagName() == hmf::connector::root) && (rDomElement.attribute(hmf::connector::endcomponent) == oldName)) {
        rDomElement.setAttribute(hmf::connector::endcomponent, newName);
    }

}

void updateRenamedPort(QDomElement &rDomElement, const QString componentType, const QString oldName, const QString newName)
{
    if(rDomElement.attribute("typename") == componentType)
    {
        // Rename startvalue parameters
        QDomElement parameter = rDomElement.firstChildElement(hmf::parameters).firstChildElement(hmf::parameter::root);
        while (!parameter.isNull())
        {
            QString paramName = parameter.attribute(hmf::name);
            if (paramName.contains(oldName+"#"))
            {
                paramName.replace(oldName+"#",newName+"#");
                parameter.setAttribute(hmf::name, paramName);
            }
            parameter = parameter.nextSiblingElement(hmf::parameter::root);
        }

        // Now try to find all connections, and replace portname
        QString compName = rDomElement.attribute(hmf::name);
        QDomElement connection = rDomElement.parentNode().parentNode().firstChildElement(hmf::connections).firstChildElement(hmf::connector::root);
        while (!connection.isNull())
        {
            QString startComp = connection.attribute(hmf::connector::startcomponent);
            QString endComp = connection.attribute(hmf::connector::endcomponent);

            if (startComp == compName)
            {
                if (connection.attribute(hmf::connector::startport) == oldName)
                {
                    connection.setAttribute(hmf::connector::startport, newName);
                }
            }
            if (endComp == compName)
            {
                if (connection.attribute(hmf::connector::endport) == oldName)
                {
                    connection.setAttribute(hmf::connector::endport, newName);
                }
            }

            connection = connection.nextSiblingElement(hmf::connector::root);
        }
    }
}

void updateRenamedParameter(QDomElement &rDomElement, const QString componentType, const QString oldName, const QString newName)
{
    if(rDomElement.attribute("typename") == componentType)
    {
        QDomElement parameter = rDomElement.firstChildElement(hmf::parameters).firstChildElement(hmf::parameter::root);
        while (!parameter.isNull())
        {
            if (parameter.attribute(hmf::name) == oldName)
            {
                parameter.setAttribute(hmf::name, newName);
                gpMessageHandler->addWarningMessage("Renamed parameter: "+oldName+" to "+newName+" in: "+rDomElement.attribute(hmf::name));
            }
            parameter = parameter.nextSiblingElement(hmf::parameter::root);
        }
    }
}


void parseLogSettingsTag(QDomElement domElement, double &rLogStartTime, int &rNumLogSamples)
{
    rLogStartTime = parseAttributeQreal(domElement, "starttime", 0);
    rNumLogSamples = domElement.attribute("numsamples", "0").toInt();
}


int parseAttributeInt(const QDomElement domElement, const QString attributeName, const int defaultValue)
{
    if (domElement.hasAttribute(attributeName))
    {
        bool isOK=false;
        const int val = domElement.attribute(attributeName).toInt(&isOK);
        if (isOK)
        {
            return val;
        }
    }
    return defaultValue;
}


QDomComment appendComment(QDomElement &rDomElement, const QString &rComment)
{
    QDomComment xmlcomment = rDomElement.ownerDocument().createComment(rComment);
    rDomElement.appendChild(xmlcomment);
    return xmlcomment;
}

//! @brief Function that parses one DOM elements containing one text node
//! @param[in] domElement The DOM Element to parse
//! @param[in] rDefaultVal The default value to use if DOM element is null
//! @returns The extracted value
QString parseDomStringNode(QDomElement domElement, const QString &rDefaultValue)
{
    if ( !domElement.isNull() )
    {
        return domElement.text();
    }
    // If dom was null we return the default value
    return rDefaultValue;
}

void updateHmfSystemProperties(QDomElement &systemElement, const QString hmfVersion, QString coreVersion)
{
    HOPSAN_UNUSED(hmfVersion)

    // For all versions older then 2.16.0
    if (isVersionGreaterThan("2.16.0", coreVersion))
    {
        // Renaming Time to SimulationTime and similar would be nice, but lets do Time_INVALID_NAME instead to minimize the risk of colission, in case a model already
        // have a component named SimulationTime
        QStringList invalidNames = {"Time", "time", "Frequency", "frequency"};

        for (const auto& oldName : invalidNames) {
            QString newName = oldName+"_INVALID_NAME";

            QDomElement xmlComponent = systemElement.firstChildElement(hmf::objects).firstChildElement(hmf::component);
            while (!xmlComponent.isNull()) {
                updateRenamedComponentName(xmlComponent, oldName, newName);
                xmlComponent = xmlComponent.nextSiblingElement(hmf::component);
            }

            QDomElement xmlConnection = systemElement.firstChildElement(hmf::connections).firstChildElement(hmf::connector::root);
            while(!xmlConnection.isNull()) {
                updateRenamedComponentName(xmlConnection, oldName, newName);
                xmlConnection = xmlConnection.nextSiblingElement(hmf::connector::root);
            }

            // This will rename the system parameter and preserve the value, but any user of the parameter must be updated manually
            updateRenamedParameter(systemElement, "Subsystem", oldName, newName);
        }
    }
}


//! @brief Reads a System Structure Parameter Values (SSV) file
//! @param filePath[in] Full path to .ssv file
//! @param rParameters[out] List of parameter specifications
void readFromSsv(const QString filePath, QList<SsvParameter> &rParameters)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        gpMessageHandler->addErrorMessage("Unable to open "+file.fileName()+" for reading.");
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
        gpMessageHandler->addErrorMessage(file.fileName()+QString(": Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr));
        return;
    }


    QDomElement configRoot = domDocument.documentElement();
    if (configRoot.tagName() != ssv::parameterSet) {
        gpMessageHandler->addErrorMessage(file.fileName()+": Incorrect root tag name: "+configRoot.tagName()+" != "+ssv::parameterSet);
        return;
    }

    QDomElement parametersElement = configRoot.firstChildElement(ssv::parameters);

    //Loop through all <ssv:Parameter> elements
    QDomElement parameterElement = parametersElement.firstChildElement(ssv::parameter);
    while(!parameterElement.isNull()) {
        SsvParameter parameter;
        parameter.name = parameterElement.attribute(ssv::attr::name);

        //Read data subelement (name = ssv:[datatype])
        QDomElement dataElement = parameterElement.firstChildElement();
        parameter.dataType = dataElement.nodeName();
        parameter.unit = dataElement.attribute(ssv::attr::unit);
        parameter.value = dataElement.attribute(ssv::attr::value);
        rParameters.append(parameter);

        parameterElement = parameterElement.nextSiblingElement(ssv::parameter);
    }

    file.close();
}
