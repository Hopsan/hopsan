#ifndef HCOMHANDLER_H
#define HCOMHANDLER_H

#include "LogVariable.h"

class TerminalWidget;
class TerminalConsole;
class ModelObject;
class HcomCommand;



class HcomHandler
{
public:
    enum VariableType{Scalar, DataVector};
    enum OptDataType{Int, Double};
    enum OptAlgorithmType{Uninitialized, Complex};
    HcomHandler(TerminalConsole *pConsole);

    QStringList getCommands();

    //Command functions
    void executeCommand(QString cmd);
    void executeExitCommand(QString cmd);
    void executeSimulateCommand(QString cmd);
    void executePlotCommand(QString cmd);
    void executePlotLeftAxisCommand(QString cmd);
    void executePlotRightAxisCommand(QString cmd);
    void executeDisplayParameterCommand(QString cmd);
    void executeChangeParameterCommand(QString cmd);
    void executeChangeSimulationSettingsCommand(QString cmd);
    void executeHelpCommand(QString cmd);
    void executeRunScriptCommand(QString cmd);
    void executeWriteHistoryToFileCommand(QString cmd);
    void executePrintCommand(QString cmd);
    void executeChangePlotWindowCommand(QString cmd);
    void executeDisplayPlotWindowCommand(QString cmd);
    void executeDisplayVariablesCommand(QString cmd);
    void executePeekCommand(QString cmd);
    void executePokeCommand(QString cmd);
    void executeDefineAliasCommand(QString cmd);
    void executeSetCommand(QString cmd);
    void executeSaveToPloCommand(QString cmd);
    void executeLoadModelCommand(QString cmd);
    void executeLoadRecentCommand(QString cmd);
    void executePwdCommand(QString cmd);
    void executeChangeDirectoryCommand(QString cmd);
    void executeListFilesCommand(QString cmd);
    void executeCloseModelCommand(QString cmd);
    void executeChangeTabCommand(QString cmd);
    void executeAddComponentCommand(QString cmd);
    void executeConnectCommand(QString cmd);
    void executeCreateModelCommand(QString cmd);
    void executeExportToFMUCommand(QString cmd);
    void executeAverageCommand(QString cmd);
    void executeMinCommand(QString cmd);
    void executeMaxCommand(QString cmd);
    void executeChangeTimestepCommand(QString cmd);
    void executeInheritTimestepCommand(QString cmd);
    void executeRandomCommand(QString cmd);
    void executeFloorCommand(QString cmd);
    void executeCeilCommand(QString cmd);
    void executeRoundCommand(QString cmd);
    void executeSizeCommand(QString cmd);
    void executeBodeCommand(QString cmd);
    void executeAbsCommand(QString cmd);
    void executeSimulationTimeCommand(QString cmd);
    void executeOptimizationCommand(QString cmd);
    void executeCallFunctionCommand(QString cmd);
    void executeEchoCommand(QString cmd);

    //Help functions
    void changePlotVariables(QString cmd, int axis);
    void addPlotCurve(QString cmd, int axis);
    void removePlotCurves(int axis);
    QString evaluateExpression(QString expr, VariableType *returnType, bool *evalOk);
    void getComponents(QString str, QList<ModelObject*> &components);
    void getParameters(QString str, ModelObject* pComponent, QStringList &parameters);
    void getParameters(QString str, QStringList &parameters);
    QString getParameterValue(QString parameter) const;
    void getVariables(const QString str, QStringList &variables) const;
    void getVariablesThatStartsWithString(const QString str, QStringList &variables) const;
    QString getWorkingDirectory() const;
    bool evaluateArithmeticExpression(QString cmd);
    void splitAtFirst(QString str, QString c, QString &left, QString &right);
    bool containsOutsideParentheses(QString str, QString c);
    QString runScriptCommands(QStringList &lines, bool *abort);
    SharedLogVariableDataPtrT getVariablePtr(QString fullName) const;
    double getNumber(const QString str, bool *ok);
    void toShortDataNames(QString &variable) const;
    QString getDirectory(const QString cmd) const;
    QStringList getArguments(const QString cmd) const;
    int getNumberOfArguments(const QString cmd) const;
    QString getArgument(const QString cmd, const int idx) const;
    void abortHCOM();
    void returnScalar(const double retval);

private:
    void opt_complex_init();
    void opt_complex_run();
    void opt_complex_forget();
    void opt_complex_calculateBestAndWorstId();
    void opt_complex_findCenter();
    bool opt_comlex_checkConvergence();
    double opt_complex_maxParDiff();

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

    VariableType mRetvalType;

    //Functions
    QMap<QString, QStringList> mFunctions;

    //Optimization
    int mOptNumPoints;
    int mOptNumParameters;
    QVector<double> mOptParMin, mOptParMax;
    QVector< QVector<double> > mOptParameters, mOptOldParameters;
    QVector<double> mOptObjectives;
    OptAlgorithmType mOptAlgorithm;
    OptDataType mOptParameterType;
    int mOptWorstCounter;
    double mOptMaxEvals;
    double mOptAlpha, mOptRfak, mOptGamma, mOptKf;
    double mOptWorst;
    int mOptWorstId, mOptBestId, mOptLastWorstId;
    QVector<double> mOptCenter;
    int mOptConvergenceReason;
    double mOptParTol, mOptFuncTol;

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
    void (HcomHandler::*fnc)(QString);
};

#endif // HCOMHANDLER_H
