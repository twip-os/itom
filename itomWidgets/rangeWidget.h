/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2013, Institut f�r Technische Optik (ITO),
    Universit�t Stuttgart, Germany

    This file is part of itom and its software development toolkit (SDK).

    itom is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public Licence as published by
    the Free Software Foundation; either version 2 of the Licence, or (at
    your option) any later version.
   
    In addition, as a special exception, the Institut f�r Technische
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

#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

// Qt includes
#include <QSlider>


#include "commonWidgets.h"

class RangeSlider;
class QSpinBox;
class RangeWidgetPrivate;

/// \ingroup Widgets
///
/// RangeWidget is a wrapper around a DoubleRangeSlider and 2 QSpinBoxes
/// \image html http://www.commontk.org/images/1/14/RangeWidget.png
/// \sa ctkSliderSpinBoxWidget, DoubleRangeSlider, QSpinBox
class ITOMWIDGETS_EXPORT RangeWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(int minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(int maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
  Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval)
  Q_PROPERTY(bool autoSpinBoxWidth READ isAutoSpinBoxWidth WRITE setAutoSpinBoxWidth)
  Q_PROPERTY(Qt::Alignment spinBoxTextAlignment READ spinBoxTextAlignment WRITE setSpinBoxTextAlignment)
  Q_PROPERTY(Qt::Alignment spinBoxAlignment READ spinBoxAlignment WRITE setSpinBoxAlignment)
  Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
  Q_PROPERTY(bool symmetricMoves READ symmetricMoves WRITE setSymmetricMoves)

public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  /// If \li parent is null, RangeWidget will be a top-leve widget
  /// \note The \li parent can be set later using QWidget::setParent()
  explicit RangeWidget(QWidget* parent = 0);
  
  /// Destructor
  virtual ~RangeWidget();

  ///
  /// This property holds the sliders and spinbox minimum value.
  /// FIXME: Test following specs.
  /// When setting this property, the maximum is adjusted if necessary
  /// to ensure that the range remains valid.
  /// Also the slider's current value is adjusted to be within the new range.
  int minimum()const;
  void setMinimum(int minimum);

  ///
  /// This property holds the sliders and spinbox minimum value.
  /// FIXME: Test following specs.
  /// When setting this property, the maximum is adjusted if necessary
  /// to ensure that the range remains valid.
  /// Also the slider's current value is adjusted to be within the new range.
  int maximum()const;
  void setMaximum(int maximum);
  /// Description
  /// Utility function that set the min/max in once
  void setRange(int min, int max);
  void range(int minimumAndMaximum[2])const;

  ///
  /// This property holds the slider and spinbox minimum value.
  /// RangeWidget forces the value to be within the
  /// legal range: minimum <= minimumValue <= maximumValue <= maximum.
  int minimumValue()const;

  ///
  /// This property holds the slider and spinbox maximum value.
  /// RangeWidget forces the value to be within the
  /// legal range: minimum <= minimumValue <= maximumValue <= maximum.
  int maximumValue()const;

  ///
  /// Utility function that returns both values at the same time
  /// Returns minimumValue and maximumValue
  void values(int &minValue, int &maxValue)const;

  ///
  /// This property holds the single step.
  /// The smaller of two natural steps that the
  /// slider provides and typically corresponds to the
  /// user pressing an arrow key.
  int singleStep()const;
  void setSingleStep(int step);

  ///
  /// This property holds the spin box's prefix.
  /// The prefix is prepended to the start of the displayed value.
  /// Typical use is to display a unit of measurement or a currency symbol
  QString prefix()const;
  void setPrefix(const QString& prefix);

  ///
  /// This property holds the spin box's suffix.
  /// The suffix is appended to the end of the displayed value.
  /// Typical use is to display a unit of measurement or a currency symbol
  QString suffix()const;
  void setSuffix(const QString& suffix);

  ///
  /// This property holds the interval between tickmarks.
  /// This is a value interval, not a pixel interval.
  /// If it is 0, the slider will choose between lineStep() and pageStep().
  /// The default value is 0.
  int tickInterval()const;
  void setTickInterval(int ti);

  ///
  /// This property holds the alignment of the spin boxes.
  /// Possible Values are Qt::AlignTop, Qt::AlignBottom, and Qt::AlignVCenter.
  /// By default, the alignment is Qt::AlignVCenter
  void setSpinBoxAlignment(Qt::Alignment alignment);
  Qt::Alignment spinBoxAlignment()const;

  ///
  /// This property holds the alignment of the text inside the spin boxes.
  /// Possible Values are Qt::AlignLeft, Qt::AlignRight, and Qt::AlignHCenter.
  /// By default, the alignment is Qt::AlignLeft
  void setSpinBoxTextAlignment(Qt::Alignment alignment);
  Qt::Alignment spinBoxTextAlignment()const;

  ///
  /// This property holds whether slider tracking is enabled.
  /// If tracking is enabled (the default), the widget emits the valueChanged()
  /// signal while the slider or spinbox is being dragged. If tracking is
  /// disabled, the widget emits the valueChanged() signal only when the user
  /// releases the slider or spinbox.
  void setTracking(bool enable);
  bool hasTracking()const;

  ///
  /// Set/Get the auto spinbox width
  /// When the autoSpinBoxWidth property is on, the width of the SpinBox is
  /// set to the same width of the largest QSpinBox of its
  // RangeWidget siblings.
  bool isAutoSpinBoxWidth()const;
  void setAutoSpinBoxWidth(bool autoWidth);
  
  ///
  /// When symmetricMoves is true, moving a handle will move the other handle
  /// symmetrically, otherwise the handles are independent. False by default
  bool symmetricMoves()const;
  void setSymmetricMoves(bool symmetry);

  /// Return the slider of the range widget.
  /// \sa minimumSpinBox(), maximumSpinBox()
  RangeSlider* slider()const;
  /// Return the minimum spinbox.
  /// \sa maximumSpinBox(), slider()
  QSpinBox* minimumSpinBox()const;
  /// Return the maximum spinbox.
  /// \sa minimumSpinBox(), slider()
  QSpinBox* maximumSpinBox()const;

public Q_SLOTS:
  ///
  /// Reset the slider and spinbox to zero (value and position)
  void reset();
  void setMinimumValue(int value);
  void setMaximumValue(int value);
  ///
  /// Utility function that set the min and max values at once
  void setValues(int minValue, int maxValue);

Q_SIGNALS:
  /// Use with care:
  /// sliderMoved is emitted only when the user moves the slider
  //void sliderMoved(int position);
  void minimumValueChanged(int value);
  void minimumValueIsChanging(int value);
  void maximumValueChanged(int value);
  void maximumValueIsChanging(int value);
  void valuesChanged(int minValue, int maxValue);
  void rangeChanged(int min, int max);

protected Q_SLOTS:
  void startChanging();
  void stopChanging();
  void changeValues(int newMinValue, int newMaxValue);
  void changeMinimumValue(int value);
  void changeMaximumValue(int value);
  /// A spinbox value has been modified, update the slider.
  void setSliderValues();
  void setMinimumToMaximumSpinBox(int minimum);
  void setMaximumToMinimumSpinBox(int maximum);
  void onSliderRangeChanged(int min, int max);

protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);

  /// can be used to change the slider by a custom one
  void setSlider(RangeSlider* slider);

protected:
  QScopedPointer<RangeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(RangeWidget);
  Q_DISABLE_COPY(RangeWidget);

};

#endif
