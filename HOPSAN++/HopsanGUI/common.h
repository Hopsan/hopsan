/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   common.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains HopsanGUI common global definitions, enums and objects
//!
//$Id$

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDesktopServices>
#include <QSplashScreen>

//Numerical definitions
#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10
#define XMLINDENTATION 2

//Web link definitions
#define HOPSANLINK "http://tiny.cc/hopsan"
#define VERSIONLINK "http://tiny.cc/hopsannews"
#define NEWSLINK "http://tiny.cc/hopsannewsfeed"
#define DOWNLOADLINK "http://tiny.cc/hopsanarchive"
#define AUTOUPDATELINK "http://tiny.cc/hopsanupdate"

// Path definitions (development and release)
// qrc paths
#define OBJECTICONPATH ":graphics/objecticons/"
#define BUILTINCAFPATH ":graphics/builtinCAF/"
#define ICONPATH ":graphics/uiicons/"
#define PORTICONPATH ":graphics/porticons/"
#define GRAPHICSPATH ":graphics/"
#define SOUNDSPATH ":sounds/"

// Gui TypeName defines
#define HOPSANGUIGROUPTYPENAME "HopsanGUIGroup"
#define HOPSANGUISYSTEMTYPENAME "Subsystem"
#define HOPSANGUICONTAINERPORTTYPENAME "HopsanGUIContainerPort"
#define HOPSANGUISCOPECOMPONENTTYPENAME "SignalSink"

#ifdef WIN32
#define LIBEXT ".dll"
#else
#define LIBEXT ".so"
#endif

//! @todo all of these dont have to be common
//Enums
enum SelectionStatusEnumT {Deselected, Selected};
enum NameVisibilityEnumT {NameVisible, NameNotVisible, UseDefault};
enum GraphicsTypeEnumT {UserGraphics, ISOGraphics, NoGraphics};
enum ConnectorGeometryEnumT {Vertical, Horizontal, Diagonal};
enum RenameRestrictionEnumT {Unrestricted, CoreRenameOnly};
enum UndoStatusEnumT {NoUndo, Undo};
enum SaveTargetEnumT {ExistingFile, NewFile};
enum SaveContentsEnumT {FullModel, ParametersOnly, CurrentContainerOnly};
enum ConnectorStyleEnumT {PowerConnectorStyle, SignalConnectorStyle, BrokenConnectorStyle, UndefinedConnectorStyle};
enum HopsanPlotIDEnumT {FirstPlot, SecondPlot};
enum HopsanPlotCurveTypeEnumT {PortVariableType, FrequencyAnalysisType, NyquistType, BodeGainType, BodePhaseType, GeneralType};
enum ContainerChildrenZValuesEnumT {WidgetZValue, ConnectorZValue, ModelobjectZValue, HoveredConnectorZValue, HoveredModelobjectZValue};
enum ModelObjectZValuesEnumT {PortZValue, SelectionboxZValue, HoveredPortZValue, LossesDisplayZValue};
enum PortZValuesEnumT {MultiportOverlayZValue, CQSOverlayZValue, PortLabelZValue};
enum YesNoToAllEnumT {UndecidedToAll, NoToAll, YesToAll};
enum InternalExternalEnumT {Internal, External};

//Global pointer to the main window
class MainWindow;
extern MainWindow* gpMainWindow;

class Configuration;
extern Configuration gConfig;

class DesktopHandler;
extern DesktopHandler gDesktopHandler;

class CopyStack;
extern CopyStack gCopyStack;

class PlotHandler;
extern PlotHandler *gpPlotHandler;

extern QSplashScreen *gpSplash;

extern QString gHopsanCoreVersion;

//! @todo this should not be in common.h
class UnitScale
{
public:
    UnitScale() {}
    UnitScale(const QString &rUnit, const QString &rScale) : mUnit(rUnit), mScale(rScale) {}
    UnitScale(const QString &rUnit, const double scale) : mUnit(rUnit)
    {
        setScale(scale);
    }
    void clear() {mUnit.clear(); mScale.clear();}
    double toDouble() const {return mScale.toDouble();}
    bool isEmpty() const {return mScale.isEmpty();}
    void setScale(const double scale)
    {
        mScale = QString("%1").arg(scale);
    }
    void setOnlyScale(const double scale)
    {
        mUnit.clear();
        mScale = QString("%1").arg(scale);
    }
    QString mUnit;
    QString mScale;
};

#endif // COMMON_H
