/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2013, Institut f�r Technische Optik (ITO),
    Universit�t Stuttgart, Germany

    This file is part of itom.
  
    itom is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public Licence as published by
    the Free Software Foundation; either version 2 of the Licence, or (at
    your option) any later version.

    itom is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
    General Public Licence for more details.

    You should have received a copy of the GNU Library General Public License
    along with itom. If not, see <http://www.gnu.org/licenses/>.
*********************************************************************** */

#include "widgetFindWord.h"

WidgetFindWord::WidgetFindWord(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);

    ui.cmdMarkAll->setVisible(false); //not supported yet

    this->hide();
    ui.txtFind->selectAll();
}

WidgetFindWord::~WidgetFindWord()
{

}

void WidgetFindWord::on_cmdClose_clicked()
{
    this->hide();
}

void WidgetFindWord::on_txtFind_returnPressed()
{
    on_cmdFindDown_clicked();
}

void WidgetFindWord::on_cmdFindUp_clicked()
{
    bool regExpr = (ui.checkRegExpr->checkState() == Qt::Checked);
    bool caseSensitive = (ui.checkCaseSensitive->checkState() == Qt::Checked);
    bool wholeWord = (ui.checkWholeWord->checkState() == Qt::Checked);
    bool wrap = (ui.checkWrapAround->checkState() == Qt::Checked);
    emit findNext( ui.txtFind->text(), regExpr, caseSensitive, wholeWord, wrap, false, true);
}

void WidgetFindWord::on_cmdFindDown_clicked()
{
    bool regExpr = (ui.checkRegExpr->checkState() == Qt::Checked);
    bool caseSensitive = (ui.checkCaseSensitive->checkState() == Qt::Checked);
    bool wholeWord = (ui.checkWholeWord->checkState() == Qt::Checked);
    bool wrap = (ui.checkWrapAround->checkState() == Qt::Checked);
    emit findNext( ui.txtFind->text(), regExpr, caseSensitive, wholeWord, wrap, true, true);
}

void WidgetFindWord::on_txtFind_textChanged ( const QString & /*text*/ )
{
    ui.txtFind->setStyleSheet("background-color: white");
}

void WidgetFindWord::setFindBarEnabled(bool enabled)
{
    ui.txtFind->setEnabled(enabled);
    ui.cmdFindDown->setEnabled(enabled);
    ui.cmdFindUp->setEnabled(enabled);
    ui.cmdMarkAll->setEnabled(enabled);
    ui.checkCaseSensitive->setEnabled(enabled);
    ui.checkRegExpr->setEnabled(enabled);
    ui.checkWholeWord->setEnabled(enabled);
    ui.checkWrapAround->setEnabled(enabled);
}

void WidgetFindWord::setSuccessState(bool successfull)
{
    if(successfull)
    {
        ui.txtFind->setStyleSheet("background-color: white");
    }
    else
    {
        ui.txtFind->setStyleSheet("background-color: red");
    }
}

void WidgetFindWord::setCursorToTextField()
{
    ui.txtFind->setFocus();
    ui.txtFind->selectAll();
}