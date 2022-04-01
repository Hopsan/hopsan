#include "hopsangeneratorgui/hopsangeneratorgui.h"

#include <QFileInfo>
#include <QDir>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLibrary>
#include <QPointer>
#include <QEventLoop>
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <future>


#ifdef _WIN32
#define SHAREDLIB_PREFIX ""
#else
#define SHAREDLIB_PREFIX "lib"
#endif

#ifdef HOPSAN_BUILD_TYPE_DEBUG
#define DEBUG_SUFFIX "_d"
#else
#define DEBUG_SUFFIX ""
#endif

namespace {

class WidgetLock;
class HopsanGeneratorWidget;

class CApiStringList
{
public:
    CApiStringList(const QStringList& list)
    {
        reserve(list.size());
        for(const auto& str : list)
        {
            append(str);
        }
    }

    const char* const* data() const
    {
        return charptr_array.data();
    }

    size_t size() const
    {
        return strings.size();
    }

private:
    void reserve(const int size)
    {
        strings.reserve(size);
        charptr_array.reserve(size);
    }

    void append(const QString& str)
    {
        strings.emplace_back(std::move(str.toStdString()));
        charptr_array.push_back(strings.back().c_str());
    }

    std::vector<std::string> strings;
    std::vector<const char*> charptr_array;
};

class HopsanGeneratorWidget : public QDialog
{
    friend class WidgetLock;
public:
    HopsanGeneratorWidget(QWidget *parent, bool autoCloseOnSuccess=false)
        : QDialog(parent, Qt::Window), mAutoCloseOnSuccess(autoCloseOnSuccess)
    {
        setWindowModality(Qt::ApplicationModal);
        setMinimumSize(640, 480);
        setWindowTitle("HopsanGenerator");
        setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        setAttribute(Qt::WA_DeleteOnClose);

        auto pLayout = new QVBoxLayout(this);

        QFont monoFont = QFont("Monospace", 10, 50);
        monoFont.setStyleHint(QFont::TypeWriter);

        mpTextEdit = new QTextEdit(this);
        mpTextEdit->setReadOnly(true);
        mpTextEdit->setFont(monoFont);
        pLayout->addWidget(mpTextEdit);

        mpCloseButton = new QPushButton("In progress...", this);
        mpCloseButton->setFixedWidth(200); //! @todo not hard coded width
        mpCloseButton->setAutoDefault(true);
        mpCloseButton->setDefault(true);
        mpCloseButton->setDisabled(true);
        connect(mpCloseButton, SIGNAL(clicked()), this, SLOT(close()));
        pLayout->addWidget(mpCloseButton);
        pLayout->setAlignment(mpCloseButton, Qt::AlignCenter);

        show();
    }

    void printMessage(const QString &msg, const char type)
    {
        QColor color;
        if (type == 'W') {
            color = "Orange";
        } else if (type == 'E') {
            color = "Red";
        } else {
            color = "Black";
        }
        printMessage(msg, color);
    }

    void printMessage(const QString &msg, const QColor &color)
    {
        mpTextEdit->setTextColor(color);
        mpTextEdit->append(msg);
    }

private:
    void finalize(bool didSucceed)
    {
        mpCloseButton->setText("Close");
        mpCloseButton->setEnabled(true);
        if (mAutoCloseOnSuccess && didSucceed) {
            close();
        }
    }

    bool mAutoCloseOnSuccess;
    QTextEdit* mpTextEdit;
    QPushButton* mpCloseButton;
};

// Thread safe message forwarder helper
class MessageForwarder {
public:
    MessageForwarder(HopsanGeneratorWidget* pWidget) : mpWidget(pWidget) {}

    void addMessage(const char* msg, const char type)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mMessages.emplace_back(msg, type);
    }

    void transfereMessage()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        for (const auto& message : mMessages)
        {
            mpWidget->printMessage(message.message.c_str(), message.type);
        }
        mMessages.clear();
    }

private:
    struct Message
    {
        Message(const char* m, char t) : message(m), type(t) {}
        std::string message;
        char type;
    };

    std::mutex mMutex;
    std::vector<Message> mMessages;
    HopsanGeneratorWidget* mpWidget;
};

using MessageHandler_t = void(const char* msg, const char type, void*);
void messageHandler(const char* msg, const char type, void* pVoidForwarder)
{
    auto pForwarder = static_cast<MessageForwarder*>(pVoidForwarder);
    pForwarder->addMessage(msg, type);
}

class WidgetLock {

public:
    explicit WidgetLock(QWidget* parent, bool autoClose)
    {
        mpWidget = new ::HopsanGeneratorWidget(parent, autoClose);
    }

    ~WidgetLock()
    {
        mpWidget->finalize(mDidSucceed);
    }

    WidgetLock(const WidgetLock& other) = delete;
    void operator=(const WidgetLock& other) = delete;


    void setDidSucceed(bool tf)
    {
        mDidSucceed = tf;
    }

    ::HopsanGeneratorWidget* widget()
    {
        return mpWidget.data();
    }

private:
    bool mDidSucceed=false;
    QPointer<::HopsanGeneratorWidget> mpWidget;
};

} // End anon namespace

struct HopsanGeneratorGUI::PrivateImpl
{
    QSharedPointer<WidgetLock> createNewWidget()
    {
        // Create a new widget, Note! it shall delete itself on close
        auto widgetlock = QSharedPointer<WidgetLock>(new WidgetLock(mpWidgetParent, mAutoCloseWidgets));
        mpCurrentWidget = widgetlock->widget();
        return widgetlock;
    }

    void printMessage(const QString &msg, const char type)
    {
        if (mpCurrentWidget) {
            mpCurrentWidget->printMessage(msg, type);
        }
    }

    template<typename FunctionT, typename ...FunctionArgs>
    bool call(MessageForwarder& messageForwarder, const char* functionName, FunctionArgs... args)
    {
        auto func = (FunctionT*) mGeneratorLibrary.resolve(functionName);
        if (func)
        {
            std::future<bool> asyncFuncCall = std::async(std::launch::async, func, args...);
            QEventLoop localEvenLoop;
            while(asyncFuncCall.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready)
            {
                messageForwarder.transfereMessage();
                localEvenLoop.processEvents();
            }
            messageForwarder.transfereMessage();
            localEvenLoop.processEvents();
            return asyncFuncCall.get();
        }
        else
        {
            const auto msg = QString("Could not load: %1").arg(functionName);
            printMessage(msg, 'E');
            return false;
        }
    }

    bool mAutoCloseWidgets = false;
    std::string hopsanRoot;
    std::string compilerPath;
    QLibrary mGeneratorLibrary;
    QPointer<QWidget> mpWidgetParent;
    QPointer<HopsanGeneratorWidget> mpCurrentWidget;
};

HopsanGeneratorGUI::HopsanGeneratorGUI(const QString& hopsanInstallPath, QWidget* pWidgetParent)
    : mPrivates(new  HopsanGeneratorGUI::PrivateImpl())
{
    mPrivates->hopsanRoot = QDir::cleanPath(hopsanInstallPath).toStdString();
    mPrivates->mpWidgetParent = pWidgetParent;
}

HopsanGeneratorGUI::~HopsanGeneratorGUI()
{
    if (isGeneratorLibraryLoaded()) {
        mPrivates->mGeneratorLibrary.unload();
    }
}

bool HopsanGeneratorGUI::isGeneratorLibraryLoaded() const
{
    return mPrivates->mGeneratorLibrary.isLoaded();
}

bool HopsanGeneratorGUI::loadGeneratorLibrary()
{
    if (!isGeneratorLibraryLoaded())
    {
        constexpr auto generatorLibName = SHAREDLIB_PREFIX "hopsangenerator" DEBUG_SUFFIX;

        mPrivates->mGeneratorLibrary.setFileName(generatorLibName);
        QString errorString1, errorString2, errorString3;
        bool loadok1 = false, loadok2 = false, loadok3 = false;
        loadok1 = mPrivates->mGeneratorLibrary.load();
        if (!loadok1)
        {
            errorString1 = mPrivates->mGeneratorLibrary.errorString();
            // Try again with expected absolute path in case current dir is not in the search path
            // (depends on distribution) or LD_LIBRARY_PATH, but if that fail, show the first error message
            const auto absGeneratorLibPathBin = QString("%1/bin/%2").arg(mPrivates->hopsanRoot.c_str()).arg(generatorLibName);
            mPrivates->mGeneratorLibrary.setFileName(absGeneratorLibPathBin);
            loadok2 = mPrivates->mGeneratorLibrary.load();
            if (!loadok2)
            {
                errorString2 = mPrivates->mGeneratorLibrary.errorString();
                // Attempt to locate in lib (Linux cmake build)
                const auto absGeneratorLibPathLib = QString("%1/lib/%2").arg(mPrivates->hopsanRoot.c_str()).arg(generatorLibName);
                mPrivates->mGeneratorLibrary.setFileName(absGeneratorLibPathLib);
                loadok3 = mPrivates->mGeneratorLibrary.load();
                if (!loadok3) {
                    errorString3 = mPrivates->mGeneratorLibrary.errorString();
                }
            }
        }

        if (loadok1 || loadok2 || loadok3)
        {
            printMessage(QString("Loaded %1").arg(generatorLibName));
            return true;
        }
        else
        {
            printErrorMessage(errorString1);
            printErrorMessage(errorString2);
            printErrorMessage(errorString3);
            return false;
        }
    }
    return true;
}

void HopsanGeneratorGUI::setCompilerPath(const QString& compilerPath)
{
    mPrivates->compilerPath = compilerPath.toStdString();
}

void HopsanGeneratorGUI::setAutoCloseWidgetsOnSuccess(bool doAutoClose)
{
    mPrivates->mAutoCloseWidgets = doAutoClose;
}

bool HopsanGeneratorGUI::generateFromModelica(const QString& modelicaFile, const CompileT compile)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callModelicaGenerator";
    const auto mofile = modelicaFile.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);
    MessageForwarder forwarder(lw->widget());

    using ModelicaImportFunction_t = bool(const char*, const char*, MessageHandler_t, void*, bool, const char*);
    bool didOK = mPrivates->call<ModelicaImportFunction_t>(forwarder, functionName, mofile.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(&forwarder), doCompile, hopsanRoot.c_str());
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::generateToFmu(const QString& outputPath, hopsan::ComponentSystem* pSystem, const QStringList& externalLibraries,
                                       const FmuVersionT version, const TargetArchitectureT architecture)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callFmuExportGenerator";
    const auto outpath = outputPath.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    int arch =  (architecture == TargetArchitectureT::x64) ? 64 : 32;
    CApiStringList extLibs(externalLibraries);
    MessageForwarder forwarder(lw->widget());

    using FmuExportFunction_t = bool(const char*, hopsan::ComponentSystem*, const char* const*, const int, const char*, const char*, int, int, MessageHandler_t, void*);
    bool didOK = mPrivates->call<FmuExportFunction_t>(forwarder, functionName, outpath.c_str(), pSystem, extLibs.data(), extLibs.size(), hopsanRoot.c_str(), compilerPath.c_str(), static_cast<int>(version), arch, &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::generateToSimulink(const QString& outputPath, const QString& modelPath, hopsan::ComponentSystem *pSystem,
                                            const QStringList& externalLibraries, const UsePortlablesT portLabels)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callSimulinkExportGenerator";
    const auto outpath = outputPath.toStdString();
    const auto modelpath = modelPath.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    CApiStringList extLibs(externalLibraries);
    bool disablePortLabels = (portLabels == UsePortlablesT::DisablePortLables);
    MessageForwarder forwarder(lw->widget());

    using SimulinkExportFunction_t = bool(const char*, const char*, hopsan::ComponentSystem*, const char* const*, const int, bool, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<SimulinkExportFunction_t>(forwarder, functionName, outpath.c_str(), modelpath.c_str(), pSystem, extLibs.data(), extLibs.size(), disablePortLabels, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::generateToLabViewSIT(const QString& outputPath, hopsan::ComponentSystem *pSystem)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callLabViewSITGenerator";
    const auto outpath = outputPath.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    MessageForwarder forwarder(lw->widget());

    using LabViewSITExportFunction_t = bool(const char*, hopsan::ComponentSystem*, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<LabViewSITExportFunction_t>(forwarder, functionName, outpath.c_str(), pSystem, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}

bool HopsanGeneratorGUI::generateToExe(const QString &outputPath, hopsan::ComponentSystem *pSystem, const QStringList &externalLibraries, HopsanGeneratorGUI::TargetArchitectureT architecture)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callExeExportGenerator";
    const auto outpath = outputPath.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    int arch =  (architecture == TargetArchitectureT::x64) ? 64 : 32;
    CApiStringList extLibs(externalLibraries);
    MessageForwarder forwarder(lw->widget());

    using ExeExportFunction_t = bool(const char*, hopsan::ComponentSystem*, const char* const*, const int, const char*, const char*, int, MessageHandler_t, void*);
    bool didOK = mPrivates->call<ExeExportFunction_t>(forwarder, functionName, outpath.c_str(), pSystem, extLibs.data(), extLibs.size(), hopsanRoot.c_str(), compilerPath.c_str(), arch, &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::generateFromCpp(const QString& hppFile, const CompileT compile)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callCppGenerator";
    const auto hppfile = hppFile.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);
    MessageForwarder forwarder(lw->widget());


    using CppGeneratorFunction_t = bool(const char*, const char*, bool, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<CppGeneratorFunction_t>(forwarder, functionName, hppfile.c_str(), compilerPath.c_str(), doCompile, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::generateLibrary(const QString& outputPath, const QStringList& hppFiles)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callLibraryGenerator";
    const auto outpath = outputPath.toStdString();
    CApiStringList hppfiles(hppFiles);
    MessageForwarder forwarder(lw->widget());

    using GenerateLibraryFunction_t = bool(const char*, const char* const*, size_t, MessageHandler_t, void*);
    bool didOK = mPrivates->call<GenerateLibraryFunction_t>(forwarder, functionName, outpath.c_str(), hppfiles.data(), hppfiles.size(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::compileComponentLibrary(const QString& libPath, const QString& extraCFlags,
                                                 const QString& extraLFlags)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callComponentLibraryCompiler";
    const auto libpath = libPath.toStdString();
    const auto cflags  = extraCFlags.toStdString();
    const auto lflags  = extraLFlags.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    MessageForwarder forwarder(lw->widget());

    using CompileLibraryFunction_t = bool(const char*, const char*, const char*, const char*, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<CompileLibraryFunction_t>(forwarder, functionName, libpath.c_str(), cflags.c_str(), lflags.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(didOK);
    return didOK;
}

bool HopsanGeneratorGUI::checkComponentLibrary(const QString& libraryXMLPath)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callCheckComponentLibrary";
    const auto libpath = libraryXMLPath.toStdString();
    MessageForwarder forwarder(lw->widget());

    using CheckLibraryFunction_t = bool(const char*, MessageHandler_t, void*);
    bool checkOK = mPrivates->call<CheckLibraryFunction_t>(forwarder, functionName, libpath.c_str(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(checkOK);
    return checkOK;
}


bool HopsanGeneratorGUI::addComponentToLibrary(const QString &libraryXmlPath, const QString &cafPath)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callAddExistingComponentToLibrary";
    const auto libpath = libraryXmlPath.toStdString();
    MessageForwarder forwarder(lw->widget());
    const auto xmlpath = libraryXmlPath.toStdString();
    const auto cafpath = cafPath.toStdString();

    using AddExistingComponentToLibraryFunction_t = bool(const char*, const char*, MessageHandler_t, void*);
    bool checkOK = mPrivates->call<AddExistingComponentToLibraryFunction_t>(forwarder, functionName, xmlpath.c_str(), cafpath.c_str(), &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(checkOK);
    return checkOK;
}


bool HopsanGeneratorGUI::addComponentToLibrary(const QString &libraryXmlPath, const QString targetPath, const QString &typeName, const QString &displayName, const QString &cqsType, QString &transformStr,
                                               const QStringList &constantNames, const QStringList &constantDescriptions, const QStringList &constantUnits, const QStringList &constantInits,
                                               const QStringList &inputNames, const QStringList &inputDescriptions, const QStringList &inputUnits, const QStringList &inputInits,
                                               const QStringList &outputNames, const QStringList &outputDescriptions, const QStringList &outputUnits, const QStringList &outputInits,
                                               const QStringList &portNames, const QStringList &portDescriptions, const QStringList &portTypes, const QList<bool> &portsRequired, bool modelica)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callAddComponentToLibrary";
    const auto targetpath = targetPath.toStdString();
    MessageForwarder forwarder(lw->widget());
    const auto xmlpath = libraryXmlPath.toStdString();
    const auto typenamestr = typeName.toStdString();
    const auto displayname = displayName.toStdString();
    const auto cqstype = cqsType.toStdString();
    const auto transform = transformStr.toStdString();
    CApiStringList constantnames(constantNames);
    CApiStringList constantdescriptions(constantDescriptions);
    CApiStringList constantunits(constantUnits);
    CApiStringList constantinits(constantInits);
    CApiStringList inputnames(inputNames);
    CApiStringList inputdescriptions(inputDescriptions);
    CApiStringList inputunits(inputUnits);
    CApiStringList inputinits(inputInits);
    CApiStringList outputnames(outputNames);
    CApiStringList outputdescriptions(outputDescriptions);
    CApiStringList outputunits(outputUnits);
    CApiStringList outputinits(outputInits);
    CApiStringList portnames(portNames);
    CApiStringList portdescriptions(portDescriptions);
    CApiStringList porttypes(portTypes);
    std::vector<int> portsrequired;
    for(const bool req : portsRequired.toVector()) {
        portsrequired.push_back(req);
    }

    using AddComponentToLibraryFunction_t = bool(const char*, const char*, const char*, const char*, const char*, const char*, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const char* const*, size_t, const int[], size_t, bool, MessageHandler_t, void*);
    bool checkOK = mPrivates->call<AddComponentToLibraryFunction_t>(forwarder, functionName, xmlpath.c_str(), targetpath.c_str(), typenamestr.c_str(), displayname.c_str(), cqstype.c_str(), transform.c_str(),
                                                                    constantnames.data(), constantnames.size(), constantdescriptions.data(), constantdescriptions.size(), constantunits.data(), constantunits.size(), constantinits.data(), constantinits.size(),
                                                                    inputnames.data(), inputnames.size(), inputdescriptions.data(), inputdescriptions.size(), inputunits.data(), inputunits.size(), inputinits.data(), inputinits.size(),
                                                                    outputnames.data(), outputnames.size(), outputdescriptions.data(), outputdescriptions.size(), outputunits.data(), outputunits.size(), outputinits.data(), outputinits.size(),
                                                                    portnames.data(), portnames.size(), portdescriptions.data(), portdescriptions.size(), porttypes.data(), porttypes.size(), portsrequired.data(), portsrequired.size(),
                                                                    modelica, &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(checkOK);
    return checkOK;
}

bool HopsanGeneratorGUI::removeComponentFromLibrary(const QString &libraryXmlPath, const QString &cafPath, const QString &hppPath, bool deleteFiles)
{

    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callRemoveComponentFromLibrary";
    const auto libpath = libraryXmlPath.toStdString();
    MessageForwarder forwarder(lw->widget());
    const auto xmlpath = libraryXmlPath.toStdString();
    const auto cafpath = cafPath.toStdString();
    const auto hpppath = hppPath.toStdString();

    using RemoveComponentFromLibraryFunction_t = bool(const char*, const char*, const char*, bool, MessageHandler_t, void*);
    bool checkOK = mPrivates->call<RemoveComponentFromLibraryFunction_t>(forwarder, functionName, xmlpath.c_str(), cafpath.c_str(), hpppath.c_str(), deleteFiles, &messageHandler, static_cast<void*>(&forwarder));
    lw->setDidSucceed(checkOK);
    return checkOK;
}

void HopsanGeneratorGUI::printMessage(const QString &msg, const char type)
{
    mPrivates->printMessage(msg, type);
}

void HopsanGeneratorGUI::printErrorMessage(const QString &msg)
{
    printMessage(msg, 'E');
}

void HopsanGeneratorGUI::printWarningMessage(const QString& msg)
{
    printMessage(msg, 'W');
}
