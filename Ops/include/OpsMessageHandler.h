#ifndef OPSMESSAGEHANDLER_H
#define OPSMESSAGEHANDLER_H

#include <sstream>

namespace Ops {


class MessageHandler
{
public:
    MessageHandler() { mIsAborted = false;}

    void printMessage(std::string msg)
    {
        std::stringstream ss;
        ss << msg;
        printMessage(ss.str().c_str());
    }

    virtual void printMessage(const char*) {}
    virtual void pointChanged(size_t) {}
    virtual void pointsChanged() {}
    virtual void candidateChanged(size_t) {}
    virtual void candidatesChanged() {}
    virtual void objectiveChanged(size_t) {}
    virtual void objectivesChanged() {}
    virtual void stepCompleted(size_t) {}

    bool aborted() { return mIsAborted; }

    void setAborted(const bool value) { mIsAborted = value; }

protected:
    bool mIsAborted;
};

}

#endif // OPSMESSAGEHANDLER_H
