//!
//! @file   CopyStack.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for an XML-based copy stack
//!
//$Id$

#ifndef COPYSTACK_H
#define COPYSTACK_H

#include <QDomElement>

class CopyStack
{
public:
    CopyStack();
    void clear();
    QString getXML();
    QDomElement *getCopyRoot();

private:
    QDomDocument mDomDocument;
    QDomElement mCopyRoot;
};

#endif // COPYSTACK_H
