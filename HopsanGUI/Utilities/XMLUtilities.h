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

QDomElement appendHMFRootElement(QDomDocument &rDomDocument, QString hmfVersion, QString hopsanGuiVersion, QString hopsanCoreVersion, QString customType=QString());
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
namespace hpf {
    constexpr auto root = "hopsanparameterfile";
}

namespace hmf {
    constexpr auto root = "hopsanmodelfile";
    constexpr auto truetag = "true";
    constexpr auto falsetag = "false";
    constexpr auto name = "name";
    constexpr auto signalquantity = "signalquantity";
    constexpr auto fullname = "fullname";
    constexpr auto objects = "objects";
    constexpr auto object = "object";              //non core gui object
    constexpr auto modelobject = "modelobject";
    constexpr auto component = "component";
    constexpr auto system = "system";
    constexpr auto systemport = "systemport";
    constexpr auto parameters = "parameters";
    constexpr auto numhopscript = "numhopscript";
    constexpr auto aliases = "aliases";
    constexpr auto alias = "alias";
    constexpr auto startvalues = "startvalues";
    constexpr auto startvalue = "startvalue";
    constexpr auto group = "group";
    constexpr auto ports = "ports";
    constexpr auto port = "port";
    constexpr auto names = "names";
    constexpr auto data = "data";

    constexpr auto logsamples = "logsamples";
    constexpr auto animation = "animation";

    constexpr auto typenametag = "typename";
    constexpr auto subtypename = "subtypename";
    constexpr auto cqstype = "cqstype";
    constexpr auto type = "type";
    constexpr auto internal = "internal";
    constexpr auto unit = "unit";
    constexpr auto quantity = "quantity";
    constexpr auto cppcode = "cppcode";
    constexpr auto cppinputs = "inputs";
    constexpr auto cppoutputs = "outputs";
    constexpr auto hopsangui = "hopsangui";
    constexpr auto externalpath = "external_path"; //contains the path to an external subsystem
    constexpr auto value = "value";
    constexpr auto systemparameter = "globalkey";
    constexpr auto connections = "connections";
    constexpr auto simulationtime = "simulationtime";
    constexpr auto simulationlogsettings = "simulationlogsettings";
    constexpr auto scriptfile = "scriptfile";
    constexpr auto undo = "hopsanundo";
    constexpr auto customtype = "customtype";

    namespace widget {
        constexpr auto textboxwidget = "textboxwidget";
        constexpr auto imagewidget = "imagewidget";
        constexpr auto x = "x";
        constexpr auto y = "y";
        constexpr auto image = "image";
        constexpr auto path = "path";
        constexpr auto scale = "scale";
        constexpr auto index = "index";
        constexpr auto color = "color";
        constexpr auto style = "style";
        constexpr auto line = "line";
        constexpr auto solidline = "solidline";
        constexpr auto dashline = "dashline";
        constexpr auto dotline = "dotline";
        constexpr auto dashdotline = "dashdotline";
        constexpr auto width = "width";
        constexpr auto height = "height";
        constexpr auto visible = "visible";
        constexpr auto size = "size";
        constexpr auto textobject = "textobject";
        constexpr auto text = "text";
        constexpr auto font = "font";
        constexpr auto fontcolor = "fontcolor";
        constexpr auto reflow = "reflow";
    }





    namespace parameter {
        constexpr auto root = "parameter";
        constexpr auto scales = "customparamscales";
        constexpr auto scale  = "customparamscale";
        constexpr auto scaleparametername = "parameter";
        constexpr auto scaleunit = "unit";
        constexpr auto scalescale = "scale";
        constexpr auto scaleoffset = "offset";
        constexpr auto scalevalue = "value";
        constexpr auto scalequantity = "quantity";
    }

    namespace plot {
        constexpr auto invert = "invert";
        constexpr auto label = "label";
    }

    namespace variable {
        constexpr auto plotsettings = "variableplotsettings";
        constexpr auto plotsetting = "variableplotsetting";
        constexpr auto invertplot = "variableinvertplot";
        constexpr auto plotlabel = "variableplotlabel";
    }

    namespace appearance {

        constexpr auto graphics = "graphics";
        constexpr auto pose = "pose";
        constexpr auto viewport = "viewport";
        constexpr auto nametext = "nametext";
        constexpr auto nametextpos = "nametextpos";
        constexpr auto visible = "nametextvisible";
        constexpr auto locked = "locked";
        constexpr auto disabled = "disabled";
        constexpr auto position = "position";
        constexpr auto alwaysvisible = "alwaysvisible";
    }

    namespace connector {
        constexpr auto root = "connect";
        constexpr auto startcomponent = "startcomponent";
        constexpr auto startport = "startport";
        constexpr auto endcomponent = "endcomponent";
        constexpr auto endport = "endport";
        constexpr auto dashed = "dashed";
        constexpr auto coordinates = "coordinates";
        constexpr auto coordinate = "coordinate";
        constexpr auto geometries = "geometries";
        constexpr auto geometry = "geometry";
        constexpr auto style = "style";
        constexpr auto color = "color";
        constexpr auto diagonal = "diagonal";
    }

    namespace version {
        constexpr auto hmf = "hmfversion";
        constexpr auto hopsangui = "hopsanguiversion";
        constexpr auto hopsancore = "hopsancoreversion";
    }

    namespace modelinfo {
        constexpr auto root = "info";
        constexpr auto author = "author";
        constexpr auto email = "email";
        constexpr auto affiliation = "affiliation";
        constexpr auto description = "description";
    }

    namespace optimization {
        constexpr auto root = "optimization";
        constexpr auto numberofsearchpoints = "nsearchp";
        constexpr auto reflectioncoefficient = "refcoeff";
        constexpr auto randomfactor = "randfac";
        constexpr auto forgettingfactor = "forgfac";
        constexpr auto partol = "partol";
        constexpr auto plot = "plot";
        constexpr auto savecsv = "savecsv";
        constexpr auto finaleval = "finaleval";
        constexpr auto logpar = "logpar";
        constexpr auto objectives = "objectives";
        constexpr auto objective = "objective";
        constexpr auto functionname = "functionname";
        constexpr auto weight = "weight";
        constexpr auto norm = "norm";
        constexpr auto exp = "exp";
        constexpr auto data = "data";
        }

    namespace sensitivityanalysis {
        constexpr auto root = "senstivitityanalysis";
        constexpr auto settings = "settings";
        constexpr auto iterations = "iterations";
        constexpr auto distribution = "distribution";
        constexpr auto plotvariables = "plotvariables";
        constexpr auto plotvariable = "variable";
        constexpr auto uniformdistribution = "uniform";
        constexpr auto normaldistribution = "normal";
        constexpr auto minmax = "minmax";
        constexpr auto average = "average";
        constexpr auto sigma = "sigma";
    }
    namespace deprecated {
        constexpr auto variable = "variable";
        constexpr auto portname = "portname";
        constexpr auto value = "value";
    }
}

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

namespace plotwindow {
    constexpr auto hopsanplot = "hopsanplot";
    constexpr auto plottab = "plottab";
    constexpr auto curve = "curve";
    constexpr auto xcurve = "xcurve";
    constexpr auto grid = "grid";
    constexpr auto color = "color";
    constexpr auto generation = "component";
    constexpr auto component = "component";
    constexpr auto port = "port";
    constexpr auto data = "data";
    constexpr auto axis = "axis";
    constexpr auto width = "width";
}

#endif // XMLUTILITIES_H
