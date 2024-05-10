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

    In addition, as a special exception, the Institut fuer Technische
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
    Common framework (http://www.commontk.org)
*********************************************************************** */

#ifndef DOUBLESPINBOX_H
#define DOUBLESPINBOX_H

// Qt includes
#include <QMetaType>
#include <QString>
#include <QWidget>

class QDoubleSpinBox;
class QEvent;
class QKeyEvent;
class QLineEdit;
class QObject;

#include "commonWidgets.h"

class DoubleSpinBoxPrivate;
class ValueProxy;

/// \brief Custom SpinBox
/// The DoubleSpinBox internally uses a QDoubleSpinBox while it retain controls
/// over it.
/// \sa DoubleSlider, SliderWidget, RangeSlider
class ITOMWIDGETS_EXPORT DoubleSpinBox : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
  Q_PROPERTY(bool frame READ hasFrame WRITE setFrame)
  Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
  Q_PROPERTY(QString cleanText READ cleanText)
  /// This property holds the precision of the spin box, in decimals.
  /// Sets how many decimals the spinbox will use for displaying and
  /// interpreting doubles.
  /// If the flag DecimalsByShortcuts is set, decimals can be increased/decreased by
  /// Ctrl+/Ctrl-, Ctrl0 restores the original decimals value.
  /// If the flag DecimalsAsMax and/or DecimalsAsMin are set, decimals behave also
  /// as the max and/or min number of decimals settable by DecimalsByShortcuts,
  /// DecimalsByKey and DecimalsByValue.
  /// 2 by default.
  /// \sa decimalsOption, decimals(), setDecimals(), decimalsChanged
  Q_PROPERTY(int decimals READ decimals WRITE setDecimals NOTIFY decimalsChanged)
  /// This property provides more controls over the decimals.
  /// The default (DecimalsByShortcuts|InsertDecimals) behaves as a QDoubleSpinbox
  /// with an explicit control of decimals via shortcuts.
  /// \sa DecimalsOptions, decimals
  Q_PROPERTY(DecimalsOptions decimalsOption READ decimalsOption WRITE setDecimalsOption)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  /// \sa setMode, decimals
  Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged USER true)
  /// This property controls how setValue behaves.
  /// \sa SetMode, setMode(), setSetMode(), value
  Q_PROPERTY(SetMode setMode READ setMode WRITE setSetMode)
  /// This property controls whether decreasing the value by the mouse
  /// button or mouse wheel increases the value of the widget, and inverts the
  /// control similarly in the other way round or not. The property is switched off by
  /// default.
  /// \sa invertedControls(), setInvertedControls()
  Q_PROPERTY(bool invertedControls READ invertedControls WRITE setInvertedControls)
  /// This property controls the size hint of the spinbox.
  /// SizeHintByMinMax by default
  /// SizeHintPolicy, sizeHintPolicy(), setSizeHintPolicy()
  Q_PROPERTY(SizeHintPolicy sizeHintPolicy READ sizeHintPolicy WRITE setSizeHintPolicy)
  /// This property wraps the keyboardTracking property of the underlying spinbox.
  Q_PROPERTY(bool keyboardTracking READ keyboardTracking WRITE setKeyboardTracking)

public:

  /// SetMode enums for the spinbox behavior.
  /// SetAlways:
  /// No check is made and the given parameters is directly set
  /// on the internal QDoubleSpinBox.
  /// SetIfDifferent:
  /// Default mode, the given parameter is checked against the
  /// current internal value and only set if they are different.
  /// For double, the comparison is based on the input parameters rounded
  /// with the current number of decimals (see round()).
  /// \sa setMode(), setSetMode(), round()
  enum SetMode
    {
    SetAlways,
    SetIfDifferent,
    };

  /// DecimalsOption enums the input style of the spinbox decimals.
  /// Default option is DecimalsByShortcuts.
  /// \sa decimals(), currentDecimals()
  enum DecimalsOption
    {
    /// Behaves just like a QDoubleSpinBox. The maximum number of decimals
    /// allowed is given by decimals().
    FixedDecimals = 0x000,
    /// When the spinbox has focus, entering the shortcut "CTRL +"
    /// adds decimals to the current number of decimals. "CTRL -" decreases the
    /// number of decimals and "CTRL 0" returns it to its original decimals()
    /// value.
    DecimalsByShortcuts = 0x001,
    /// Allow the number of decimals to be controlled by adding/removing digits
    /// with key strokes.
    /// \sa InsertDecimals, ReplaceDecimals
    DecimalsByKey = 0x002,
    /// Allow the number of decimals to be controlled by the value set by
    /// setValue().
    DecimalsByValue = 0x004,
    /// This flag controls whether inserted intermediate decimals increase the
    /// number of decimals (on) or not (off).
    /// It is incompatible with the ReplaceDecimals flag.
    /// \sa DecimalsByKey.
    InsertDecimals = 0x008,
    /// This flag controls whether inserted intermediate decimals replace the
    /// existing decimals (on) or not (off). This is similar to Insert but just
    /// for decimal digits.
    /// It is incompatible with the InsertDecimals flag.
    /// \sa DecimalsByKey, InsertDecimals
    ReplaceDecimals = 0x010,
    /// Use the "decimals" property as a maximum limit for the number of
    /// decimals.
    DecimalsAsMax = 0x020,
    /// Use the "decimals" property as a minimum limit for the number of
    /// decimals.
    DecimalsAsMin = 0x040,
    /// Even if the number of decimals is 0, it enforces the decimal to be visible
    /// (e.g. "0." )
    /// \sa decimals
    DecimalPointAlwaysVisible = 0x080
    };
  Q_DECLARE_FLAGS(DecimalsOptions, DecimalsOption)

  enum SizeHintPolicy
    {
    SizeHintByMinMax,
    SizeHintByValue
    };

  //Q_ENUM exposes a meta object to the enumeration types, such that the key names for the enumeration
  //values are always accessible.
  Q_ENUM(SetMode)
  Q_ENUM(SizeHintPolicy)
  Q_FLAG(DecimalsOptions)

  typedef QWidget Superclass;

  /// Constructor, creates a DoubleSpinBox. The look and feel
  /// are the same as of a QDoubleSpinBox
  explicit DoubleSpinBox(QWidget* parent = 0);
  explicit DoubleSpinBox(DoubleSpinBox::SetMode mode, QWidget* parent = 0);
  virtual ~DoubleSpinBox();

  /// Get the spinbox current value
  /// \sa setValue(), cleanText()
  double value() const;

  /// Get the spinbox current displayed value
  /// \sa value(), cleanText(), setValue(), displayedValue()
  double displayedValue() const;

  /// Set the displayed value if there is no proxy installed.
  /// If there is a proxy installed, then it allows to modify the proxy value.
  /// If there is no value proxy installed, then it's just a setter to the
  /// value property.
  /// \sa displayedValue(), setValue(), value(), setValueProxy()
  void setDisplayedValue(double displayValue);

  /// Get the spinbox current text. This includes any prefix or suffix.
  /// \sa prefix(), suffix()
  QString text() const;

  /// Get the spinbox current text. This excludes any prefix or suffix.
  /// \sa value()
  QString cleanText() const;

  /// Set/Get the spinbox alignment
  Qt::Alignment alignment () const;
  void setAlignment (Qt::Alignment flag);

  /// Set/Get the frame
  void setFrame(bool frame);
  bool hasFrame() const;

  /// Add/Get a prefix to the displayed value. For example, one might want to
  /// add the $ sign.
  /// \sa suffix(), text()
  QString prefix() const;
  void setPrefix(const QString &prefix);

  /// Add/Get a suffix to the displayed value. For example, one might want to
  /// add the F (fahrenheit) sign.
  /// \sa prefix(), text()
  QString suffix() const;
  void setSuffix(const QString &suffix);

  /// Set/Get the single step. This represents by how much the value will
  /// decrease or increase when clicking the spinbox arrow or using stepUp or
  /// stepDown(). Default is 1.0.
  /// \sa setUp(), stepDown(), setDecimals().
  double singleStep() const;
  void setSingleStep(double value);

  /// Set/Get the range of the spinbox. Default range is [0.0, 9.9].
  double minimum() const;
  void setMinimum(double min);
  double maximum() const;
  void setMaximum(double max);
  void setRange(double min, double max);

  /// Set/Get number of displayed decimals.
  /// For a spinbox dealing only with integers, set this to 0.
  /// \sa DoubleSpinBox::DecimalsOption
  int decimals() const;

  /// Returns the rounded value according to the number of decimals of
  /// the spinbox.
  /// \sa decimals()
  double round(double value) const;

  /// Get a pointer on the spinbox used internally. It can be useful to
  /// change display properties for example. To use with caution.
  /// \sa QDoubleSpinBox, lineEdit()
  QDoubleSpinBox* spinBox() const;

  /// Get a pointer on the line edit of the spinbox.
  /// \sa QLineEdit, spinBox()
  QLineEdit* lineEdit()const;

  /// Set the spinbox mode when using a set*() method.
  //// \sa round(), setValue(), setValueIfDifferent(), setValueAlways()
  DoubleSpinBox::SetMode setMode() const;
  void setSetMode(SetMode mode);

  /// Set/Get the option used to input the decimals.
  /// \sa DoubleSpinBox::DecimalsOption
  DoubleSpinBox::DecimalsOptions decimalsOption();
  void setDecimalsOption(DoubleSpinBox::DecimalsOptions option);

  /// This property holds whether or not the spin box inverts its wheel and key
  /// events.
  /// If this property is false, scrolling the mouse wheel "up" and using keys
  /// like page up will increase the spinbox's value towards its maximum.
  /// Otherwise pressing page up will move value towards the slider's minimum.
  void setInvertedControls(bool invertedControls);
  bool invertedControls() const;

  /// Set the sizeHintPolicy property value.
  /// \sa sizeHintPolicy
  void setSizeHintPolicy(SizeHintPolicy newSizeHintPolicy);
  /// Return the sizeHintPolicy property value.
  /// \sa sizeHintPolicy
  SizeHintPolicy sizeHintPolicy()const;

  /// Install or remove a value proxy filter. The value proxy decouples the
  /// displayed value from the value retrieved by the value property.
  /// For example, the value proxy can allow one to display celsius in the
  /// spinbox while the value retrieved from the value property and signals
  /// are in fahrenheit.
  /// To remove the proxy, simply install a new empty proxy. The proxy
  /// installation/removal is silent.
  /// \sa installValueProxy(), valueProxy()
  void setValueProxy(ValueProxy* proxy);
  ValueProxy* valueProxy() const;

  /// Reimplemented to respect the sizeHintPolicy property value.
  /// \sa sizeHintPolicy
  virtual QSize sizeHint()const;
  /// Reimplemented to respect the sizeHintPolicy property value.
  /// \sa sizeHintPolicy
  virtual QSize minimumSizeHint()const;

  void setKeyboardTracking(bool kt);
  bool keyboardTracking() const;

public slots:
  /// Set the value of the spinbox following the current mode.
  /// \sa setMode(), value(), setValueIfDifferent(), setValueAlways()
  void setValue(double value);

  /// Set the value of the spinbox following the SetIfDifferent mode.
  /// \sa value(), setValue(), setMode(), setValueAlways()
  void setValueIfDifferent(double value);

  /// Set the value of the spinbox following the SetAlways mode.
  /// \sa value(), setValue(), setMode(), setValueIfDifferent()
  void setValueAlways(double value);

  /// Increase/Decrease the current value by a single step.
  /// \sa value(), singleStep()
  void stepUp();
  void stepDown();

  /// Set the decimals property value.
  /// \sa decimals
  void setDecimals(int decimal);

signals:
  /// Emitted every time the spinbox value is modified
  /// \sa QDoubleSpinBox::valueChanged()
  void valueChanged(double);
  void valueChanged(const QString &);

  /// Simple broadcast of the QAbstractSpinbox::editingFinished
  /// \sa QAbstractSpinbox::editingFinished
  void editingFinished();

  /// Signal emitted when the decimals of the displayed are changed.
  void decimalsChanged(int);

protected:
  DoubleSpinBoxPrivate* const d_ptr;

  /// Reimplemented to support shortcuts.
  virtual void keyPressEvent(QKeyEvent* event);
  /// Reimplemented to support shortcuts on the double spinbox.
  virtual bool eventFilter(QObject *obj, QEvent *event);

  friend class CoordinatesWidgetPrivate;
private:
  Q_DECLARE_PRIVATE(DoubleSpinBox);
  Q_DISABLE_COPY(DoubleSpinBox);
};

Q_DECLARE_METATYPE(DoubleSpinBox::SetMode)
Q_DECLARE_OPERATORS_FOR_FLAGS(DoubleSpinBox::DecimalsOptions)

#endif //__DoubleSpinBox_h
