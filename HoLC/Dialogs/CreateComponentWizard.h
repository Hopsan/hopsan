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

#ifndef CREATECOMPONENTWIZARD_H
#define CREATECOMPONENTWIZARD_H

#include <QWidget>
#include <QWizard>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

class FileHandler;
class MessageHandler;

class NodeInfo
{
    public:
        NodeInfo(QString nodeType);
        static void getNodeTypes(QStringList &nodeTypes);

        QString niceName;
        QStringList qVariables;
        QStringList cVariables;
        QStringList variableLabels;
        QStringList shortNames;
        QList<size_t> varIdx;
        QString intensity;
        QString flow;
};


class CreateComponentWizard : public QWizard
{
    Q_OBJECT

public:
    CreateComponentWizard(FileHandler *pFileHandler, MessageHandler *pMessageHandler, QWidget *parent = 0);

public slots:
    void open();

private slots:
    void updatePage(int i);
    void generate();

private:
    //EditComponentDialog::SourceCodeEnumT mLanguage;

    QWidget *mpParent;

    QLineEdit *mpTypeNameLineEdit;
    QLineEdit *mpDisplayNameLineEdit;
    QComboBox *mpCqsTypeComboBox;
    QSpinBox *mpNumberOfPortsSpinBox;
    QSpinBox *mpNumberOfConstantsSpinBox;
    QSpinBox *mpNumberOfInputsSpinBox;
    QSpinBox *mpNumberOfOutputsSpinBox;

    QGridLayout *mpConstantsLayout;
    QGridLayout *mpInputsLayout;
    QGridLayout *mpOutputsLayout;
    QGridLayout *mpPortsLayout;
    QList<QLabel*> mPortIdPtrs;
    QList<QLineEdit*> mPortNameLineEditPtrs;
    QList<QLineEdit*> mPortDescriptionLineEditPtrs;
    QList<QComboBox*> mPortTypeComboBoxPtrs;
    QList<QDoubleSpinBox*> mPortDefaultSpinBoxPtrs;

    QList<QLineEdit*> mConstantsNameLineEditPtrs;
    QList<QLineEdit*> mConstantsUnitLineEditPtrs;
    QList<QLineEdit*> mConstantsDescriptionLineEditPtrs;
    QList<QLineEdit*> mConstantsValueLineEditPtrs;

    QList<QLineEdit*> mInputsNameLineEditPtrs;
    QList<QLineEdit*> mInputsUnitLineEditPtrs;
    QList<QLineEdit*> mInputsDescriptionLineEditPtrs;
    QList<QLineEdit*> mInputsValueLineEditPtrs;

    QList<QLineEdit*> mOutputsNameLineEditPtrs;
    QList<QLineEdit*> mOutputsUnitLineEditPtrs;
    QList<QLineEdit*> mOutputsDescriptionLineEditPtrs;
    QList<QLineEdit*> mOutputsValueLineEditPtrs;

    FileHandler *mpFileHandler;
    MessageHandler *mpMessageHandler;

};

#endif // CREATECOMPONENTWIZARD_H
