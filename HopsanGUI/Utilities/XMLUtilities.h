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
//! @file   XMLUtilities.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-xx
//!
//! @brief Contains XML DOM help functions that are more or less Hopsan specific
//!
//$Id$

#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include <QtXml>

QString bool2str(const bool in);

QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);

void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument);

QDomElement appendHMFRootElement(QDomDocument &rDomDocument, QString hmfVersion, QString hopsanGuiVersion, QString hopsanCoreVersion);
QDomElement getOrAppendNewDomElement(QDomElement &rDomElement, const QString element_name);

QDomComment appendComment(QDomElement &rDomElement, const QString &rComment);

QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name);
QDomElement appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text);
QDomElement appendDomBooleanNode(QDomElement &rDomElement, const QString element_name, const bool value);
QDomElement appendDomIntegerNode(QDomElement &rDomElement, const QString element_name, const int val);
QDomElement appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val, const char format='g', const int precision=-1);
QDomElement appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b, const char format='g', const int precision=-1);
QDomElement appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c, const char format='g', const int precision=-1);
QDomElement appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<double> &rValues, const char format='g', const int precision=6);

int parseDomIntegerNode(QDomElement, const int defaultValue);
bool parseDomBooleanNode(QDomElement domElement, const bool defaultValue);
double parseDomValueNode(QDomElement domElement, const double defaultValue);
QString parseDomStringNode(QDomElement domElement, const QString &rDefaultValue);
void parseDomValueNode2(QDomElement domElement, double &rA, double &rB);
void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC);

void setQrealAttribute(QDomElement domElement, const QString attrName, const double attrValue, const int precision=6,  const char format='f');

//Attribute help functions
void appendPoseTag(QDomElement &rDomElement, const double x, const double y, const double th, const bool flipped, const int precision=6);
void appendCoordinateTag(QDomElement &rDomElement, const double x, const double y, const int precision=20);
void appendViewPortTag(QDomElement &rDomElement, const double x, const double y, const double zoom);
void appendSimulationTimeTag(QDomElement &rDomElement, const double start, const double step, const double stop, const bool inheritTs);
void appendLogSettingsTag(QDomElement &rDomElement, const double logStartTime, const unsigned int numLogSamples);

void parsePoseTag(QDomElement domElement, double &rX, double &rY, double &rTheta, bool &rFlipped);
void parseCoordinateTag(QDomElement domElement, double &rX, double &rY);
void parseViewPortTag(QDomElement domElement, double &rX, double &rY, double &rZoom);
void parseSimulationTimeTag(QDomElement domElement, QString &rStart, QString &rStep, QString &rStop, bool &rInheritTs);
void parseLogSettingsTag(QDomElement domElement, double &rLogStartTime, int &rNumLogSamples);

bool parseAttributeBool(const QDomElement domElement, const QString attributeName, const bool defaultValue);
int parseAttributeInt(const QDomElement domElement, const QString attributeName, const int defaultValue);
double parseAttributeQreal(const QDomElement domElement, const QString attributeName, const double defaultValue);

//Color help functions
QString makeRgbString(QColor color);
void parseRgbString(QString rgb, double &red, double &green, double &blue);

// Update old models functions
bool verifyHmfFormatVersion(const QString hmfVersion);
void updateHmfComponentProperties(QDomElement &element, const QString hmfVersion, QString coreVersion);
void updateHmfSystemProperties(QDomElement &systemElement, const QString hmfVersion, QString coreVersion);
void verifyConfigurationCompatibility(QDomElement &rConfigElement);
void updateRenamedComponentType(QDomElement &rDomElement, const QString oldType, const QString newType);
void updateRenamedPort(QDomElement &rDomElement, const QString componentType, const QString oldName, const QString newName);
void updateRenamedParameter(QDomElement &rDomElement, const QString componentType, const QString oldName, const QString newName);
void updateRenamedComponentName(QDomElement &rDomElement, const QString oldName, const QString newName);

//SSV help functions
class SsvParameter {
public:
    QString name;
    QString dataType;
    QString unit;
    QString value;
};
void readFromSsv(const QString filePath, QList<SsvParameter> &rParameters);

//Save Load Definitions
//! @todo clean up this list and give some smarter names, remove TAG from end, also make sure we use theses defines where appropriate instead of hardcoded strings
#define HPF_ROOTTAG "hopsanparameterfile"

#define HMF_ROOTTAG "hopsanmodelfile"
#define HMF_OBJECTS "objects"
#define HMF_OBJECTTAG "object"              //Non core Gui Object
#define HMF_MODELOBJECT "modelobject"
#define HMF_COMPONENTTAG "component"
#define HMF_SYSTEMTAG "system"
#define HMF_SYSTEMPORTTAG "systemport"
#define HMF_CONNECTIONS "connections"
#define HMF_CONNECTORTAG "connect"
#define HMF_PARAMETERTAG "parameter"
#define HMF_PARAMETERS "parameters"
#define HMF_NUMHOPSCRIPT "numhopscript"
#define HMF_ALIASES "aliases"
#define HMF_ALIAS "alias"
#define HMF_STARTVALUES "startvalues"
#define HMF_STARTVALUE "startvalue"
#define HMF_GROUPTAG "group"
#define HMF_TEXTBOXWIDGETTAG "textboxwidget"
#define HMF_TEXTWIDGETTAG "textwidget"
#define HMF_BOXWIDGETTAG "boxwidget"
#define HMF_PORTSTAG "ports"
#define HMF_PORTTAG "port"
#define HMF_NAMESTAG "names"
#define HMF_GFXTAG "graphics"
#define HMF_LOGSAMPLES "logsamples"
#define HMF_ANIMATION "animation"

#define HMF_NAMETAG "name"
#define HMF_TYPENAME "typename"
#define HMF_SUBTYPENAME "subtypename"
#define HMF_CQSTYPE "cqstype"
#define HMF_TYPE "type"
#define HMF_UNIT "unit"
#define HMF_QUANTITY "quantity"
#define HMF_CPPCODETAG "cppcode"
#define HMF_CPPINPUTS "inputs"
#define HMF_CPPOUTPUTS "outputs"

#define HMF_HOPSANGUITAG "hopsangui"
#define HMF_COORDINATES "coordinates"
#define HMF_COORDINATETAG "coordinate"
#define HMF_GEOMETRIES "geometries"
#define HMF_GEOMETRYTAG "geometry"
#define HMF_STYLETAG "style"
#define HMF_COLORTAG "color"
#define HMF_XYTAG "xy"                      //Contains an xy coordinate pair
#define HMF_EXTERNALPATHTAG "external_path" //Contains the path to an external subsystem
#define HMF_VALUETAG "value"
#define HMF_SYSTEMPARAMETERTAG "globalkey"
#define HMF_TRUETAG "true"
#define HMF_FALSETAG "false"

#define HMF_PARAMETERSCALES "customparamscales"
#define HMF_PARAMETERSCALE  "customparamscale"
#define HMF_PARAMETERSCALEPARAMNAME "parameter"
#define HMF_PARAMETERSCALEUNIT "unit"
#define HMF_PARAMETERSCALESCALE "scale"
#define HMF_PARAMETERSCALEOFFSET "offset"
#define HMF_PARAMETERSCALEVALUE "value"
#define HMF_PARAMETERSCALEQUANTITY "quantity"

#define HMF_PLOTSCALES "plotscales"
#define HMF_PLOTSCALE "plotscale"
#define HMF_PLOTSCALEPORTDATANAME "portdataname"
#define HMF_PLOTSCALEDESCRIPTION "description"
#define HMF_PLOTSCALEVALUE "scale"

#define HMF_VARIABLEPLOTSETTINGS "variableplotsettings"
#define HMF_VARIABLEPLOTSETTING "variableplotsetting"
#define HMF_VARIABLEINVERTPLOT "variableinvertplot"
#define HMF_VARIABLEPLOTLABEL "variableplotlabel"

#define HMF_POSETAG "pose"
#define HMF_VIEWPORTTAG "viewport"
#define HMF_NAMETEXTTAG "nametext"
#define HMF_NAMETEXTPOSTAG "nametextpos"
#define HMF_VISIBLETAG "nametextvisible"
#define HMF_LOCKEDTAG "locked"
#define HMF_DISABLEDTAG "disabled"

#define HMF_CONNECTORSTARTCOMPONENTTAG "startcomponent"
#define HMF_CONNECTORSTARTPORTTAG "startport"
#define HMF_CONNECTORENDCOMPONENTTAG "endcomponent"
#define HMF_CONNECTORENDPORTTAG "endport"
#define HMF_CONNECTORDASHEDTAG "dashed"

#define HMF_SYSTEMAPPEARANCETAG "systemappearance"

#define HMF_VERSIONTAG "hmfversion"
#define HMF_HOPSANGUIVERSIONTAG "hopsanguiversion"
#define HMF_HOPSANCOREVERSIONTAG "hopsancoreversion"
#define HMF_SIMULATIONTIMETAG "simulationtime"
#define HMF_SIMULATIONLOGSETTINGS "simulationlogsettings"
#define HMF_SCRIPTFILETAG "scriptfile"

#define HMF_UNDO "hopsanundo"

#define HMF_INFOTAG "info"
#define HMF_AUTHORTAG "author"
#define HMF_EMAILTAG "email"
#define HMF_AFFILIATIONTAG "affiliation"
#define HMF_DESCRIPTIONTAG "description"

#define HMF_OPTIMIZATION "optimization"
#define HMF_SEARCHPOINTS "nsearchp"
#define HMF_REFLCOEFF "refcoeff"
#define HMF_RANDOMFACTOR "randfac"
#define HMF_FORGETTINGFACTOR "forgfac"
#define HMF_PARTOL "partol"
#define HMF_PLOT "plot"
#define HMF_SAVECSV "savecsv"
#define HMF_FINALEVAL "finaleval"
#define HMF_LOGPAR "logpar"
#define HMF_OBJECTIVES "objectives"
#define HMF_OBJECTIVE "objective"
#define HMF_FUNCNAME "functionname"
#define HMF_WEIGHT "weight"
#define HMF_NORM "norm"
#define HMF_EXP "exp"
#define HMF_DATA "data"

#define HMF_SENSITIVITYANALYSIS "senstivitityanalysis"
#define HMF_SETTINGS "settings"
#define HMF_ITERATIONS "iterations"
#define HMF_DISTRIBUTIONTYPE "distribution"
#define HMF_PLOTVARIABLES "plotvariables"
#define HMF_PLOTVARIABLE "variable"
#define HMF_UNIFORMDIST "uniform"
#define HMF_NORMALDIST "normal"
#define HMF_MINMAX "minmax"
#define HMF_AVERAGE "average"
#define HMF_SIGMA "sigma"

namespace hmf {
    constexpr auto imagewidget="imagewidget";
    constexpr auto x="x";
    constexpr auto y="y";
    constexpr auto image="image";
    constexpr auto path="path";
    constexpr auto scale="scale";
    constexpr auto index="index";
}

#define XML_LIBS "libs"
#define XML_USERLIB "userlib"
#define XML_LIBTYPE "libtype"
#define XML_LIBTYPE_INTERNAL "internal"
#define XML_LIBTYPE_EXTERNAL "external"
#define XML_LIBTYPE_FMU "fmu"

#define XML_MODELS "models"
#define XML_LASTSESSIONMODEL "lastsessionmodel"
#define XML_RECENTMODEL "recentmodel"
#define XML_RECENTGENERATORMODEL "recentgeneratormodel"

namespace ssv {
    constexpr auto parameterSet = "ssv:ParameterSet";
    constexpr auto parameters = "ssv:Parameters";
    constexpr auto parameter = "ssv:Parameter";
    constexpr auto units = "ssv:Units";
    constexpr auto unit = "ssv:Unit";
    constexpr auto baseUnit = "ssv:BaseUnit";
    constexpr auto name = "name";
    constexpr auto offset = "offset";
    constexpr auto url = "http://ssp-standard.org/SSP1/SystemStructureParameterValues";

    namespace datatype {
        constexpr auto real = "ssv:Real";
        constexpr auto integer = "ssv:Integer";
        constexpr auto boolean = "ssv:Boolean";
        constexpr auto string = "ssv:String";
    }

    namespace attr {
        constexpr auto name = "name";
        constexpr auto value = "value";
        constexpr auto factor = "factor";
        constexpr auto offset = "offset";
        constexpr auto unit = "unit";
        constexpr auto version = "version";
        constexpr auto xmlns = "xmlns:ssv";
        constexpr auto kg =  "kg";
        constexpr auto m = "m";
        constexpr auto s = "s";
        constexpr auto A = "A";
        constexpr auto K = "K";
        constexpr auto mol = "mol";
        constexpr auto cd = "cd";
        constexpr auto rad = "rad";
    }

    static QMap<QString, QString> dataTypeTranslator{{"double",      ssv::datatype::real},
                                                     {"integer",     ssv::datatype::integer},
                                                     {"bool",        ssv::datatype::boolean},
                                                     {"string",      ssv::datatype::string},
                                                     {"textblock",   ssv::datatype::string},
                                                     {"filepath",    ssv::datatype::string},
                                                     {"conditional", ssv::datatype::integer}};
}

#endif // XMLUTILITIES_H
