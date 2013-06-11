#ifndef HCOMHANDLER_H
#define HCOMHANDLER_H

#include "LogVariable.h"
#include "symhop/SymHop.h"

class TerminalWidget;
class TerminalConsole;
class ModelObject;
class HcomCommand;



class HcomHandler : public QObject
{
    Q_OBJECT

    friend class TerminalWidget;
    friend class TerminalConsole;
public:
    //Enums
    enum VariableType{Scalar, DataVector};
    enum OptDataType{Int, Double};
    enum OptAlgorithmType{Uninitialized, Complex, ParticleSwarm};

    //Constructor
    HcomHandler(TerminalConsole *pConsole);

    //Command functions
    void executeCommand(QString cmd);

    //Public access functions
    QString runScriptCommands(QStringList &lines, bool *abort);
    SharedLogVariableDataPtrT getVariablePtr(QString fullName) const;
    QStringList getCommands() const;
    QMap<QString, double> getLocalVariables() const;
    QMap<QString, SymHop::Function> getLocalFunctionPointers() const;
    double getOptimizationObjectiveValue(int idx);

    //Public utilities
    void toShortDataNames(QString &variable) const;

public slots:
    void abortHCOM();
    void optPlotPoints();
    void optPlotBestWorstObj();

private:

    //Private command functions (to be accessed by public executeCommand function)
    void executeExitCommand(const QString cmd);
    void executeSimulateCommand(const QString cmd);
    void executePlotCommand(const QString cmd);
    void executePlotLeftAxisCommand(const QString cmd);
    void executePlotRightAxisCommand(const QString cmd);
    void executeDisplayParameterCommand(const QString cmd);
    void executeChangeParameterCommand(const QString cmd);
    void executeChangeSimulationSettingsCommand(const QString cmd);
    void executeHelpCommand(const QString cmd);
    void executeRunScriptCommand(const QString cmd);
    void executeWriteHistoryToFileCommand(const QString cmd);
    void executePrintCommand(const QString cmd);
    void executeChangePlotWindowCommand(const QString cmd);
    void executeDisplayPlotWindowCommand(const QString cmd);
    void executeDisplayVariablesCommand(const QString cmd);
    void executePeekCommand(const QString cmd);
    void executePokeCommand(const QString cmd);
    void executeDefineAliasCommand(const QString cmd);
    void executeSetCommand(const QString cmd);
    void executeSaveToPloCommand(const QString cmd);
    void executeLoadModelCommand(const QString cmd);
    void executeLoadRecentCommand(const QString cmd);
    void executePwdCommand(const QString cmd);
    void executeMwdCommand(const QString cmd);
    void executeChangeDirectoryCommand(const QString cmd);
    void executeListFilesCommand(const QString cmd);
    void executeCloseModelCommand(const QString cmd);
    void executeChangeTabCommand(const QString cmd);
    void executeAddComponentCommand(const QString cmd);
    void executeConnectCommand(const QString cmd);
    void executeCreateModelCommand(const QString cmd);
    void executeExportToFMUCommand(const QString cmd);
    void executeChangeTimestepCommand(const QString cmd);
    void executeInheritTimestepCommand(const QString cmd);
    void executeBodeCommand(const QString cmd);
    void executeAbsCommand(const QString cmd);
    void executeOptimizationCommand(const QString cmd);
    void executeCallFunctionCommand(const QString cmd);
    void executeEchoCommand(const QString cmd);
    void executeEditCommand(const QString cmd);
    void executeLp1Command(const QString cmd);

    //Help functions
    void createCommands();
    void generateCommandsHelpText();
    void changePlotVariables(const QString cmd, const int axis) const;
    void addPlotCurve(QString cmd, const int axis) const;
    void removePlotCurves(const int axis) const;
    QString evaluateExpression(QString expr, VariableType *returnType, bool *evalOk);
    void getComponents(QString str, QList<ModelObject*> &components);
    void getParameters(QString str, ModelObject* pComponent, QStringList &parameters);
    void getParameters(QString str, QStringList &parameters);
    QString getParameterValue(QString parameter) const;
    void getVariables(const QString str, QStringList &variables) const;
    void getVariablesThatStartsWithString(const QString str, QStringList &variables) const;
    void setWorkingDirectory(QString dir);
    QString getWorkingDirectory() const;
    bool evaluateArithmeticExpression(QString cmd);
    void splitAtFirst(QString str, QString c, QString &left, QString &right);
    bool containsOutsideParentheses(QString str, QString c);
    double getNumber(const QString str, bool *ok);
    QString getDirectory(const QString cmd) const;
    QStringList getArguments(const QString cmd) const;
    int getNumberOfArguments(const QString cmd) const;
    QString getArgument(const QString cmd, const int idx) const;
    void returnScalar(const double retval);
    void registerFunction(const QString func, const QString description, const SymHop::Function fptr);

    void optComplexInit();
    void optComplexRun();
    void optComplexForget();
    void optComplexCalculatebestandworstid();
    void optComplexFindcenter();
    bool optComlexCheckconvergence();
    double optComplexMaxpardiff();

    void optParticleInit();
    void optParticleRun();



    TerminalConsole *mpConsole;

    //Used to abort HCOM evaluation
    bool mAborted;

    //Working directory
    QString mPwd;

    //Commands
    QList<HcomCommand> mCmdList;

    //Plotting
    QString mCurrentPlotWindowName;

    //Local variables
    QMap<QString, double> mLocalVars;
    QMap<QString, SymHop::Function> mLocalFunctionPtrs;
    QMap<QString, QString> mLocalFunctionDescriptions;

    VariableType mRetvalType;

    //Functions
    QMap<QString, QStringList> mFunctions;

    //Optimization
    int mOptNumPoints;
    int mOptNumParameters;
    QVector<double> mOptParMin, mOptParMax;
    QVector< QVector<double> > mOptParameters, mOptOldParameters;
    QVector< QVector<double> > mOptVelocities, mOptBestKnowns;
    QVector<double> mOptObjectives, mOptBestObjectives, mOptBestPoint;
    OptAlgorithmType mOptAlgorithm;
    OptDataType mOptParameterType;
    int mOptWorstCounter;
    double mOptBestObj;
    double mOptMaxEvals;
    double mOptAlpha, mOptRfak, mOptGamma, mOptKf;
    double mOptOmega, mOptC1, mOptC2;
    double mOptWorst;
    int mOptWorstId, mOptBestId, mOptLastWorstId;
    QVector<double> mOptCenter;
    int mOptConvergenceReason;
    double mOptParTol, mOptFuncTol;
    bool mOptMulticore;
    bool mOptPlotPoints;
    bool mOptPlotBestWorst;
    bool mOptPlotVariables;
};


class HcomCommand
{
public:
    void runCommand(QString cmd, HcomHandler *pHandler)
    {
        //(fnc)(cmd);
        (pHandler->*HcomCommand::fnc)(cmd);
    }

    QString cmd;
    QString description;
    QString help;
    QString group;
    void (HcomHandler::*fnc)(QString);
};


double _funcAver(QString str);
double _funcMin(QString str);
double _funcMax(QString str);
double _funcIMin(QString str);
double _funcIMax(QString str);
double _funcPeek(QString str);
double _funcRand(QString str);
double _funcSize(QString str);
double _funcObj(QString str);
double _funcTime(QString);

#endif // HCOMHANDLER_H
