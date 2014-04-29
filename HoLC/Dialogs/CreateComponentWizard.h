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
