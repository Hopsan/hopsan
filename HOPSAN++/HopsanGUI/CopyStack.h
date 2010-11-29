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
