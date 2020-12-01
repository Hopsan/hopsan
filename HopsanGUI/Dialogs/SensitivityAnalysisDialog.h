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
//! @file   SensitivityAnalysisDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the sensitivity analysis dialog
//!
//$Id$

#ifndef SENSITIVITYANALYSISDIALOG_H
#define SENSITIVITYANALYSISDIALOG_H

#include "OpsEvaluator.h"
#include "OpsWorkerParameterSweep.h"
#include "OpsMessageHandler.h"

#include <QDialog>

// Qt forward declaration
class QTreeWidgetItem;
class QTreeWidget;
class QGridLayout;
class QSpinBox;
class QRadioButton;
class QProgressBar;
class QLabel;
class QLineEdit;
class QCheckBox;

// Hopsan forward declaration
class ModelWidget;
class SensitivityAnalysisSettings;
class SystemObject;
class SensitivityAnalysisDialog;
class RemoteSimulationQueueHandler;

class SensitivityAnalysisEvaluator : public Ops::Evaluator
{
    friend class SensitivityAnalysisDialog;
public:
    SensitivityAnalysisEvaluator(SensitivityAnalysisDialog *pDialog, ModelWidget *pModel, QVector<QPair<double, double> > parLimits, QStringList &parComps, QStringList &par, QList<QStringList> &outputVars, int nThreads);
    bool init();
    void finalize();
    void evaluateAllPoints();
    void evaluateAllCandidates();
private:
    void plot();
    void setParameters(std::vector<std::vector<double> > *pPoints);
    SensitivityAnalysisDialog *mpDialog;
    QVector<QPair<double, double> > mLimits;
    ModelWidget *mpModel;
    QVector<ModelWidget *> mModelPtrs;
    QStringList mParComps, mPars;
    QList<QStringList> mOutputVars;
    RemoteSimulationQueueHandler *mpRemoteSimulationQueueHandler=0;

    int mNumParallelModels=1;
    bool mUseRemoteSimulators=false;
};




class SensitivityAnalysisMessageHandler : public QObject, public Ops::MessageHandler
{
    Q_OBJECT
public:
    SensitivityAnalysisMessageHandler(SensitivityAnalysisDialog *pDialog);
    void stepCompleted(size_t);

public slots:
    void abort();

private:
    SensitivityAnalysisDialog *mpDialog;
};


class SensitivityAnalysisDialog : public QDialog
{
    Q_OBJECT

    friend class SensitivityAnalysisMessageHandler;
public:
    SensitivityAnalysisDialog(QWidget *parent = 0);
    enum DistributionEnumT {UniformDistribution, NormalDistribution};

public slots:
    void open();
    void loadSettings();
    void saveSettings();
    void updateProgressBar(int i);

private slots:
    void updateChosenParameters(QTreeWidgetItem* item, int i);
    void updateChosenVariables(QTreeWidgetItem* item, int i);
    void run();
    void abort();

private:
    //Parameters
    QTreeWidget *mpParametersList;
    //QLabel *mpParametersLabel;
    //QLabel *mpParameterNameLabel;
    //QLabel *mpParameterAverageLabel;
    //QLabel *mpParameterSigmaLabel;
    QGridLayout *mpParametersLayout;
    //QGroupBox *mpParametersGroupBox;

    //Output
    QTreeWidget *mpOutputList;
    //QLabel *mpOutputLabel;
    //QLabel *mpOutputNameLabel;
    QGridLayout *mpOutputLayout;
    //QGroupBox *mpOutputGroupBox;

    //Member widgets
    QCheckBox *mpUseRemoteSimulatorsCheckBox;
    QSpinBox *mpNumRemoteParallelModelsSpinBox;
    QSpinBox *mpStepsSpinBox;
    QRadioButton *mpUniformDistributionRadioButton;
    QRadioButton *mpNormalDistributionRadioButton;
    QProgressBar *mpProgressBar;
    QPushButton *mpAbortButton;

    //Member variables
    ModelWidget *mpModel;
    SensitivityAnalysisSettings *mpSettings;
    QStringList mSelectedComponents;
    QStringList mSelectedParameters;
    QList<QLabel*> mpParameterLabels;
    QList<QLineEdit*> mpParameterAverageLineEdits;
    QList<QLineEdit*> mpParameterSigmaLineEdits;
    QList<QLineEdit*> mpParameterMinLineEdits;
    QList<QLineEdit*> mpParameterMaxLineEdits;

    QList<QStringList> mOutputVariables;
    QList<QLabel*> mpOutputLabels;

    bool mAborted;

    SensitivityAnalysisEvaluator *mpEvaluator;
    Ops::WorkerParameterSweep *mpWorker;

    size_t mNumIterations;
};

#endif // SENSITIVITYANALYSISDIALOG_H
