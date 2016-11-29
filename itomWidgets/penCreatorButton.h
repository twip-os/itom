/* ********************************************************************
itom measurement system
URL: http://www.uni-stuttgart.de/ito
Copyright (C) 2016, Institut fuer Technische Optik (ITO),
Universitaet Stuttgart, Germany

This file is part of itom.

itom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

itom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with itom. If not, see <http://www.gnu.org/licenses/>.
*********************************************************************** */


#ifndef PENCREATORBUTTON_H
#define PENCREATORBUTTON_H

#include <QPushButton>
#include "commonWidgets.h"

class PenCreatorButtonPrivate;

class ITOMWIDGETS_EXPORT PenCreatorButton : public QPushButton
{
     Q_OBJECT
    public:
     explicit PenCreatorButton(QWidget* parent = 0);
     ~PenCreatorButton();

    protected:
        QScopedPointer<PenCreatorButtonPrivate> d_ptr;
    private:
        
       Q_DECLARE_PRIVATE(PenCreatorButton);
       Q_DISABLE_COPY(PenCreatorButton);

};






#endif