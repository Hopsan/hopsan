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
//! @file   ModelicaHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-14
//!
//! @brief Contains a handler for modelica models
//!
//$Id: ModelHandler.h 6629 2014-02-24 14:06:24Z petno25 $

#ifndef MODELICAHANDLER_H
#define MODELICAHANDLER_H

#include <QStringList>
#include <QMap>


class ModelicaVariable
{
public:
    ModelicaVariable(const QString &name, const QString &type, const double defaultValue = 0);
    const QString &getName() const;
    const QString &getType() const;
    double getDefaultValue() const;

    bool operator ==(const ModelicaVariable &rhs);
private:
    QString mName;
    QString mType;
    double mDefaultValue;
};


class ModelicaModel
{
public:
    ModelicaModel() {}
    ModelicaModel(const QString &rCode);
    ModelicaModel(const ModelicaModel &rOther);
    QList<ModelicaVariable> getParameters() const;
    QList<ModelicaVariable> getVariables() const;
    void getInitAlgorithms(QStringList &rAlgorithms) const;
    void getPreAlgorithms(QStringList &rAlgorithms) const;
    void getEquations(QStringList &rEquations) const;
    QString getAnnotations() const;
    void toFlatEquations(QStringList &rInitAlgorithms, QStringList &rPreAlgorighms, QStringList &rEquations, QMap<QString, QString> &rLocalVars, const QString &rPrefix="", const QStringList &rSubModels=QStringList());
private:
    QStringList mCodeLines;
};



class ModelicaConnector
{
public:
    ModelicaConnector() {}
    ModelicaConnector(const QString &rCode);
    void getIntensityVariables(QMap<QString, QString> &rVariables);
    void getFlowVariables(QMap<QString, QString> &rVariables);
private:
    QStringList mCodeLines;
};


class ModelicaLibrary
{
public:
    ModelicaLibrary();
    void loadFile(const QString &code);
    QStringList getModelNames() const;
    QStringList getConnectorNames() const;
    bool hasModel(const QString &rModelName);
    ModelicaModel getModel(const QString &rModelName);
    void getConnector(const QString &rConnectorName, ModelicaConnector &rConnector);
private:
    QMap<QString, ModelicaConnector> mConnectorsMap;
    QMap<QString, ModelicaModel> mModelsMap;
};

#endif // MODELICAHANDLER_H
