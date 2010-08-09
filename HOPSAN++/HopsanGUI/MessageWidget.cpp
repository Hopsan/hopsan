/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

#include "MessageWidget.h"
#include "MainWindow.h"

using namespace hopsan;

MessageWidget::MessageWidget(MainWindow *pParent)
    : QTextEdit(pParent)
{
    mpParentMainWindow = pParent;
}

QSize MessageWidget::sizeHint() const
{
    QSize size = QTextEdit::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 48; //pixels
    return size;
}

void MessageWidget::setMessageColor(int type)
{
    if (type == HopsanCoreMessage::ERROR)
    {
        setTextColor("RED");
    }
    else if (type == HopsanCoreMessage::WARNING)
    {
        setTextColor("ORANGE");
    }
    else if (type == HopsanCoreMessage::INFO)
    {
        setTextColor("GREEN");
    }
    else
    {
        setTextColor("BLACK");
    }
}


void MessageWidget::printCoreMessages()
{
    //*****Core Interaction*****
    HopsanCoreMessage msg;
    HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
    if (pHopsanCore != 0)
    {
        size_t nmsg = pHopsanCore->checkMessage();
        for (size_t idx=0; idx < nmsg; ++idx)
        {
            msg = pHopsanCore->getMessage();
            if (true) //! @todo Debug level rules
            {
                setMessageColor(msg.type);
                append(QString::fromStdString(msg.message));
            }
        }
    }
    else
    {
        printGUIMessage("The hopsan core pointer is not set, can not get core messages");
    }
    //**************************
}

void MessageWidget::printGUIMessage(QString message)
{
    //! @todo make better
    setMessageColor(-1);
    append(message);
}

void MessageWidget::printGUIErrorMessage(QString message)
{
    //! @todo make better
    setMessageColor(HopsanCoreMessage::ERROR);
    append(message);
}

void MessageWidget::printGUIWarningMessage(QString message)
{
    //! @todo make better
    setMessageColor(HopsanCoreMessage::WARNING);
    append(message);
}

void MessageWidget::printGUIInfoMessage(QString message)
{
    //! @todo make better
    setMessageColor(HopsanCoreMessage::INFO);
    append(message);
}

void MessageWidget::checkMessages()
{
    printCoreMessages();
}
