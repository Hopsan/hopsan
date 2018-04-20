#include "hopsangeneratorgui.h"

#include <QFileInfo>
#include <QDir>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLibrary>
#include <QPointer>

#ifdef _WIN32
#define SHAREDLIB_PREFIX ""
#else
#define SHAREDLIB_PREFIX "lib"
#endif

#ifdef DEBUGCOMPILING
#define DEBUG_SUFFIX "_d"
#else
#define DEBUG_SUFFIX ""
#endif

class WidgetLock;

namespace {

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

class HopsanGeneratorWidget : public QWidget
{
    friend WidgetLock;
public:
    HopsanGeneratorWidget(QWidget *parent, bool autoCloseOnSuccess=false)
        : QWidget(parent, Qt::Window), mAutoCloseOnSuccess(autoCloseOnSuccess)
    {
        setWindowModality(Qt::ApplicationModal);
        setMinimumSize(640, 480);
        setWindowTitle("HopsanGenerator");
        setAttribute(Qt::WA_DeleteOnClose);

        auto pLayout = new QVBoxLayout(this);

        QFont monoFont = QFont("Monospace", 10, 50);
        monoFont.setStyleHint(QFont::TypeWriter);

        mpTextEdit = new QTextEdit(this);
        mpTextEdit->setReadOnly(true);
        mpTextEdit->setFont(monoFont);
        pLayout->addWidget(mpTextEdit);

        mpCloseButton = new QPushButton("Close", this);
        mpCloseButton->setFixedWidth(200); //! @todo not hard coded width
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
//#ifdef _WIN32
//    Sleep(10);
//#else
//    usleep(10000);
//#endif
    }

private:
    void finalize(bool didSucceed)
    {
        if (mAutoCloseOnSuccess && didSucceed) {
            close();
        } else {
            mpCloseButton->setEnabled(true);
        }
    }

    bool mAutoCloseOnSuccess;
    QTextEdit* mpTextEdit;
    QPushButton* mpCloseButton;
};


using MessageHandler_t = void(const char* msg, const char type, void*);
void messageHandler(const char* msg, const char type, void* pObj)
{
    auto pWidget = static_cast<HopsanGeneratorWidget*>(pObj);
    pWidget->printMessage(QString(msg), type);
}
}



class WidgetLock {

public:
    WidgetLock(QWidget* parent)
    {
        mpWidget = new ::HopsanGeneratorWidget(parent);
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

class HopsanGeneratorGUIPrivateImpl
{
public:
    QSharedPointer<WidgetLock> createNewWidget()
    {
        // Create a new widget, Note! it shall delete itself on close
        auto widgetlock = QSharedPointer<WidgetLock>(new WidgetLock(mpWidgetParent));
        mpCurrentWidget = widgetlock->widget();
        return widgetlock;
    }

    std::string hopsanRoot;
    std::string compilerPath;
    QLibrary mGeneratorLibrary;
    QPointer<QWidget> mpWidgetParent;
    QPointer<HopsanGeneratorWidget> mpCurrentWidget;
};

HopsanGeneratorGUI::HopsanGeneratorGUI(const QString& hopsanInstallPath, QWidget* pWidgetParent)
    : mPrivates(new  HopsanGeneratorGUIPrivateImpl())
{
    mPrivates->hopsanRoot = hopsanInstallPath.toStdString();
    mPrivates->mpWidgetParent = pWidgetParent;
}

HopsanGeneratorGUI::~HopsanGeneratorGUI()
{
    // Virtual destructor needed so that PIMPL works
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
        bool loadok = mPrivates->mGeneratorLibrary.load();
        if (loadok)
        {
            printMessage(QString("Loaded %1").arg(generatorLibName));
        }
        else
        {
            printErrorMessage(mPrivates->mGeneratorLibrary.errorString());
        }
        return loadok;
    }
    return true;
}

void HopsanGeneratorGUI::setCompilerPath(const QString& compilerPath)
{
    mPrivates->compilerPath = compilerPath.toStdString();
}

bool HopsanGeneratorGUI::generateFromModelica(const QString& modelicaFile, const int solver, const CompileT compile)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

//extern "C" HOPSANGENERATOR_DLLAPI void callModelicaGenerator(const char* outputPath, const char* compilerPath, bool quiet=false, int solver=0, bool compile=false, const char* hopsanInstallPath="")

//        pHandler->callModelicaGenerator(hPath, hGccPath, showDialog, solver, compile, hIncludePath, hBinPath);

    constexpr auto functionName = "callModelicaGenerator";
    auto mofile = modelicaFile.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);

    using ModelicaImportFunction_t = void(const char*, const char*, MessageHandler_t, void*, int, bool, const char*);
    auto func = (ModelicaImportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        func(mofile.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(lw->widget()), solver, doCompile, hopsanRoot.c_str());
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}

bool HopsanGeneratorGUI::generateFromFmu(const QString& fmuFilePath, const QString& destination)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    QFileInfo fmuFileInfo(fmuFilePath);
    QString fmuFileName = fmuFileInfo.fileName();
    fmuFileName.chop(4);
    QDir importDestination(destination);
    bool generatedOK = false;
    if(importDestination.exists())
    {
        printErrorMessage(QString("Destination already exist %1").arg(destination));
    }
    else
    {
        constexpr auto functionName = "callFmuImportGenerator";
        auto fmu = fmuFilePath.toStdString();
        auto dst = destination.toStdString();
        auto& hopsanRoot = mPrivates->hopsanRoot;
        auto& compilerPath = mPrivates->compilerPath;

        using FmuImportFunction_t = void(const char*, const char*, const char*, const char*, MessageHandler_t, void*);
        auto func = (FmuImportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
        if (func)
        {
            func(fmu.c_str(), dst.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
            generatedOK = true;
        }
        else
        {
            printErrorMessage(QString("Could not load: %1").arg(functionName));
        }
    }

    return generatedOK;
}

bool HopsanGeneratorGUI::generateToFmu(const QString& outputPath, hopsan::ComponentSystem* pSystem,
                                       const FmuVersionT version, const TargetArchitectureT architecture)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callFmuExportGenerator";
    auto outpath = outputPath.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;
    int arch =  (architecture == TargetArchitectureT::x64) ? 64 : 32;

//callFmuExportGenerator(const char* outputPath, hopsan::ComponentSystem *pSystem, const char* hopsanInstallPath, const char* compilerPath, int version=2, int architecture=64, bool quiet=false)
    using FmuExportFunction_t = void(const char*, hopsan::ComponentSystem*, const char*, const char*, int, int, MessageHandler_t, void*);
    auto func = (FmuExportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        func(outpath.c_str(), pSystem, hopsanRoot.c_str(), compilerPath.c_str(), static_cast<int>(version), arch, &messageHandler, static_cast<void*>(lw->widget()));
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}


bool HopsanGeneratorGUI::generateToSimulink(const QString& outputPath, const QString& modelPath,
                                            hopsan::ComponentSystem *pSystem, const UsePortlablesT portLabels)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

//    extern "C" HOPSANGENERATOR_DLLAPI void callSimulinkExportGenerator(const char* outputPath, const char* modelFile, hopsan::ComponentSystem *pSystem, bool disablePortLabels, const char* hopsanInstallPath, bool quiet=false)

//                pHandler->callSimulinkExportGenerator(path.toStdString().c_str(), pSystem->getModelFileInfo().fileName().toStdString().c_str(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), disablePortLabels, gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);

    constexpr auto functionName = "callSimulinkExportGenerator";
    auto outpath = outputPath.toStdString();
    auto modelpath = modelPath.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    bool disablePortLabels = (portLabels == UsePortlablesT::DisablePortLables);

    using SimulinkExportFunction_t = void(const char*, const char*,  hopsan::ComponentSystem*, bool, const char*, MessageHandler_t, void*);
    auto func = (SimulinkExportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        func(outpath.c_str(), modelpath.c_str(), pSystem, disablePortLabels, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}


bool HopsanGeneratorGUI::generateToLabViewSIT(const QString& outputPath, hopsan::ComponentSystem *pSystem)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();
//    extern "C" HOPSANGENERATOR_DLLAPI void callLabViewSITGenerator(const char* outputPath, hopsan::ComponentSystem *pSystem, const char* hopsanInstallPath, bool quiet=false)

    constexpr auto functionName = "callLabViewSITGenerator";
    auto outpath = outputPath.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;

    using LabViewSITExportFunction_t = void(const char*, hopsan::ComponentSystem*, const char*, MessageHandler_t, void*);
    auto func = (LabViewSITExportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        func(outpath.c_str(), pSystem, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}


bool HopsanGeneratorGUI::generateFromCpp(const QString& hppFile, const CompileT compile)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

//    extern "C" HOPSANGENERATOR_DLLAPI? void callCppGenerator(const char* hppPath, const char* compilerPath, bool compile=false, const char* hopsanInstallPath="")
//        pHandler->callCppGenerator(hHppFile, hGccPath, compile, hIncludePath, hBinPath);

    auto hppfile = hppFile.toStdString();
    constexpr auto functionName = "callCppGenerator";
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);


    using CppGeneratorFunction_t = void(const char*, const char*, bool, const char*, MessageHandler_t, void*);
    auto func = (CppGeneratorFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        func(hppfile.c_str(), compilerPath.c_str(), doCompile, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}


bool HopsanGeneratorGUI::generateLibrary(const QString& outputPath, const QStringList& hppFiles)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

//    extern "C" HOPSANGENERATOR_DLLAPI void callLibraryGenerator(const char* outputPath, std::vector<hopsan::HString> hppFiles, bool quiet=false)

    constexpr auto functionName = "callLibraryGenerator";
    auto outpath = outputPath.toStdString();

    using GenerateLibraryFunction_t = void(const char*, const char* const*, size_t, MessageHandler_t, void*);
    auto func = (GenerateLibraryFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        CApiStringList hppfiles(hppFiles);
        func(outpath.c_str(), hppfiles.data(), hppfiles.size(), &messageHandler, static_cast<void*>(lw->widget()));
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}

bool HopsanGeneratorGUI::compileComponentLibrary(const QString& libPath, const QString& extraCFlags,
                                                 const QString& extraLFlags)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    //   extern "C" HOPSANGENERATOR_DLLAPI void callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, bool quiet=false)
//        pHandler->callComponentLibraryCompiler(hLibPath, hExtraCFlags, hExtraLFlags, hIncludePath, hBinPath, hGccPath, showDialog);

    constexpr auto functionName = "callComponentLibraryCompiler";
    auto libpath = libPath.toStdString();
    auto cflags  = extraCFlags.toStdString();
    auto lflags  = extraLFlags.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;

    using CompileLibraryFunction_t = void(const char*, const char*, const char*, const char*, const char*, MessageHandler_t, void*);
    auto func = (CompileLibraryFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK = false;
    if (func)
    {
        func(libpath.c_str(), cflags.c_str(), lflags.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
    }

    return generatedOK;
}

void HopsanGeneratorGUI::printMessage(const QString &msg, const char type)
{
    if (mPrivates->mpCurrentWidget) {
        mPrivates->mpCurrentWidget->printMessage(msg, type);
    }
}

void HopsanGeneratorGUI::printErrorMessage(const QString &msg)
{
    printMessage(msg, 'E');
}

void HopsanGeneratorGUI::printWarningMessage(const QString& msg)
{
    printMessage(msg, 'W');
}
