//!
//! @file   UndoWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains the undo widget class (which displays the stack)
//!
//$Id$

#ifndef UNDOWIDGET_H
#define UNDOWIDGET_H

#include <QList>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QGridLayout>

#include <QDomElement>
#include <QDomDocument>
#include "../MainWindow.h"

    //Forward Declarations
class MainWindow;

class UndoWidget : public QDialog
{
public:
    UndoWidget(MainWindow *parent = 0);
    void show();
    void refreshList();
    QString translateTag(QString tag);
    QPushButton *getUndoButton();
    QPushButton *getRedoButton();
    QPushButton *getClearButton();

private:
    QTableWidget *mUndoTable;
    QList< QList<QString> > mTempStack;
    QPushButton *mpUndoButton;
    QPushButton *mpRedoButton;
    QPushButton *mpClearButton;
    QGridLayout *mpLayout;
};


#endif // UNDOWIDGET_H
