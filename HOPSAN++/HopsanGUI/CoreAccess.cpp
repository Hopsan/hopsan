//$Id$

#include "CoreAccess.h"
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include <QString>
#include <QVector>

using namespace std;
using namespace hopsan;


size_t CoreMessagesAccess::getNumberOfMessages()
{
    return HopsanEssentials::getInstance()->checkMessage();
}

void CoreMessagesAccess::getMessage(QString &message, QString &type, QString &tag)
{
    hopsan::HopsanCoreMessage coreMsg = HopsanEssentials::getInstance()->getMessage();
    message = QString(coreMsg.message.c_str());
    tag = QString(coreMsg.tag.c_str());
    switch (coreMsg.type)
    {
    case hopsan::HopsanCoreMessage::ERROR:
        type = "error";
        break;
    case hopsan::HopsanCoreMessage::WARNING:
        type = "warning";
        break;
    case hopsan::HopsanCoreMessage::INFO:
        type = "info";
        break;
    case hopsan::HopsanCoreMessage::DEBUG:
        type = "debug";
        break;
    }
}


CoreSystemAccess::CoreSystemAccess(QString name, CoreSystemAccess* pParentCoreSystemAccess)
{
    //Create new Core system component
    if (pParentCoreSystemAccess == 0)
    {
        //Create new root system
        mpCoreComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
    }
    else
    {
        //Creating a subsystem, setting internal pointer
        mpCoreComponentSystem = pParentCoreSystemAccess->getCoreSubSystemPtr(name);
    }
}

ComponentSystem* CoreSystemAccess::getCoreSubSystemPtr(QString name)
{
    qDebug() << " corecomponentsystemname: " <<  QString::fromStdString(mpCoreComponentSystem->getName()) << "  Subname: " << name;
    return mpCoreComponentSystem->getSubComponentSystem(name.toStdString());
}

CoreSystemAccess::~CoreSystemAccess()
{
    //Dont remove the mpCoreComponentSystem here you must do that manually until we have found a samrter way to do all of this
    //see deleteRootSystemPtr()
    //delete mpCoreComponentSystem;
}

//! @todo This is very strange, needed becouse core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the ore object
void CoreSystemAccess::deleteRootSystemPtr()
{
    delete mpCoreComponentSystem;
}

bool CoreSystemAccess::connect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->connect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

bool CoreSystemAccess::disconnect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->disconnect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

void CoreSystemAccess::setDesiredTimeStep(double timestep)
{
    mpCoreComponentSystem->setDesiredTimestep(timestep);
}

double CoreSystemAccess::getDesiredTimeStep()
{
    return mpCoreComponentSystem->getDesiredTimeStep();
}

void CoreSystemAccess::setRootTypeCQS(const QString cqs_type)
{
    qDebug () << "setting cqs to: " << cqs_type;
    mpCoreComponentSystem->setTypeCQS(cqs_type.toStdString());
}

void CoreSystemAccess::setSubSystemTypeCQS(QString systemName, const QString cqs_type)
{
    mpCoreComponentSystem->getSubComponentSystem(systemName.toStdString())->setTypeCQS(cqs_type.toStdString());
}

QString CoreSystemAccess::getRootSystemTypeCQS()
{
    //qDebug() << "getRootTypeCQS: " << componentName;
    return QString::fromStdString(mpCoreComponentSystem->getTypeCQSString());
}

QString CoreSystemAccess::getSubComponentTypeCQS(QString componentName)
{
    //qDebug() << "getSubComponentTypeCQS: " << componentName << " in " << QString::fromStdString(mpCoreComponentSystem->getName());
    QString ans = QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getTypeCQSString());
    //qDebug() << "cqs answer: " << ans;
    return ans;
}


QString CoreSystemAccess::setRootSystemName(QString name)
{
    //qDebug() << "setting root system name to: " << name;
    mpCoreComponentSystem->setName(name.toStdString());
    //qDebug() << "root system name after rename: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}


QString CoreSystemAccess::renameSubComponent(QString componentName, QString name)
{
    qDebug() << "rename subcomponent from " << componentName << " to: " << name;
    Component *pTempComponent = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    pTempComponent->setName(name.toStdString());
    qDebug() << "name after: " << QString::fromStdString(pTempComponent->getName());
    return QString::fromStdString(pTempComponent->getName());
}

QString CoreSystemAccess::getRootSystemName()
{
   // qDebug() << "getNAme from core root: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}

double CoreSystemAccess::getCurrentTime()
{
    return *(mpCoreComponentSystem->getTimePtr());
}

void CoreSystemAccess::stop()
{
    mpCoreComponentSystem->stop();
}

QString CoreSystemAccess::getPortType(QString componentName, QString portName)
{
    //qDebug() << "name for port fetch " << componentName << " " << portName;
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        return QString(pPort->getPortTypeString().c_str());
    }
    else
    {
        qDebug() <<  "================================= EMPTY porttype: " << componentName << " " << portName << " in: " << this->getRootSystemName();
        return QString(); //Empty
    }
}

QString CoreSystemAccess::getNodeType(QString componentName, QString portName)
{
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        return QString(pPort->getNodeType().c_str());
    }
    else
    {
        qDebug() <<  "======================================== EMPTY nodetype: " << componentName << " " << portName << " in: " << this->getRootSystemName();
        return QString(); //Empty
    }
}


void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    std::vector<std::string> stdNames, stdUnits;
    std::vector<double> stdValues;
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValues, stdUnits);
    }
    rNames.resize(stdNames.size());
    rValues.resize(stdValues.size());
    rUnits.resize(stdUnits.size());
    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        rNames[i] = QString::fromStdString(stdNames[i]);
        rValues[i] = stdValues[i];
        rUnits[i] = QString::fromStdString(stdUnits[i]);
    }
}


void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
{
    std::vector<std::string> stdNames, stdUnits;
    std::vector<std::string> stdValuesTxt;
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValuesTxt, stdUnits);
    }
    rNames.resize(stdNames.size());
    rValuesTxt.resize(stdValuesTxt.size());
    rUnits.resize(stdUnits.size());
    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        rNames[i] = QString::fromStdString(stdNames[i]);
        rValuesTxt[i] = QString::fromStdString(stdValuesTxt[i]);
        rUnits[i] = QString::fromStdString(stdUnits[i]);
    }
}


void CoreSystemAccess::setStartValueDataByNames(QString componentName, QString portName, QVector<QString> names, QVector<double> values)
{
    std::vector<std::string> stdNames;
    std::vector<double> stdValues;
    stdNames.resize(names.size());
    stdValues.resize(values.size());
    for(int i=0; i < names.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        stdNames[i] = names[i].toStdString();
    }
    stdValues = values.toStdVector();

    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        pPort->setStartValueDataByNames(stdNames, stdValues);
    }
}


void CoreSystemAccess::setStartValueDataByNames(QString componentName, QString portName, QVector<QString> names, QVector<QString> valuesTxt)
{
    //Used for plain values
    QVector<QString> startDataNamesStr;
    QVector<double> startDataValuesDbl;
    //Used for mapped to system parameters start values
    QVector<QString> startDataNamesStrSysPar;
    QVector<QString> startDataValuesTxtSysPar;

    for(int i=0; i < names.size(); ++i)
    {
        bool isDbl;
        //Check if the start value is convertible to a double, if so assume that it is just a plain value that should be used,
        //if not assume that it should be mapped to a System parameter
        double valueDbl = valuesTxt[i].toDouble(&isDbl);
        if(isDbl)
        {
            //Save the start values that should be set to just plain values (e.g. 17.0) into tmp vectors
            startDataNamesStr.append(names[i]);
            startDataValuesDbl.append(valueDbl);
        }
        else
        {
            //Save the System parameter name (e.g. "myparameter") that should be mapped with the start value
            startDataNamesStrSysPar.append(names[i]);
            startDataValuesTxtSysPar.append(valuesTxt[i]);
        }
    }
    //Set this plain start values to the ports
    this->setStartValueDataByNames(componentName, portName, startDataNamesStr, startDataValuesDbl);

    //Set the System parameters to the ports
    std::vector<std::string> stdNames, stdValuesTxt;
    stdNames.resize(startDataNamesStrSysPar.size());
    stdValuesTxt.resize(startDataValuesTxtSysPar.size());
    for(int i=0; i < startDataNamesStrSysPar.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        stdNames[i] = startDataNamesStrSysPar[i].toStdString();
        stdValuesTxt[i] = startDataValuesTxtSysPar[i].toStdString();
    }

    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        pPort->setStartValueDataByNames(stdNames, stdValuesTxt);
    }
}


void CoreSystemAccess::setParameter(QString componentName, QString parameterName, double value)
{
    mpCoreComponentSystem->getSubComponent(componentName.toStdString())->setParameterValue(parameterName.toStdString(), value);
}

bool CoreSystemAccess::setParameter(QString componentName, QString parameterName, QString valueTxt)
{
    bool isDbl;
    //Check if the parameter is convertible to a double, if so assume that it is just a plain value that should be used
    double valueDbl = valueTxt.toDouble(&isDbl);

    if(!isDbl)     //Should be set/mapped to a system parameter
    {
        if(hasSystemParameter(valueTxt))
        {
            mpCoreComponentSystem->getSubComponent(componentName.toStdString())->setParameterValue(parameterName.toStdString(), valueTxt.toStdString());
        }
        else    //User has written something illegal
        {
            //! @todo Make something better, like showing a warning box, if parameter is not ok. Maybe check all parameters before setting any of them.
            MessageWidget *messageWidget = gpMainWindow->mpMessageWidget;//qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent()->parent())->mpMessageWidget;
            messageWidget->printGUIInfoMessage(QString("ComponentPropertiesDialog::setParameters(): You must give a correct value for '").append(parameterName).append(QString("', putz. Try again!")));
            qDebug() << "Inte okej!";
            return false;
        }
    }
    else
    {
        //The parameter is just a simple double with no mapping to System parameter
        setParameter(componentName, parameterName, valueDbl);
    }
    return true;
}

void CoreSystemAccess::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(componentName.toStdString(), doDelete);
}


//! @brief This function loads a system from a file without adding any graphics
//! It calls the core load function and adds all contents into the specified subsystem
//! @param [in] sysname The name of the subsystem in which to add loaded contents
//! @param [in] filepath The path to the file to load from
void CoreSystemAccess::loadSystemFromFileCoreOnly(QString sysname, QString filepath)
{
    mpCoreComponentSystem->getSubComponentSystem(sysname.toStdString())->loadSystemFromFile(filepath.toStdString());
}


vector<double> CoreSystemAccess::getTimeVector(QString componentName, QString portName)
{
    vector<double>* ptr = (mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getPort(portName.toStdString())->getTimeVectorPtr());
    if (ptr != 0)
    {
        return *ptr;
    }
    else
    {
        //Return empty dummy
        vector<double> dummy;
        return dummy;
    }
}


bool CoreSystemAccess::isSimulationOk()
{
    return mpCoreComponentSystem->isSimulationOk();
}


void CoreSystemAccess::initialize(double mStartTime, double mFinishTime, size_t nSamples)
{
    mpCoreComponentSystem->initialize(mStartTime, mFinishTime, nSamples);
}


void CoreSystemAccess::simulate(double mStartTime, double mFinishTime, simulationMethod type, size_t nThreads)
{
    if(type == MULTICORE)
    {
        qDebug() << "Starting multicore simulation";
        mpCoreComponentSystem->simulateMultiThreaded(mStartTime, mFinishTime, nThreads);
    }
    else
    {
        qDebug() << "Starting singlecore simulation";
        mpCoreComponentSystem->simulate(mStartTime, mFinishTime);
    }
}


void CoreSystemAccess::finalize(double mStartTime, double mFinishTime)
{
    mpCoreComponentSystem->finalize(mStartTime, mFinishTime);
}

QString CoreSystemAccess::createComponent(QString type, QString name)
{
    //qDebug() << "createComponent: " << "type: " << type << " desired name:  " << name << " in system: " << this->getRootSystemName();
    Component *pCoreComponent = HopsanEssentials::getInstance()->CreateComponent(type.toStdString());
    if (pCoreComponent != 0)
    {
        mpCoreComponentSystem->addComponent(pCoreComponent);
        if (!name.isEmpty())
        {
            pCoreComponent->setName(name.toStdString());
        }
        //qDebug() << "createComponent: name after add: " << QString::fromStdString(pCoreComponent->getName()) << " added to: " << QString::fromStdString(mpCoreComponentSystem->getName());
        return QString::fromStdString(pCoreComponent->getName());
    }
    else
    {
        qDebug() << "failed to create component of type: " << type << " maybe it is not registered in the core";
        return QString();
    }
}

QString CoreSystemAccess::createSubSystem(QString name)
{
    ComponentSystem *pTempComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    if (!name.isEmpty())
    {
        pTempComponentSystem->setName(name.toStdString());
    }
    return QString::fromStdString(pTempComponentSystem->getName());
}

QVector<QString> CoreSystemAccess::getParameterNames(QString componentName)
{
    QVector<QString> names;
    //*****Core Interaction*****
    //First check if subcomponent can be found
    //! @todo this is temporary hack to avoid trying to find ourselfs when access through GUIsystem
    if (mpCoreComponentSystem->haveSubComponent(componentName.toStdString()))
    {
        vector<string> core_names = mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameterNames();
        vector<string>::iterator nit;
        //Copy and cast to qt datatypes
        for ( nit=core_names.begin(); nit!=core_names.end(); ++nit)
        {
            names.push_back(QString::fromStdString(*nit));
        }
        //**************************
    }

    return names;
}

QString CoreSystemAccess::getParameterUnit(QString componentName, QString parameterName)
{
    return QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameterUnit(parameterName.toStdString()));
}

QString CoreSystemAccess::getParameterDescription(QString componentName, QString parameterName)
{
    return QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameterDescription(parameterName.toStdString()));
}

double CoreSystemAccess::getParameterValue(QString componentName, QString parameterName)
{
    return mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameterValue(parameterName.toStdString());
}

QString CoreSystemAccess::getParameterValueTxt(QString componentName, QString parameterName)
{
    return QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameterValueTxt(parameterName.toStdString()));
}

void CoreSystemAccess::deleteSystemPort(QString portname)
{
    mpCoreComponentSystem->deleteSystemPort(portname.toStdString());
}

QString CoreSystemAccess::addSystemPort(QString portname)
{
    //qDebug() << "add system port: " << portname;
    return QString::fromStdString(mpCoreComponentSystem->addSystemPort(portname.toStdString())->getPortName());
}

QString CoreSystemAccess::renameSystemPort(QString oldname, QString newname)
{
    return QString::fromStdString(mpCoreComponentSystem->renameSystemPort(oldname.toStdString(), newname.toStdString()));
}

QString CoreSystemAccess::getPlotDataUnit(const QString compname, const QString portname, const QString dataname)
{
    std::string dummy, unit;
    Port* pPort = this->getPortPtr(compname, portname);
    if(pPort)
    {
        int idx = pPort->getNodeDataIdFromName(dataname.toStdString());
        if(idx >= 0)
            pPort->getNodeDataNameAndUnit(idx,dummy,unit);
    }
    return QString::fromStdString(unit);
}

//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits)
{
    vector<string> corenames, coreunits;
    rNames.clear();
    rUnits.clear();

    Port* pPort = this->getPortPtr(compname, portname);
    if (pPort)
    {
        pPort->getNodeDataNamesAndUnits(corenames, coreunits);
        //Copy into QT datatype vector (assumes bothe received vectors same length (should always be same)
        for (size_t i=0; i<corenames.size(); ++i)
        {
            rNames.push_back(QString::fromStdString(corenames[i]));
            rUnits.push_back(QString::fromStdString(coreunits[i]));
        }
    }
}

void CoreSystemAccess::getPlotData(const QString compname, const QString portname, const QString dataname, QVector<double> &rData)
{
    int dataId = -1;
    Port* pPort = this->getPortPtr(compname, portname);
    if (pPort)
        if(pPort->isConnected())
        {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if (dataId >= 0)
        {
            vector< vector<double> > *pData = pPort->getDataVectorPtr();

            //Ok lets copy all of the data to a QT vector
            rData.clear();
            rData.resize(pData->size()); //Allocaate memory
            for (size_t i=0; i<pData->size(); ++i) //Denna loop ar inte klok
            {
                rData[i] = pData->at(i).at(dataId);
            }
        }
    }
}


bool CoreSystemAccess::getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData)
{
    int dataId = -1;
    Port* pPort = this->getPortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if (dataId >= 0)
        {
            vector< vector<double> > *pData = pPort->getDataVectorPtr();
            rData = pData->back().at(dataId);
            return 1;
        }
    }
    return 0;
}


bool CoreSystemAccess::isPortConnected(QString componentName, QString portName)
{
    Port* pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        return pPort->isConnected();
    }
    else
    {
        return false;
    }
}

//! @brief Helpfunction that tries to fetch a port pointer
//! @param [in] componentName The name of the component to which the port belongs
//! @param [in] portName The name of the port
//! @returns A pointer to the port or a 0 ptr if component or port not found
hopsan::Port* CoreSystemAccess::getPortPtr(QString componentName, QString portName)
{
    //We must use getcomponent here if we want to be able to find root system ptr
    //! @todo see if we can reduce the number f public get functions one, the one which only searches subcomponents make function in core to solve the other access type like bellow
    Component* pComp = mpCoreComponentSystem->getComponent(componentName.toStdString());
    if (pComp)
    {
        return pComp->getPort(portName.toStdString());
    }
    return 0;
}



void CoreSystemAccess::setSystemParameter(QString name, double value)
{
    mpCoreComponentSystem->getSystemParameters().add(name.toStdString(), value);
}


double CoreSystemAccess::getSystemParameter(QString name)
{
    double value;
    mpCoreComponentSystem->getSystemParameters().getValue(name.toStdString(), value);
    return value;
}


bool CoreSystemAccess::hasSystemParameter(QString name)
{
    double dummy;
    return mpCoreComponentSystem->getSystemParameters().getValue(name.toStdString(), dummy);
}


void CoreSystemAccess::removeSystemParameter(QString name)
{
    mpCoreComponentSystem->getSystemParameters().erase(name.toStdString());
}


QMap<std::string, double> CoreSystemAccess::getSystemParametersMap()
{
    return QMap<std::string, double>(mpCoreComponentSystem->getSystemParameters().getSystemParameterMap());
}
