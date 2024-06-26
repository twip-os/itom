/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2020, Institut für Technische Optik (ITO),
    Universität Stuttgart, Germany

    This file is part of itom and its software development toolkit (SDK).

    itom is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public Licence as published by
    the Free Software Foundation; either version 2 of the Licence, or (at
    your option) any later version.

    In addition, as a special exception, the Institut für Technische
    Optik (ITO) gives you certain additional rights.
    These rights are described in the ITO LGPL Exception version 1.0,
    which can be found in the file LGPL_EXCEPTION.txt in this package.

    itom is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
    General Public Licence for more details.

    You should have received a copy of the GNU Library General Public License
    along with itom. If not, see <http://www.gnu.org/licenses/>.

    This file is a port and modified version of the
    CTK Common Toolkit (http://www.commontk.org)
*********************************************************************** */
/*=========================================================================

Program:   Maverick
Module:    $RCSfile: config.h,v $

Copyright (c) Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#ifndef PATHLINEEDIT_H
#define PATHLINEEDIT_H

// Qt includes
#include <QDir>
#include <QWidget>
class QComboBox;

// CTK includes
#include "commonWidgets.h"
class PathLineEditPrivate;

/// \ingroup Widgets
/// \brief Advanced line edit to select a file or directory.
/// \sa ctkDirectoryButton, ctkPathListWidget
///
class ITOMWIDGETS_EXPORT PathLineEdit: public QWidget
{
  Q_OBJECT

  Q_FLAGS(Filters)
  Q_PROPERTY ( QString label READ label WRITE setLabel )

  Q_PROPERTY ( Filters filters READ filters WRITE setFilters)
  Q_PROPERTY ( QString currentPath READ currentPath WRITE setCurrentPath USER true )
  /// Qt versions prior to 4.7.0 didn't expose QFileDialog::Options in the
  /// public API. We need to create a custom property that will be used when
  /// instantiating a QFileDialog in PathLineEdit::browse()
#ifdef USE_QFILEDIALOG_OPTIONS
  Q_PROPERTY(QFileDialog::Options options READ options WRITE setOptions)
#else
  Q_PROPERTY(Options options READ options WRITE setOptions)
  Q_FLAGS(Option Options)
#endif

  /// This property controls the key used to search the settings for recorded
  /// paths.
  /// If multiple path line edits share the same key, their history is then
  /// shared.
  /// If an empty key string is given, the object name is used as key.
  /// Setting the key automatically retrieve the history from settings
  /// Empty by default.
  /// \sa retrieveHistory(), addCurrentPathToHistory(), showHistoryButton
  Q_PROPERTY(QString settingKey READ settingKey WRITE setSettingKey )

  /// This property controls whether the browse ("...") button is visible or
  /// not. Clicking on the button calls opens a dialog to select the current path.
  /// True by default
  /// \sa browse()
  Q_PROPERTY(bool showBrowseButton READ showBrowseButton WRITE setShowBrowseButton)

  /// This property controls whether the history button (arrow button that opens
  /// the history menu) is visible or not.
  /// True by default.
  /// \sa retrieveHistory(), addCurrentPathToHistory(), settingKey
  Q_PROPERTY(bool showHistoryButton READ showHistoryButton WRITE setShowHistoryButton)

  /// This property holds the policy describing how the size of the path line edit widget
  /// changes when the content changes.
  /// The default value is AdjustToContentsOnFirstShow.
  Q_PROPERTY(SizeAdjustPolicy sizeAdjustPolicy READ sizeAdjustPolicy WRITE setSizeAdjustPolicy)

  /// This property holds the minimum number of characters that should fit into
  /// the path line edit.
  /// The default value is 0.
  /// If this property is set to a positive value, the minimumSizeHint() and sizeHint() take it into account.
  Q_PROPERTY(int minimumContentsLength READ minimumContentsLength WRITE setMinimumContentsLength)

  /// This property holds the list of regular expressions (in wildcard mode) used to help the user
  /// complete a line.
  /// For example: "Images (*.jpg *.gif *.png)"
  Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)

public:
  enum Filter { Dirs        = 0x001,
                Files       = 0x002,
                Drives      = 0x004,
                NoSymLinks  = 0x008,
                AllEntries  = Dirs | Files | Drives,
                TypeMask    = 0x00f,
                Readable    = 0x010,
                Writable    = 0x020,
                Executable  = 0x040,
                PermissionMask    = 0x070,
                Modified    = 0x080,
                Hidden      = 0x100,
                System      = 0x200,
                AccessMask  = 0x3F0,
                AllDirs       = 0x400,
                CaseSensitive = 0x800,
                NoDotAndDotDot = 0x1000, // ### Qt5 NoDotAndDotDot = NoDot|NoDotDot
                NoDot         = 0x2000,
                NoDotDot      = 0x4000,
                NoFilter = -1
  };
  Q_DECLARE_FLAGS(Filters, Filter)

#ifndef USE_QFILEDIALOG_OPTIONS
  // Same options than QFileDialog::Options
  enum Option
  {
    ShowDirsOnly          = 0x00000001,
    DontResolveSymlinks   = 0x00000002,
    DontConfirmOverwrite  = 0x00000004,
    DontUseSheet          = 0x00000008,
    DontUseNativeDialog   = 0x00000010,
    ReadOnly              = 0x00000020,
    HideNameFilterDetails = 0x00000040
  };
  Q_DECLARE_FLAGS(Options, Option)
#endif

  enum SizeAdjustPolicy
  {
    /// The path line edit will always adjust to the contents.
    AdjustToContents,
    /// The path line edit will adjust to its contents the first time it is shown.
    AdjustToContentsOnFirstShow,
    /// The combobox will adjust to minimumContentsLength. For performance reasons
    /// use this policy on large models.
    AdjustToMinimumContentsLength
  };

  //Q_ENUM exposes a meta object to the enumeration types, such that the key names for the enumeration
  //values are always accessible.
  Q_ENUM(SizeAdjustPolicy);

  /** Default constructor
  */
  PathLineEdit(QWidget *parent = 0);

  /** Constructor
   *  /param label        Used in file dialogs
   *  /param nameFilters  Regular expression (in wildcard mode) used to help the user to complete the line,
   *                      example: "Images (*.jpg *.gif *.png)"
   *  /param parent       Parent widget
  */
  PathLineEdit( const QString& label,
                   const QStringList& nameFilters,
                   Filters filters = PathLineEdit::AllEntries,
                   QWidget *parent=0 );
  virtual ~PathLineEdit();
  QString currentPath()const;

  void setLabel(const QString &label);
  const QString& label()const;

  void setNameFilters(const QStringList &nameFilters);
  const QStringList& nameFilters()const;

  void setFilters(const Filters& filters);
  Filters filters()const;

  /// Options of the file dialog pop up.
  /// \sa QFileDialog::getExistingDirectory
#ifdef USE_QFILEDIALOG_OPTIONS
  void setOptions(const QFileDialog::Options& options);
  const QFileDialog::Options& options()const;
#else
  void setOptions(const Options& options);
  const Options& options()const;
#endif

  /// Change the current extension of the edit line.
  ///  If there is no extension yet, set it
  void setCurrentFileExtension(const QString& extension);

  QString settingKey()const;
  void setSettingKey(const QString& key);

  bool showBrowseButton()const;
  void setShowBrowseButton(bool visible);

  bool showHistoryButton()const;
  void setShowHistoryButton(bool visible);

  /// the policy describing how the size of the combobox changes
  /// when the content changes
  ///
  /// The default value is \c AdjustToContentsOnFirstShow.
  ///
  /// \sa SizeAdjustPolicy
  SizeAdjustPolicy sizeAdjustPolicy() const;

  void setSizeAdjustPolicy(SizeAdjustPolicy policy);

  int minimumContentsLength()const;
  void setMinimumContentsLength(int length);

  /// Return the combo box internally used by the path line edit
  QComboBox* comboBox() const;

  /// The width returned, in pixels, is the length of the file name (with no
  /// path) if any. Otherwise, it's enough for 15 to 20 characters.
  virtual QSize minimumSizeHint()const;

  /// The width returned, in pixels, is the entire length of the current path
  /// if any. Otherwise, it's enough for 15 to 20 characters.
  virtual QSize sizeHint()const;

signals:
  /** the signal is emit when the state of hasValidInput changed
  */
  void validInputChanged(bool);

  void currentPathChanged(const QString& path);

public slots:
  void setCurrentPath(const QString& path);

  /// Open a QFileDialog to select a file or directory and set current text to it
  /// You would probably connect a browse push button like this:
  /// connect(myPushButton,SIGNAL(clicked()),myPathLineEdit,SLOT(browse()))
  /// As a conveniency, such button is provided by default via the browseButton
  /// \sa showBrowseButton
  void browse();

  /// Load the history of the paths that have been saved in the application
  /// settings with addCurrentPathToHistory().
  /// The history is identified using \a settingKey
  /// \sa addCurrentPathToHistory(), showHistoryButton, settingKey
  void retrieveHistory();

  /// Save the current value (this->currentPath()) into the history. That value
  ///  will be retrieved next time retrieveHistory() is called.
  /// \sa retrieveHistory(), showHistoryButton, settingKey
  void addCurrentPathToHistory();

protected Q_SLOTS:
  void setCurrentDirectory(const QString& directory);
  void updateHasValidInput();

protected:
  QScopedPointer<PathLineEditPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(PathLineEdit);
  Q_DISABLE_COPY(PathLineEdit);

  Q_PRIVATE_SLOT(d_ptr, void _q_recomputeCompleterPopupSize())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PathLineEdit::Filters)
#ifndef USE_QFILEDIALOG_OPTIONS
Q_DECLARE_OPERATORS_FOR_FLAGS(PathLineEdit::Options);
#endif

#endif // PATHLINEEDIT_H
