//$Id$

#ifndef CopyStack_H
#define CopyStack_H

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

#endif // CopyStack_H
