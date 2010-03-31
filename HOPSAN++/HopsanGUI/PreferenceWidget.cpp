//$Id$


#include <QtGui>
#include "PreferenceWidget.h"

PreferenceWidget::PreferenceWidget(QWidget *parent)
    : QMainWindow(parent,Qt::Tool)
{
    //Set the name and size of the main window
    this->setObjectName("PreferenceWidget");
    this->resize(640,480);
    this->setWindowTitle("Model Preferences");
    this->setWindowIcon(QIcon("../../HopsanGUI/icons/hopsan.ico"));
}

PreferenceWidget::~PreferenceWidget()
{
}
