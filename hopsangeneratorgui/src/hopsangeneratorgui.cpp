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

namespace {
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
}

class HopsanGeneratorWidget : public QWidget
{
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

    void finalize(bool didSucceed)
    {
        if (mAutoCloseOnSuccess && didSucceed) {
            close();
        } else {
            mpCloseButton->setEnabled(true);
        }
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
    bool mAutoCloseOnSuccess;
    QTextEdit* mpTextEdit;
    QPushButton* mpCloseButton;
};


class HopsanGeneratorGUIPrivateImpl
{
public:
    void createNewWidget(QWidget* parent)
    {
        // If we switch widget, enable close button on previous one
        if (mpCurrentWidget) {
            mpCurrentWidget->finalize(false);
        }
        // Create a new widget, Note! it shall delete itself on close
        mpCurrentWidget = new HopsanGeneratorWidget(parent);
    }

    std::string hopsanRoot;
    std::string compilerPath;
    QLibrary mGeneratorLibrary;
    QPointer<QWidget> mpWidgetParent=nullptr;
    QPointer<HopsanGeneratorWidget> mpCurrentWidget=nullptr;
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
    createWidget();
    loadGeneratorLibrary();

//extern "C" HOPSANGENERATOR_DLLAPI void callModelicaGenerator(const char* outputPath, const char* compilerPath, bool quiet=false, int solver=0, bool compile=false, const char* hopsanInstallPath="")

//        pHandler->callModelicaGenerator(hPath, hGccPath, showDialog, solver, compile, hIncludePath, hBinPath);

    constexpr auto functionName = "callModelicaGenerator";
    auto mofile = modelicaFile.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);

    using ModelicaImportFunction_t = void(const char*, const char*, bool quiet, int, bool, const char*);
    auto func = (ModelicaImportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        func(mofile.c_str(), compilerPath.c_str(), false, solver, doCompile, hopsanRoot.c_str());
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}

bool HopsanGeneratorGUI::generateFromFmu(const QString& fmuFilePath, const QString& destination)
{
    createWidget();
    loadGeneratorLibrary();

    QFileInfo fmuFileInfo(fmuFilePath);
    QString fmuFileName = fmuFileInfo.fileName();
    fmuFileName.chop(4);
    QDir importDestination(destination);
    bool generatedOK;
    if(importDestination.exists())
    {
        printErrorMessage(QString("Destination already exist %1").arg(destination));
        generatedOK =  false;
    }
    else
    {
        constexpr auto functionName = "callFmuImportGenerator";
        auto fmu = fmuFilePath.toStdString();
        auto dst = destination.toStdString();
        auto& hopsanRoot = mPrivates->hopsanRoot;
        auto& compilerPath = mPrivates->compilerPath;

        using FmuImportFunction_t = void(const char*, const char*, const char*, const char*, bool);
        auto func = (FmuImportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
        if (func)
        {
            func(fmu.c_str(), dst.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), false);
            generatedOK = true;
        }
        else
        {
            printErrorMessage(QString("Could not load: %1").arg(functionName));
            generatedOK = false;
        }
    }
    generateDone(generatedOK);
    return generatedOK;
}

bool HopsanGeneratorGUI::generateToFmu(const QString& outputPath, hopsan::ComponentSystem* pSystem,
                                       const FmuVersionT version, const TargetArchitectureT architecture)
{
    createWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callFmuExportGenerator";
    auto outpath = outputPath.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;
    int arch =  (architecture == TargetArchitectureT::x64) ? 64 : 32;

//callFmuExportGenerator(const char* outputPath, hopsan::ComponentSystem *pSystem, const char* hopsanInstallPath, const char* compilerPath, int version=2, int architecture=64, bool quiet=false)
    using FmuExportFunction_t = void(const char*, hopsan::ComponentSystem*, const char*, const char*, int, int, bool);
    auto func = (FmuExportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        func(outpath.c_str(), pSystem, hopsanRoot.c_str(), compilerPath.c_str(), static_cast<int>(version), arch, false);
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}


bool HopsanGeneratorGUI::generateToSimulink(const QString& outputPath, const QString& modelPath,
                                            hopsan::ComponentSystem *pSystem, const UsePortlablesT portLabels)
{
    createWidget();
    loadGeneratorLibrary();

//    extern "C" HOPSANGENERATOR_DLLAPI void callSimulinkExportGenerator(const char* outputPath, const char* modelFile, hopsan::ComponentSystem *pSystem, bool disablePortLabels, const char* hopsanInstallPath, bool quiet=false)

//                pHandler->callSimulinkExportGenerator(path.toStdString().c_str(), pSystem->getModelFileInfo().fileName().toStdString().c_str(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), disablePortLabels, gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);

    constexpr auto functionName = "callSimulinkExportGenerator";
    auto outpath = outputPath.toStdString();
    auto modelpath = modelPath.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    bool disablePortLabels = (portLabels == UsePortlablesT::DisablePortLables);

    using SimulinkExportFunction_t = void(const char*, const char*,  hopsan::ComponentSystem*, bool, const char*, bool);
    auto func = (SimulinkExportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        func(outpath.c_str(), modelpath.c_str(), pSystem, disablePortLabels, hopsanRoot.c_str(), false);
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}


bool HopsanGeneratorGUI::generateToLabViewSIT(const QString& outputPath, hopsan::ComponentSystem *pSystem)
{
    createWidget();
    loadGeneratorLibrary();
//    extern "C" HOPSANGENERATOR_DLLAPI void callLabViewSITGenerator(const char* outputPath, hopsan::ComponentSystem *pSystem, const char* hopsanInstallPath, bool quiet=false)

    constexpr auto functionName = "callLabViewSITGenerator";
    auto outpath = outputPath.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;

    using LabViewSITExportFunction_t = void(const char*, hopsan::ComponentSystem*, const char*, bool);
    auto func = (LabViewSITExportFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        func(outpath.c_str(), pSystem, hopsanRoot.c_str(), false);
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}


bool HopsanGeneratorGUI::generateFromCpp(const QString& hppFile, const CompileT compile)
{
    createWidget();
    loadGeneratorLibrary();

//    extern "C" HOPSANGENERATOR_DLLAPI? void callCppGenerator(const char* hppPath, const char* compilerPath, bool compile=false, const char* hopsanInstallPath="")
//        pHandler->callCppGenerator(hHppFile, hGccPath, compile, hIncludePath, hBinPath);

    auto hppfile = hppFile.toStdString();
    constexpr auto functionName = "callCppGenerator";
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);


    using CppGeneratorFunction_t = void(const char*, const char*, bool, const char*);
    auto func = (CppGeneratorFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        func(hppfile.c_str(), compilerPath.c_str(), doCompile, hopsanRoot.c_str());
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}


bool HopsanGeneratorGUI::generateLibrary(const QString& outputPath, const QStringList& hppFiles)
{
    createWidget();
    loadGeneratorLibrary();

//    extern "C" HOPSANGENERATOR_DLLAPI void callLibraryGenerator(const char* outputPath, std::vector<hopsan::HString> hppFiles, bool quiet=false)

    constexpr auto functionName = "callLibraryGenerator";
    auto outpath = outputPath.toStdString();

    using GenerateLibraryFunction_t = void(const char*, const char* const*, size_t,  bool);
    auto func = (GenerateLibraryFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        CApiStringList hppfiles(hppFiles);
        func(outpath.c_str(), hppfiles.data(), hppfiles.size(),  false);
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}

bool HopsanGeneratorGUI::compileComponentLibrary(const QString& libPath, const QString& extraCFlags,
                                                 const QString& extraLFlags)
{
    createWidget();
    loadGeneratorLibrary();

    //   extern "C" HOPSANGENERATOR_DLLAPI void callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, bool quiet=false)
//        pHandler->callComponentLibraryCompiler(hLibPath, hExtraCFlags, hExtraLFlags, hIncludePath, hBinPath, hGccPath, showDialog);

    constexpr auto functionName = "callComponentLibraryCompiler";
    auto libpath = libPath.toStdString();
    auto cflags  = extraCFlags.toStdString();
    auto lflags  = extraLFlags.toStdString();
    auto& hopsanRoot = mPrivates->hopsanRoot;
    auto& compilerPath = mPrivates->compilerPath;

    using CompileLibraryFunction_t = void(const char*, const char*, const char*, const char*, const char*, bool);
    auto func = (CompileLibraryFunction_t*) mPrivates->mGeneratorLibrary.resolve(functionName);
    bool generatedOK;
    if (func)
    {
        func(libpath.c_str(), cflags.c_str(), lflags.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), false);
        generatedOK = true;
    }
    else
    {
        printErrorMessage(QString("Could not load: %1").arg(functionName));
        generatedOK = false;
    }

    generateDone(generatedOK);
    return generatedOK;
}

void HopsanGeneratorGUI::printMessage(const char *message, const char type)
{
    printMessage(QString(message), type);
}

void HopsanGeneratorGUI::printMessage(const QString &msg, const QColor &color)
{
    if (mPrivates->mpCurrentWidget) {
        mPrivates->mpCurrentWidget->printMessage(msg, color);
    }
}

void HopsanGeneratorGUI::printMessage(const QString &msg, const char type)
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

void HopsanGeneratorGUI::printErrorMessage(const QString &msg)
{
    printMessage(msg, 'E');
}

void HopsanGeneratorGUI::printWarningMessage(const QString& msg)
{
    printMessage(msg, 'W');
}

void HopsanGeneratorGUI::generateDone(bool didSucceed)
{
    if (mPrivates->mpCurrentWidget) {
        mPrivates->mpCurrentWidget->finalize(didSucceed);
    }

}

void HopsanGeneratorGUI::createWidget()
{
    mPrivates->createNewWidget(mPrivates->mpWidgetParent);
}
