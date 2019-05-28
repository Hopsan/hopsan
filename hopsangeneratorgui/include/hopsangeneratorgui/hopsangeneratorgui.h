#ifndef HOPSANGENERATORGUI_H
#define HOPSANGENERATORGUI_H

#include <QWidget>
#include <QTextEdit>
#include <QColor>
#include <memory>

// Forward declarations
namespace hopsan {
class ComponentSystem;
}

class HopsanGeneratorGUI : public QWidget
{
public:
    HopsanGeneratorGUI(const QString& hopsanInstallPath, QWidget* pWidgetParent=0);
    virtual ~HopsanGeneratorGUI();

    bool isGeneratorLibraryLoaded() const;
    bool loadGeneratorLibrary();
    void setCompilerPath(const QString& compilerPath);
    void setAutoCloseWidgetsOnSuccess(bool doAutoClose);

    enum class FmuKindT {ModelExchange, CoSimulation};
    enum class FmuVersionT {One=1, Two=2};
    enum class TargetArchitectureT {x86, x64};
    enum class CompileT {DoCompile, DoNotCompile};
    enum class UsePortlablesT {EnablePortLabels, DisablePortLables};

    bool generateFromModelica(const QString& modelicaFile, const int solver=0,
                              const CompileT compile=CompileT::DoNotCompile);

    bool generateFromFmu(const QString& fmuFilePath, const QString& destination);
    bool generateToFmu(const QString& outputPath, hopsan::ComponentSystem *pSystem, const QStringList& externalLibraries, const FmuVersionT version,
                       TargetArchitectureT arch);

    bool generateToSimulink(const QString& outputPath, const QString& modelPath, hopsan::ComponentSystem *pSystem, const QStringList &externalLibraries,
                            const UsePortlablesT portLables=UsePortlablesT::EnablePortLabels);

    bool generateToLabViewSIT(const QString& outputPath, hopsan::ComponentSystem *pSystem);

    bool generateToExe(const QString& outputPath, hopsan::ComponentSystem *pSystem, const QStringList& externalLibraries, TargetArchitectureT architecture);

    bool generateFromCpp(const QString& hppFile, CompileT compile=CompileT::DoNotCompile);

    bool generateLibrary(const QString& outputPath, const QStringList& hppFiles);
    bool compileComponentLibrary(const QString& libPath, const QString& extraCFlags="", const QString& extraLFlags="");

    bool checkComponentLibrary(const QString& libraryXMLPath);

    void printMessage(const QString& msg, const char type='I');
    void printErrorMessage(const QString& msg);
    void printWarningMessage(const QString& msg);

private:
    struct PrivateImpl;
    std::unique_ptr<PrivateImpl> mPrivates;
};

#endif // HOPSANGENERATORGUI_H
