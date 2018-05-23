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

struct HopsanGeneratorGUI::PrivateImpl
{
    QSharedPointer<WidgetLock> createNewWidget()
    {
        // Create a new widget, Note! it shall delete itself on close
        auto widgetlock = QSharedPointer<WidgetLock>(new WidgetLock(mpWidgetParent));
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
    bool call(const char* functionName, FunctionArgs... args)
    {
        auto func = (FunctionT*) mGeneratorLibrary.resolve(functionName);
        if (func)
        {
            func(args...);
            return true;
        }
        else
        {
            const auto msg = QString("Could not load: %1").arg(functionName);
            printMessage(msg, 'E');
            return false;
        }
    }

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
        QString errorString1, errorString2;
        bool loadok1, loadok2;
        loadok1 = mPrivates->mGeneratorLibrary.load();
        if (!loadok1)
        {
            errorString1 = mPrivates->mGeneratorLibrary.errorString();
            // Try again with expected absolute path in case current dir is not in the search path
            // (depends on distribution) or LD_LIBRARY_PATH, but if that fail, show the first error message
            const auto absGeneratorLibName = QString("%1/bin/%2").arg(mPrivates->hopsanRoot.c_str()).arg(generatorLibName);
            mPrivates->mGeneratorLibrary.setFileName(absGeneratorLibName);
            loadok2 = mPrivates->mGeneratorLibrary.load();
            if (!loadok2)
            {
                errorString2 = mPrivates->mGeneratorLibrary.errorString();
            }
        }

        if (loadok1 || loadok2)
        {
            printMessage(QString("Loaded %1").arg(generatorLibName));
            return true;
        }
        else
        {
            printErrorMessage(errorString1);
            printErrorMessage(errorString2);
            return false;
        }
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

    constexpr auto functionName = "callModelicaGenerator";
    const auto mofile = modelicaFile.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    bool doCompile = (compile == CompileT::DoCompile);

    using ModelicaImportFunction_t = void(const char*, const char*, MessageHandler_t, void*, int, bool, const char*);
    bool didOK = mPrivates->call<ModelicaImportFunction_t>(functionName, mofile.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(lw->widget()), solver, doCompile, hopsanRoot.c_str());
    lw->setDidSucceed(didOK);
    return didOK;
}

bool HopsanGeneratorGUI::generateFromFmu(const QString& fmuFilePath, const QString& destination)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    QFileInfo fmuFileInfo(fmuFilePath);
    QString fmuFileName = fmuFileInfo.baseName();
    QDir importDestination(QDir::cleanPath(destination+"/"+fmuFileName));
    if(importDestination.exists())
    {
        printErrorMessage(QString("Destination already exist %1").arg(importDestination.path()));
        return false;
    }
    else
    {
        constexpr auto functionName = "callFmuImportGenerator";
        const auto fmu = fmuFilePath.toStdString();
        const auto dst = destination.toStdString();
        const auto& hopsanRoot = mPrivates->hopsanRoot;
        const auto& compilerPath = mPrivates->compilerPath;

        using FmuImportFunction_t = void(const char*, const char*, const char*, const char*, MessageHandler_t, void*);
        bool didOK = mPrivates->call<FmuImportFunction_t>(functionName, fmu.c_str(), dst.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
        lw->setDidSucceed(didOK);
        return didOK;
    }
}

bool HopsanGeneratorGUI::generateToFmu(const QString& outputPath, hopsan::ComponentSystem* pSystem,
                                       const FmuVersionT version, const TargetArchitectureT architecture)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callFmuExportGenerator";
    const auto outpath = outputPath.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    const auto& compilerPath = mPrivates->compilerPath;
    int arch =  (architecture == TargetArchitectureT::x64) ? 64 : 32;

    using FmuExportFunction_t = void(const char*, hopsan::ComponentSystem*, const char*, const char*, int, int, MessageHandler_t, void*);
    bool didOK = mPrivates->call<FmuExportFunction_t>(functionName, outpath.c_str(), pSystem, hopsanRoot.c_str(), compilerPath.c_str(), static_cast<int>(version), arch, &messageHandler, static_cast<void*>(lw->widget()));
    lw->setDidSucceed(didOK);
    return didOK;
}


bool HopsanGeneratorGUI::generateToSimulink(const QString& outputPath, const QString& modelPath,
                                            hopsan::ComponentSystem *pSystem, const UsePortlablesT portLabels)
{
    auto lw = mPrivates->createNewWidget();
    loadGeneratorLibrary();

    constexpr auto functionName = "callSimulinkExportGenerator";
    const auto outpath = outputPath.toStdString();
    const auto modelpath = modelPath.toStdString();
    const auto& hopsanRoot = mPrivates->hopsanRoot;
    bool disablePortLabels = (portLabels == UsePortlablesT::DisablePortLables);

    using SimulinkExportFunction_t = void(const char*, const char*,  hopsan::ComponentSystem*, bool, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<SimulinkExportFunction_t>(functionName, outpath.c_str(), modelpath.c_str(), pSystem, disablePortLabels, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
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

    using LabViewSITExportFunction_t = void(const char*, hopsan::ComponentSystem*, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<LabViewSITExportFunction_t>(functionName, outpath.c_str(), pSystem, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
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


    using CppGeneratorFunction_t = void(const char*, const char*, bool, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<CppGeneratorFunction_t>(functionName, hppfile.c_str(), compilerPath.c_str(), doCompile, hopsanRoot.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
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

    using GenerateLibraryFunction_t = void(const char*, const char* const*, size_t, MessageHandler_t, void*);
    bool didOK = mPrivates->call<GenerateLibraryFunction_t>(functionName, outpath.c_str(), hppfiles.data(), hppfiles.size(), &messageHandler, static_cast<void*>(lw->widget()));
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

    using CompileLibraryFunction_t = void(const char*, const char*, const char*, const char*, const char*, MessageHandler_t, void*);
    bool didOK = mPrivates->call<CompileLibraryFunction_t>(functionName, libpath.c_str(), cflags.c_str(), lflags.c_str(), hopsanRoot.c_str(), compilerPath.c_str(), &messageHandler, static_cast<void*>(lw->widget()));
    lw->setDidSucceed(didOK);
    return didOK;
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
