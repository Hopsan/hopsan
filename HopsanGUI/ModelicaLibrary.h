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
    void reload();
    void loadModelicaFile();
    void loadModelicaFile(const QString &fileName);
    void unloadModelicaFile(const QString &fileName);
    QStringList getModelNames() const;
    QStringList getConnectorNames() const;
    bool hasModel(const QString &rModelName);
    ModelicaModel getModel(const QString &rModelName);
    void getConnector(const QString &rConnectorName, ModelicaConnector &rConnector);
    void getModelicaFiles(QStringList &files) const;
private:
    QMap<QString, ModelicaConnector> mConnectorsMap;
    QMap<QString, ModelicaModel> mModelsMap;
    QStringList mModelicaFiles;
};

#endif // MODELICAHANDLER_H
