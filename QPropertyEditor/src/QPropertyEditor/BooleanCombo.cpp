// *************************************************************************************************
//
// QPropertyEditor v 0.3
//   
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// Acknowledgements to Roman alias banal from qt-apps.org for the Enum enhancement
//
//
// The QPropertyEditor Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The Horde3D Scene Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************************************

#include "BooleanCombo.h"

#include <qcolordialog.h>
#include <qdebug.h>

BooleanCombo::BooleanCombo(QWidget* parent /*= 0*/) : QComboBox(parent)
{	
	addItem("true");
    addItem("false");
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentChanged(int)));
}


BooleanCombo::~BooleanCombo()
{
}


bool BooleanCombo::value() const
{
	return currentIndex() == 0;
}

void BooleanCombo::setValue(bool value)
{
    setCurrentIndex( value == true ? 0 : 1 );
}

void BooleanCombo::currentChanged(int index)
{
    emit boolChanged(index == 0);
}
