.. _plot-line:

line plots (1D)
******************

Plot of an N x M dataObject
===========================

In order to plot a 1D line (1 x M or M x 1 DataObject) or multiple lines in a
line plot, the designer plugin **Itom1DQwtPlot** is the recommended
visualization tool. You can either use the :py:meth:`~itom.plot1` command or
add this class name (*itom1dqwtplot*) to any :py:meth:`~itom.plot` or
:py:meth:`~itom.liveImage` command in order to force the data to be plotted
in this plot type. Alternatively set this plugin to be the default plot for
1D static and live plots.

The latter can be done in the property dialog or itom, tab **Plots and
Figures >> Default Plots**. Set there the *default figure plot* to
*Itom1DQwtPlot* for the categories

* DObjLiveLine (live 1D visualization of a dataObject)
* DObjStaticLine (static 1D visualization of a dataObject)

.. figure:: images/plot1d_2.png
    :scale: 100%
    :align: center


This plot has been created by the following code snippet:

.. code-block:: python

    import numpy as np
    X = np.linspace(0,100)
    Y = np.sin(X/20)
    plot(Y, className = 'itom1dqwtplot', \
        properties = {"title":"sin(X/20)", "valueLabel":"Amplitude"})

.. _itom1dqwtplot:

Plot of an N x M dataObject with 1 x M x-vector
===============================================

It is also possible to set an optional x-vector to the plot. This can only be
done by giving a second argument to the :py:meth:`~itom.plot1` function or by
setting the property **xData** to an existing plot. To plot an N x M
dataObject, the x-vector has to be a 1 x M dataObject.
For the y axis and x axis label the valueDescription and valueUnit definitions are taken accordingly.

.. figure:: images/spiral.png
    :scale: 100%
    :align: center

This spiral plot has been created by the demo
:file:`itom/demo/plots/plotXY.py`. The three methods to create such a plot
are listed here:

.. code-block:: python

    # method 1
    plot1(yDataObject, xVector)

    # method 2
    figNumber, handle = plot1(yDataObject)
    handle['xData'] = xVector

    # method 3
    plot1(yDataObject, properties = {'xData', xVector})


.. _itom1dqwtplotdatetime:

DateTime x-axis
=================

It is also possible to use date time values for the x-axis. This is done by setting
a ``datetime`` :py:class:`~itom.dataObject` as ``xData`` property of the 1d plot (see section above).

The ``datetime`` :py:class:`~itom.dataObject` can either be constructed from :py:class:`datetime.datetime`
values or a ``np.datetime64`` :py:class:`numpy.ndarray`.

The following code snippet:

.. code-block:: python

    # This demo shows how the x-axis of a 1d plot can be a date time.

    import numpy as np
    import datetime

    # start date with a specific timezone
    a = datetime.datetime(
        2022, 5, 6, 12, 23, 5, tzinfo=datetime.timezone(datetime.timedelta(0, -7200))
    )

    # create a list of datetime.datetime objects
    numsteps = 100
    dateList = []

    for x in range(0, numsteps, 15):
        dateList.append(
            a + datetime.timedelta(hours=x)
        )

    # create a dataObject from the list of datetime objects
    dateScale = dataObject([1, len(dateList)], "datetime", data=dateList)

    values = dataObject.randN(dateScale.shape, "float32")

    [i, h] = plot1(values, dateScale)

    h["lineWidth"] = 3
    h["axisLabelRotation"] = -45
    h["axisLabelAlignment"] = "AlignLeft"
    h["fillCurve"] = "FillFromBottom"
    h["grid"] = "GridMajorXY"
    h["axisLabel"] = "date"
    h["valueLabel"] = "value"

The result looks like this:

.. figure:: images/plot1d_datetime.png
    :scale: 100%
    :align: center

It is also possible to directly use a ``datetime`` :py:class:`numpy.ndarray` as ``xData`` object:

.. code-block:: python

    dateScale = np.arange('2005-02', '2005-03', dtype='datetime64[D]')
    values = dataObject.randN([1, len(dateScale)], "uint8")
    plot1(values, dateScale)

.. note::

    For more information about datetime in :py:class:`itom.dataObject`, see :ref:`itomDataObjectDatetime`.

Itom1dQwtPlot
==========================

If you call :py:meth:`~itom.plot1` or choose *itom1dqwtplot* as className for
the :py:meth:`~itom.plot` command with a 2D data object as argument, is it
also possible to plot multiple lines. The plot plugin accepts all available
data types, including colors and complex values.

Data is plotted as follows:

* Real data types: One or multiple lines are plotted where the horizontal axis
  corresponds to the grid of the data object considering possible scaling and
  offset values. The line(s) have different colours. The colour of one line
  can also be adjusted.
* Complex types: This is the same than for real data types, however you can
  choose whether the *absolute*, *phase*, *real* or *imaginary* part of the
  complex values is plotted.
* color type: Coloured data objects will be represented by either 3 or 4 lines
  (red, green and blue, alpha optional) that correspond to the three colour
  channels or by one line representing the converted gray values.

Using Python or the properties toolbox (View >> properties or via right click
on the toolbar), it is possible to adjust many properties like stated in the
section Properties below.

The plot allows value and min/max-picking via place-able marker.

The plot supports geometric element and marker interaction via
**drawAndPickElements(...)** and **call("userInteractionStart",...)**. See
section :ref:`primitives` for a short introduction.

Calling the :py:meth:`~itom.plot1` you can pass also an optional x-vector to
plot a xy data set.

You can also use the "matplotlib"-backend to plot slices or xy-coordinates.
See section :ref:`pymod-matplotlib` for more information about how to use
"matplotlib".

The plot-canvas can be exported to vector and bitmap-graphics via button or
menu entry or it can be exported to clipBoard via ctrl-c or a public slot.

Setting properties
-------------------------

If you want to modify the lines, plotted in the :ref:`itom1dqwtplot`,
individually you can do this with the help of Python or the curve properties
widget (View >> curve properties or via right click on the toolbar). The
following line properties are accessible via Python:

* **visible**
* **lineStyle**
* **lineWidth**
* **lineColor**
* **lineJoinStyle**
* **lineCapStyle**
* **lineSymbolSize**
* **lineSymbolStyle**
* **legendVisible**

A property of a line can be accessed as followed:

.. code-block:: python

    plotHandle.call("setCurveProperty", index, property, value)

with the *index* (int) of the curve, the name of the *property* (str) and the
value (various) the property will be set to. For a detailed description of
the properties read the section below. In the file
:file:`itom/demo/plots/demoPlot1DLineProperties.py` a short demonstration of
how to set the properties is given.

For accessing the properties via the user interface the line properties widget
can be used. Additional to the properties listed above the legend name, the
legend visibility and the symbol color can be set.

.. figure:: images/curveProperties.png
    :scale: 100%
    :align: center

This shows the curve Properties widget.

If you want to set a property global for all curves you can also use the
properties widget (View >> properties or via right click on the toolbar)

Legend title of line plots
---------------------------

Legend titles of a line plot can be activated and modified by the **curve
properties** or by the **plot properties**. Per default the legendtitles are
defined as *curve 0, curve 1,...* If the dataObject which is plotted has
defined tags named **legendTitle0, legendTitle1, ...**, then the tag entries
are used as legendTitles.

.. note::

    An example is located in the demo script folder of itom.


.. _itom1dqwtplot-ref:

.. BEGIN The following part is obtained by the script plot_help_to_rst_format.py (source/itom/docs/userDoc) from the designer plugin itom1dqwtplot.
..       Before starting this script, disable the long-line wrap mode in the console / line wrap property page of itom.

Properties
-------------------------


.. py:attribute:: legendLabelWidth : int
    :noindex:

    Defines the width of a legend label. This can be used to create a longer line in legend entries. The minimal size is 10

.. py:attribute:: antiAliased : bool
    :noindex:

    True, if all curves should be plot with an anti-aliased render mode (slower) or False if not (faster).

.. py:attribute:: xData : dataObject
    :noindex:

    DataObject representing the xData of the plot. Expect a two dimensional dataObject with a (n x m) or (1 x m) shape for an (n x m) source object with n < m. For n > m a shape of (n x 1) or (n x m) is expected.

.. py:attribute:: axisScale : ItomQwtPlotEnums::ScaleEngine
    :noindex:

    linear or logarithmic scale (various bases) can be chosen for the horizontal axis (x-axis). Please consider, that a logarithmic scale can only display values > 1e-100.

    The type 'ItomQwtPlotEnums::ScaleEngine' is an enumeration that can have one of the following values (str or int):

    * 'Linear' (1)
    * 'Log2' (2)
    * 'Log10' (10)
    * 'Log16' (16)
    * 'LogLog2' (1002)
    * 'LogLog10' (1010)
    * 'LogLog16' (1016)

.. py:attribute:: valueScale : ItomQwtPlotEnums::ScaleEngine
    :noindex:

    linear or logarithmic scale (various bases) can be chosen for the vertical axis (y-axis). Please consider, that a logarithmic scale can only display values > 1e-100 while the lower limit for the double-logarithmic scale is 1+1e-100.

    The type 'ItomQwtPlotEnums::ScaleEngine' is an enumeration that can have one of the following values (str or int):

    * 'Linear' (1)
    * 'Log2' (2)
    * 'Log10' (10)
    * 'Log16' (16)
    * 'LogLog2' (1002)
    * 'LogLog10' (1010)
    * 'LogLog16' (1016)

.. py:attribute:: pickerLabelAlignment : Qt::Alignment
    :noindex:

    Get / set label alignment for the picker labels.

    The type 'Qt::Alignment' is a flag mask that can be a combination of one or several of the following values (or-combination number values or semicolon separated strings):

    * 'AlignLeft' (1)
    * 'AlignLeading' (1)
    * 'AlignRight' (2)
    * 'AlignTrailing' (2)
    * 'AlignHCenter' (4)
    * 'AlignJustify' (8)
    * 'AlignAbsolute' (16)
    * 'AlignHorizontal_Mask' (31)
    * 'AlignTop' (32)
    * 'AlignBottom' (64)
    * 'AlignVCenter' (128)
    * 'AlignBaseline' (256)
    * 'AlignVertical_Mask' (480)
    * 'AlignCenter' (132)

.. py:attribute:: pickerLabelOrientation : Qt::Orientation
    :noindex:

    Get / set the label orientation for the picker labels.

    The type 'Qt::Orientation' is an enumeration that can have one of the following values (str or int):

    * 'Horizontal' (1)
    * 'Vertical' (2)

.. py:attribute:: pickerLabelVisible : bool
    :noindex:

    Enable and disable the labels next to each picker.

.. py:attribute:: picker : dataObject (readonly)
    :noindex:

    Get picker defined by a Mx4 float32 data object. Each row represents one picker and contains the following information: [pixelIndex, physIndex, value, curveIndex]. PixelIndex and physIndex are equal if axisScale = 1 and axisOffset = 0 for the corresponding dataObject.

.. py:attribute:: currentPickerIndex : int
    :noindex:

    Get / set currently active picker.

.. py:attribute:: pickerCount : int (readonly)
    :noindex:

    Number of picker within the plot.

.. py:attribute:: pickerLimit : int
    :noindex:

    Define the maximal number of picker for this plot.

.. py:attribute:: pickerType : ItomQwtPlotEnums::PlotPickerType
    :noindex:

    Get / set the current picker type ('DefaultMarker', 'RangeMarker', 'ValueRangeMarker', 'AxisRangeMarker')

    The type 'ItomQwtPlotEnums::PlotPickerType' is an enumeration that can have one of the following values (str or int):

    * 'DefaultMarker' (0)
    * 'RangeMarker' (1)
    * 'ValueRangeMarker' (2)
    * 'AxisRangeMarker' (3)

.. py:attribute:: columnInterpretation : ItomQwtPlotEnums::MultiLineMode
    :noindex:

    Define the interpretation of M x N objects as Auto, FirstRow, FirstCol, MultiRows, MultiCols.

    The type 'ItomQwtPlotEnums::MultiLineMode' is an enumeration that can have one of the following values (str or int):

    * 'AutoRowCol' (0)
    * 'FirstRow' (1)
    * 'FirstCol' (2)
    * 'MultiRows' (3)
    * 'MultiCols' (4)
    * 'MultiLayerAuto' (5)
    * 'MultiLayerCols' (6)
    * 'MultiLayerRows' (7)

.. py:attribute:: grid : GridStyle
    :noindex:

    Style of the grid ('GridNo', 'GridMajorXY', 'GridMajorX', 'GridMajorY', 'GridMinorXY', 'GridMinorX', 'GridMinorY').

    The type 'GridStyle' is an enumeration that can have one of the following values (str or int):

    * 'GridNo' (0)
    * 'GridMajorXY' (1)
    * 'GridMajorX' (2)
    * 'GridMajorY' (3)
    * 'GridMinorXY' (4)
    * 'GridMinorX' (5)
    * 'GridMinorY' (6)

.. py:attribute:: lineSymbolSize : int
    :noindex:

    Get / Set the current line symbol size

.. py:attribute:: lineSymbol : Symbol
    :noindex:

    Get / Set the current line symbol type

    The type 'Symbol' is an enumeration that can have one of the following values (str or int):

    * 'NoSymbol' (0)
    * 'Ellipse' (1)
    * 'Rect' (2)
    * 'Diamond' (3)
    * 'Triangle' (4)
    * 'DTriangle' (5)
    * 'UTriangle' (6)
    * 'LTriangle' (7)
    * 'RTriangle' (8)
    * 'Cross' (9)
    * 'XCross' (10)
    * 'HLine' (11)
    * 'VLine' (12)
    * 'Star1' (13)
    * 'Star2' (14)
    * 'Hexagon' (15)

.. py:attribute:: baseLine : float
    :noindex:

    If curveStyle is set to 'Sticks', 'SticksVertical' or 'SticksHorizontal', the baseline indicates the start point of each line either in vertical or horizontal direction. For all other curve types, the baseline is considered if fillCurve is set to 'FillBaseLine'.

.. py:attribute:: curveFillAlpha : int
    :noindex:

    set the alpha value for the curve fill color separately.

.. py:attribute:: curveFillColor : color str, rgba or hex
    :noindex:

    the fill color for the curve, invalid color leads to line color selection.

.. py:attribute:: fillCurve : ItomQwtPlotEnums::FillCurveStyle
    :noindex:

    fill curve below / above or according to baseline.

    The type 'ItomQwtPlotEnums::FillCurveStyle' is an enumeration that can have one of the following values (str or int):

    * 'NoCurveFill' (-1)
    * 'FillBaseLine' (0)
    * 'FillFromTop' (1)
    * 'FillFromBottom' (2)

.. py:attribute:: curveStyle : ItomQwtPlotEnums::CurveStyle
    :noindex:

    set the style of the qwt-plot according to curve styles.

    The type 'ItomQwtPlotEnums::CurveStyle' is an enumeration that can have one of the following values (str or int):

    * 'NoCurve' (-1)
    * 'Lines' (0)
    * 'FittedLines' (1)
    * 'Sticks' (2)
    * 'SticksHorizontal' (3)
    * 'SticksVertical' (4)
    * 'Steps' (5)
    * 'StepsRight' (6)
    * 'StepsLeft' (7)
    * 'Dots' (8)

.. py:attribute:: lineStyle : Qt::PenStyle
    :noindex:

    style of all lines.

    The type 'Qt::PenStyle' is an enumeration that can have one of the following values (str or int):

    * 'NoPen' (0)
    * 'SolidLine' (1)
    * 'DashLine' (2)
    * 'DotLine' (3)
    * 'DashDotLine' (4)
    * 'DashDotDotLine' (5)
    * 'CustomDashLine' (6)

.. py:attribute:: lineWidth : float
    :noindex:

    width of all curves in pixel.

.. py:attribute:: legendTitles : seq. of str
    :noindex:

    Seq. of strings with the legend titles for all curves. If no legends are given, the dataObject is checked for tags named 'legendTitle0', 'legendTitle1'... If these tags are not given, the default titles 'curve 0', 'curve 1'... are taken.

.. py:attribute:: legendPosition : LegendPos
    :noindex:

    Position of the legend (Off, Left, Top, Right, Bottom)

    The type 'LegendPos' is an enumeration that can have one of the following values (str or int):

    * 'Off' (0)
    * 'Left' (1)
    * 'Top' (2)
    * 'Right' (3)
    * 'Bottom' (4)

.. py:attribute:: axisLabelAlignment : Qt::Alignment
    :noindex:

    The label alignment for the x-axis. This value has to be adjusted if the rotation is changed.

    The type 'Qt::Alignment' is a flag mask that can be a combination of one or several of the following values (or-combination number values or semicolon separated strings):

    * 'AlignLeft' (1)
    * 'AlignLeading' (1)
    * 'AlignRight' (2)
    * 'AlignTrailing' (2)
    * 'AlignHCenter' (4)
    * 'AlignJustify' (8)
    * 'AlignAbsolute' (16)
    * 'AlignHorizontal_Mask' (31)
    * 'AlignTop' (32)
    * 'AlignBottom' (64)
    * 'AlignVCenter' (128)
    * 'AlignBaseline' (256)
    * 'AlignVertical_Mask' (480)
    * 'AlignCenter' (132)

.. py:attribute:: axisLabelRotation : float
    :noindex:

    The rotation angle in degree of the labels on the bottom x-axis. If changed, the alignment should also be adapted.

.. py:attribute:: legendFont : font
    :noindex:

    Font for legend entries

.. py:attribute:: labelFont : font
    :noindex:

    Font for axes descriptions.

.. py:attribute:: axisFont : font
    :noindex:

    Font for axes tick values.

.. py:attribute:: titleFont : font
    :noindex:

    Font for title.

.. py:attribute:: valueLabel : str
    :noindex:

    Label of the value axis (y-axis) or '<auto>' if the description should be used from data object.

.. py:attribute:: axisLabel : str
    :noindex:

    Label of the direction (x/y) axis or '<auto>' if the descriptions from the data object should be used.

.. py:attribute:: title : str
    :noindex:

    Title of the plot or '<auto>' if the title of the data object should be used.

.. py:attribute:: bounds :
    :noindex:



.. py:attribute:: allowCameraParameterEditor : bool
    :noindex:

    If a live camera is connected to this plot, a camera parameter editor can be displayed as toolbox of the plot. If this property is false, this toolbox is not available (default: true)

.. py:attribute:: complexStyle : ItomQwtPlotEnums::ComplexType
    :noindex:

    Defines whether the real, imaginary, phase or absolute of a complex number is shown. Possible options are CmplxAbs(0), CmplxImag (1), CmplxReal (2) and CmplxArg (3).

    The type 'ItomQwtPlotEnums::ComplexType' is an enumeration that can have one of the following values (str or int):

    * 'CmplxAbs' (0)
    * 'CmplxImag' (1)
    * 'CmplxReal' (2)
    * 'CmplxArg' (3)

.. py:attribute:: markerLabelsVisible : bool
    :noindex:

    Toggle visibility of marker labels, the label is the set name of the marker.

.. py:attribute:: unitLabelStyle : ito::AbstractFigure::UnitLabelStyle
    :noindex:

    style of the axes label (slash: 'name / unit', keyword-in: 'name in unit', square brackets: 'name [unit]'

    The type 'ito::AbstractFigure::UnitLabelStyle' is an enumeration that can have one of the following values (str or int):

    * 'UnitLabelSlash' (0)
    * 'UnitLabelKeywordIn' (1)
    * 'UnitLabelSquareBrackets' (2)

.. py:attribute:: geometricShapesLabelsVisible : bool
    :noindex:

    Toggle visibility of shape labels, the label is the name of the shape.

.. py:attribute:: geometricShapesFillOpacitySelected : int
    :noindex:

    Opacity for the selected geometric shapes with an area > 0. This value ranges from 0 (not filled) to 255 (opaque).

.. py:attribute:: geometricShapesFillOpacity : int
    :noindex:

    Opacity for geometric shapes with an area > 0. This value ranges from 0 (not filled) to 255 (opaque).

.. py:attribute:: allowedGeometricShapes : ItomQwtPlotEnums::ShapeTypes
    :noindex:

    Combination of values of enumeration ShapeType to decide which types of geometric shapes are allowed (default: all shape types are allowed)

    The type 'ItomQwtPlotEnums::ShapeTypes' is a flag mask that can be a combination of one or several of the following values (or-combination number values or semicolon separated strings):

    * 'MultiPointPick' (1)
    * 'Point' (2)
    * 'Line' (4)
    * 'Rectangle' (8)
    * 'Square' (16)
    * 'Ellipse' (32)
    * 'Circle' (64)
    * 'Polygon' (128)

.. py:attribute:: geometryModificationModes : ItomQwtPlotEnums::ModificationModes
    :noindex:

    Bitmask to globally change how geometric shapes can be modified. The possible modes of a shape are both restricted by the shape's flags and the allowed modes of the plot (move: 0x01, rotate: 0x02, resize: 0x04)

    The type 'ItomQwtPlotEnums::ModificationModes' is a flag mask that can be a combination of one or several of the following values (or-combination number values or semicolon separated strings):

    * 'Move' (1)
    * 'Rotate' (2)
    * 'Resize' (4)

.. py:attribute:: geometricShapesDrawingEnabled : bool
    :noindex:

    Enable and disable internal plotting functions and GUI-elements for geometric elements.

.. py:attribute:: selectedGeometricShape : int
    :noindex:

    Get or set the currently highlighted geometric shape. After manipulation the last element stays selected.

.. py:attribute:: geometricShapesCount : int (readonly)
    :noindex:

    Number of currently existing geometric shapes.

.. py:attribute:: geometricShapes : seq. of shape
    :noindex:

    Get or set the geometric shapes on the canvas, they are set as a sequence of itom.shape for each shape.

.. py:attribute:: keepAspectRatio : bool
    :noindex:

    Enable or disable a fixed 1:1 aspect ratio between x and y axis.

.. py:attribute:: backgroundColor : color str, rgba or hex
    :noindex:

    Get/set the background color.

.. py:attribute:: canvasColor : color str, rgba or hex
    :noindex:

    Get/set the color of the canvas.

.. py:attribute:: textColor : color str, rgba or hex
    :noindex:

    Get/set the color of text and tick-numbers.

.. py:attribute:: axisColor : color str, rgba or hex
    :noindex:

    Get/set the color of the axis.

.. py:attribute:: buttonSet : ButtonStyle
    :noindex:

    Get/set the button set used (normal or light color for dark themes).

    The type 'ButtonStyle' is an enumeration that can have one of the following values (str or int):

    * 'StyleBright' (0)
    * 'StyleDark' (1)

.. py:attribute:: enableBoxFrame : bool
    :noindex:

    If true, a 1px solid border is drawn as a boxed rectangle around the canvas, else no margin is visible on the upper and right side.

.. py:attribute:: colorMap : str
    :noindex:

    Color map (string) that should be used to colorize a non-color data object.

.. py:attribute:: zAxisInterval : autoInterval
    :noindex:

    Sets the visible range of the displayed z-axis (in coordinates of the data object). Set it to 'auto' if range should be automatically set [default].

.. py:attribute:: yAxisInterval : autoInterval
    :noindex:

    Sets the visible range of the displayed y-axis (in coordinates of the data object). Set it to 'auto' if range should be automatically set [default].

.. py:attribute:: xAxisInterval : autoInterval
    :noindex:

    Sets the visible range of the displayed x-axis (in coordinates of the data object). Set it to 'auto' if range should be automatically set [default].

.. py:attribute:: camera : dataIO
    :noindex:

    Use this property to set a camera/grabber to this plot (live image).

.. py:attribute:: displayed : dataObject (readonly)
    :noindex:

    This returns the currently displayed data object [read only].

.. py:attribute:: source : dataObject
    :noindex:

    Sets the input data object for this plot.

.. py:attribute:: renderLegend : bool
    :noindex:

    If this property is true, the legend are included in pixelmaps renderings.

.. py:attribute:: contextMenuEnabled : bool
    :noindex:

    Defines whether the context menu of the plot should be enabled or not.

.. py:attribute:: toolbarVisible : bool
    :noindex:

    Toggles the visibility of the toolbar of the plot.

Slots
-------------------------


.. py:function:: getDisplayed() [slot]
    :noindex:


    returns the currently displayed dataObject.


.. py:function:: showCurveProperties() [slot]
    :noindex:



.. py:function:: getCurveProperty(index, property) [slot]
    :noindex:


    Get a property of a specific curve

    Get the value of a property of a specific curve (see slot 'setCurveProperty').

    :param index: zero-based index of the curve whose property should be changed.
    :type index: int
    :param property: name of the property to be changed
    :type property: str

    :return: value -> value of the requested property
    :rtype: variant


.. py:function:: setCurveProperty(index, property, value) [slot]
    :noindex:


    Set a property of a specific curve

    Some curve properties can be changed globally for all curves using the global properties. However, it is also possible to
    set a property to different values for each curve.

    :param index: zero-based index of the curve whose property should be changed.
    :type index: int
    :param property: name of the property to be changed
    :type property: str
    :param value: value of the property
    :type value: various


.. py:function:: deletePicker(id) [slot]
    :noindex:


    Delete the i-th picker (id >= 0) or all pickers (id = -1)

    :param id: zero-based index of the picker to be deleted, or -1 if all pickers should be deleted (default). This parameter is optional.
    :type id: int


.. py:function:: appendPicker(coordinates, curveIndex, physicalCoordinates) [slot]
    :noindex:


    Append plot pickers to a specific curve either in physical (axis) or in pixel coordinates.

    The pixel coordinates are the pixels of the currently displayed dataObject. The coordinates are the axis positions only,
    the values are chosen from the curve values. Existing pickers are not removed before this operation.

    :param coordinates: x-coordinates of each picker, the y-coordinate is automatically chosen from the shape of the curve. If the size of the sequence plus the number of existing pickers exceed the 'pickerLimit', a RuntimeError is thrown.curveIndex : {int} index of the curve where the pickers should be attached to (optional, default: 0 - first curve)
    :type coordinates: seq. of float
    :param physicalCoordinates: optional, if True (default), 'coordinates' are given in axis coordinates of the plot (hence, physical coordinates of the dataObject; False: 'coordinates' are given in pixel coordinates of the dataObject
    :type physicalCoordinates: bool


.. py:function:: setPicker(coordinates, curveIndex, physicalCoordinates) [slot]
    :noindex:


    Set plot pickers to a specific curve either in physical (axis) or in pixel coordinates.

    The pixel coordinates are the pixels of the currently displayed dataObject. The coordinates are the axis positions only,
    the values are chosen from the curve values. Existing pickers are deleted at first.

    :param coordinates: x-coordinates of each picker, the y-coordinate is automatically chosen from the shape of the curve. If the size of the sequence exceeds the 'pickerLimit', a RuntimeError is thrown.curveIndex : {int} index of the curve where the pickers should be attached to (optional, default: 0 - first curve)
    :type coordinates: seq. of float
    :param physicalCoordinates: optional, if True (default), 'coordinates' are given in axis coordinates of the plot (hence, physical coordinates of the dataObject; False: 'coordinates' are given in pixel coordinates of the dataObject
    :type physicalCoordinates: bool


.. py:function:: setLegendTitles(legends) [slot]
    :noindex:



.. py:function:: replot() [slot]
    :noindex:


    Force a replot which is for instance necessary if values of the displayed data object changed and you want to update the plot, too.


.. py:function:: hideMarkers(id) [slot]
    :noindex:


    Hides all existing markers with the given name

    :param id: Name of the set of markers, that should be hidden.
    :type id: str


.. py:function:: showMarkers(id) [slot]
    :noindex:


    Shows all existing markers with the given name

    :param id: Name of the set of markers, that should be shown.
    :type id: str


.. py:function:: deleteMarkers(id) [slot]
    :noindex:


    Delete all sets of markers with the given id or all markers if no or an empty id is passed.

    :param id: name of the marker set that should be removed (optional)
    :type id: str


.. py:function:: plotMarkers(coordinates, style, id, plane) [slot]
    :noindex:


    Draws sub-pixel wise markers to the canvas of the plot

    :param coordinates: 2xN data object with the 2D coordinates of the markers (first row: X, second row: Y coordinates in axis coordinates of the plot)
    :type coordinates: dataObject
    :param style: Style string for the set of markers (e.g. 'r+20' for red crosses with a size of 20px)
    :type style: str
    :param id: Name of the set of added markers (optional, default='')
    :type id: str
    :param plane: If the dataObject has more than 2 dimensions, it is possible to add the markers to a specific plane only (optional, default=-1 -> all planes)
    :type plane: int


.. py:function:: setGeometricShapeLabelVisible(idx, visible) [slot]
    :noindex:


    Set the visibility of the label of a geometric shape with the given index.

    :param idx: index of the shape
    :type idx: int
    :param visible: True if the label should be displayed close to the shape, else False
    :type visible: bool


.. py:function:: setGeometricShapeLabel(idx, label) [slot]
    :noindex:


    Set the label of geometric shape with the index idx.

    :param idx: index of the shape
    :type idx: int
    :param label: new label of the shape
    :type label: str


.. py:function:: updateGeometricShape(geometricShape) [slot]
    :noindex:


    Updates an existing geometric shape by the new shape if the index of the shape already exists, else add the new shape to the canvas (similar to 'addGeometricShape'.

    If the index of the new shape is -1 (default), the next free auto-incremented index will be set for this shape. (C++ only: this new index is
    stored in the optional 'newIndex' parameter).

    :param geometricShape: new geometric shape
    :type geometricShape: shape


.. py:function:: addGeometricShape(geometricShape) [slot]
    :noindex:


    Add a new geometric shape to the canvas if no shape with the same index already exists.

    If the index of the new shape is -1 (default), the next free auto-incremented index will be set for this shape. (C++ only: this new index is
    stored in the optional 'newIndex' parameter).

    :param geometricShape: new geometric shape
    :type geometricShape: shape

    :raises RuntimeError: if the index of the shape is != -1 and does already exist


.. py:function:: setGeometricShapes(geometricShapes) [slot]
    :noindex:


    This slot is the same than assigning a sequence of shape to the property 'geometricShapes'. It replaces all existing shapes by the new set of shapes.

    :param geometricShapes: Sequence (e.g tuple or list) of shapes that replace all existing shapes by this new set.
    :type geometricShapes: seq. of shapes


.. py:function:: deleteGeometricShape(idx) [slot]
    :noindex:


    deletes the geometric shape with the given index.

    :param idx: idx is the index of the shape to be removed. This is the index of the shape instance itself and must not always correspond to the index-position of the shape within the tuple of all shapes
    :type idx: int


.. py:function:: clearGeometricShapes() [slot]
    :noindex:


    removes all geometric shapes from the canvas.


.. py:function:: userInteractionStart(type, start, maxNrOfPoints) [slot]
    :noindex:


    starts or aborts the process to let the user add a certain number of geometric shapes to the canvas.

    :param type: type of the geometric shape the user should add (e.g. shape.Line, shape.Point, shape.Rectangle, shape.Square...
    :type type: int
    :param start: True if the interaction should be started, False if a running interaction process should be aborted
    :type start: bool
    :param maxNrOfPoints: number of shapes that should be added, the user can quit earlier by pressing Esc (optional, default: -1 -> infinite number of shapes)
    :type maxNrOfPoints: int


.. py:function:: renderToPixMap(xsize, ysize, resolution) [slot]
    :noindex:


    returns a QPixmap with the content of the plot

    :param xsize: width of the pixmap
    :type xsize: int
    :param ysize: height of the pixmap
    :type ysize: int
    :param resolution: resolution of the pixmap in dpi
    :type resolution: int


.. py:function:: savePlot(filename, xsize, ysize, resolution) [slot]
    :noindex:


    saves the plot as image, pdf or svg file (the supported file formats are listed in the save dialog of the plot)

    :param filename: absolute or relative filename whose suffix defines the file format
    :type filename: str
    :param xsize: x-size of the canvas in mm. If 0.0 [default], the size of the canvas is determined by the current size of the figure
    :type xsize: float
    :param ysize: y-size of the canvas in mm. If 0.0 [default], the size of the canvas is determined by the current size of the figure
    :type ysize: float
    :param resolution: resolution of image components in the plot in dpi (default: 300dpi)
    :type resolution: int


.. py:function:: copyToClipBoard() [slot]
    :noindex:


    copies the entire plot to the clipboard as bitmap data (uses the default export resolution).


.. py:function:: setLinePlot(x0, y0, x1, y1, destID) [slot]
    :noindex:


    This slot can be implemented by any plot plugin to force the plot to open a line plot. Here it is not required and therefore not implemented.


.. py:function:: refreshPlot() [slot]
    :noindex:


    Triggers an update of the current plot window.


.. py:function:: getPlotID() [slot]
    :noindex:


    Return window ID of this plot {int}.


Signals
-------------------------


.. py:function:: pickerChanged(pickerIndex, positionX, positionY, curveIndex) [signal]
    :noindex:


    This signal is emitted whenever the current picker changed its position

    :param pickerIndex: index of the changed picker
    :type pickerIndex: int
    :param positionX: horizontal position of currently changed picker
    :type positionX: double
    :param positionY: vertical position of the currently changed picker
    :type positionY: double
    :param curveIndex: index of the curve the picker is attached to
    :type curveIndex: int

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('pickerChanged(int,double,double,int)', yourMethod)


.. py:function:: geometricShapeCurrentChanged(currentShape) [signal]
    :noindex:


    This signal is emitted whenever the currently selected geometric has been changed

    :param currentShape: new current shape or an invalid shape if the current shape has been deleted and no other shape is selected now
    :type currentShape: shape

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapeCurrentChanged(ito::Shape)', yourMethod)


.. py:function:: geometricShapeFinished(shapes, aborted) [signal]
    :noindex:


    This signal is emitted whenever one or multiple geometric shapes have been added, removed or modified

    :param shapes: A tuple containing all shapes that have been modified
    :type shapes: tuple of shape
    :param aborted: True if the modification process has been aborted, else False
    :type aborted: bool

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapeFinished(QVector<ito::Shape>,bool)', yourMethod)


.. py:function:: geometricShapesDeleted() [signal]
    :noindex:


    This signal is emitted when the last geometric shape has been deleted or removed.

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapesDeleted()', yourMethod)


.. py:function:: geometricShapeDeleted(idx) [signal]
    :noindex:


    This signal is emitted whenever a geometric shape has been deleted

    :param idx: index of the deleted shape
    :type idx: int

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapeDeleted(int)', yourMethod)


.. py:function:: geometricShapeChanged(idx, shape) [signal]
    :noindex:


    This signal is emitted whenever a geometric shape has been changed (e.g. its position or form has been changed)

    :param idx: index of the changed shape (this is the index of the second parameter 'shape')
    :type idx: int
    :param shape: shape that has been changed
    :type shape: shape

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapeChanged(int,ito::Shape)', yourMethod)


.. py:function:: geometricShapeAdded(idx, shape) [signal]
    :noindex:


    This signal is emitted whenever a geometric shape has been added

    :param idx: index of the new shape (this is the index of the second parameter 'shape')
    :type idx: int
    :param shape: new shape
    :type shape: shape

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapeAdded(int,ito::Shape)', yourMethod)


.. py:function:: userInteractionDone(type, aborted, shapes) [signal]
    :noindex:


    This signal is emitted if the user finished adding the requested number of shapes or aborted the process by pressing the Esc key

    This signal is only emitted if the user interaction has been started by the slot *userInteractionStart* or by plotItem.drawAndPickElements.

    :param type: type of the shapes that have been recently added (e.g. shape.Line, shape.Point, shape.Rectangle, ...)
    :type type: int
    :param aborted: True if the user aborted the process by pressing the Esc key before having added the total number of requested shapesshapes : {list of shape} list of shapes that have been added.
    :type aborted: bool

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('userInteractionDone(int,bool,QVector<ito::Shape>)', yourMethod)


.. py:function:: geometricShapeStartUserInput(type, userInteractionReason) [signal]
    :noindex:


    This signal is emitted whenever the plot enters a mode where the user can add a new geometric shape using the mouse

    :param type: Type of the shape that could be added by the user, this is one of the constants shape.Circle, shape.Ellipse, shape.Line...
    :type type: int
    :param userInteractionReason: True if the process to add a new shape has been initialized by a script-base call, False if it has been started by a button in the toolbar or menu of the plot
    :type userInteractionReason: bool

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('geometricShapeStartUserInput(int,bool)', yourMethod)


.. py:function:: windowTitleModified(windowTitleSuffix) [signal]
    :noindex:


    signature for connection to this signal: windowTitleModified(QString)



.. END plot_help_to_rst_format.py: itom1dqwtplot

Deprecated figures
==========================

The plot-dll "itom1DQWTFigure"  is deprecated and has been replaced by
"Itom1DQwtPlot".
