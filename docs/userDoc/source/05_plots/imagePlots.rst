.. _plot-image:

image plots (2D)
*****************

"Itom2dQwtPlot" and "GraphicViewPlot" are the basic plots for visualization of images, dataObjects or other array-like objects.
Both plots have a line-cut and point picker included. By pressing "Ctrl" during picker movement, the picker can only be moved
horizontal or vertical according to the mouse movement.

You can also use the "matplotlib"-backend to plot any data structures (lines, bars, statistical plots, images, contours, 3d plots...).
See section :ref:`pymod-matplotlib` for more information about how to use "matplotlib".

.. _itom2dqwtplot:

Itom2dQwtPlot
==========================

"Itom2dQwtPlot" is designed for visualizing metrical data, false color or topography measurements.
It supports the axis-scaling / axis offset of **dataObjects**, offers axis-tags and meta-data handling.
All data types are accepted except the plotting of real color objects (rgba). To plot complex objects, it is possible to choose
between the following modes: "absolute", "phase", "real" and "imaginary". The data is plotted mathematically correct. This means
the value at [0,0] is in the lower left position. This can be changed by the property *yAxisFlipped*.

The plot supports geometric element and marker interaction via **drawAndPickElements(...)** and **call("userInteractionStart",...)**.
See section :ref:`primitives` for a short introduction.

**Features:**

* Export graphics to images, pdf, vector graphics (via button) or to the clipboard (ctrl-c).
* Metadata support (the 'title'-tag is used as title of the plot).
* Supports fixed ratio x/y-axis but not necessary fixed ratio to monitor-pixel
* Drawing of geometrical elements and markers by script and user interaction.
* Images are displayed either mathematically ([0,0] lower left) or in windows-style ([0,0] upper left) (Property: 'yAxisFlipped')

**Available colorMaps:**

+-----------------------------------------------------+---------------------------------------------------+---------------------------------------------------+
| .. figure:: images/plot2d_colorMap_gray.png         | .. figure:: images/plot2d_colorMap_grayMarked.png | .. figure:: images/plot2d_colorMap_falseColor.png |
+-----------------------------------------------------+---------------------------------------------------+---------------------------------------------------+
| .. figure:: images/plot2d_colorMap_falseColorIR.png | .. figure:: images/plot2d_colorMap_HotIron.png    | .. figure:: images/plot2d_colorMap_red.png        |
+-----------------------------------------------------+---------------------------------------------------+---------------------------------------------------+
| .. figure:: images/plot2d_colorMap_blue.png         | .. figure:: images/plot2d_colorMap_green.png      | .. figure:: images/plot2d_colorMap_viridis.png    |
+-----------------------------------------------------+---------------------------------------------------+---------------------------------------------------+

This example shows how the colorMap is set with the **itom** :py:meth:`~itom.plot` or the **itom** :py:meth:`~itom.plot2` command:

.. code-block:: python

    plot(dataObject, properties ={"colorMap" : "viridis"})
    plot2(dataObject, properties ={"colorMap" : "viridis"})

.. _itom2dqwtplot-ref:

.. BEGIN The following part is obtained by the script plot_help_to_rst_format.py (source/itom/docs/userDoc) from the designer plugin itom2dqwtplot
..       Before starting this script, disable the long-line wrap mode in the console / line wrap property page of itom.

Properties
-------------------------


.. py:attribute:: volumeCutPlotItem : uiItem
    :noindex:

    Set/get the uiItem of the current line plot respective the destination line plot for the volume cut. The 'uiItem' can be safely cast to 'plotItem'.

.. py:attribute:: zSlicePlotItem : uiItem
    :noindex:

    Set/get the uiItem of the current line plot respective the destination line plot for z slicing. The 'uiItem' can be safely cast to 'plotItem'.

.. py:attribute:: lineCutPlotItem : uiItem
    :noindex:

    Set/get the uiItem of the current line plot respective the destination line plot for lateral slicing. The 'uiItem' can be safely cast to 'plotItem'.

.. py:attribute:: dataChannel : ItomQwtPlotEnums::DataChannel
    :noindex:

    Type of visualized dataChannel. This is only considered for rgba32 dataObjects, in all other cases this property is ignored.

    The type 'ItomQwtPlotEnums::DataChannel' is an enumeration that can have one of the following values (str or int):

    * 'ChannelAuto' (0)
    * 'ChannelRGBA' (1)
    * 'ChannelGray' (256)
    * 'ChannelRed' (257)
    * 'ChannelGreen' (258)
    * 'ChannelBlue' (260)
    * 'ChannelAlpha' (264)

.. py:attribute:: planeIndex : int
    :noindex:

    Plane index of currently visible plane.

.. py:attribute:: bounds :
    :noindex:



.. py:attribute:: lineCutData : dataObject (readonly)
    :noindex:

    Get the currently displayed slices from the child lineplot

.. py:attribute:: contourLineWidth : float
    :noindex:

    Defines the line width of the contour lines

.. py:attribute:: contourColorMap : str
    :noindex:

    Defines which color map should be used for the contour lines [e.g. gray, grayMarked, falseColor, falseColorIR, hotIron, red, blue, green, viridis].

.. py:attribute:: contourLevels : dataObject
    :noindex:

    Defines which contour levels should be plotted. Each value inside the given dataObject results in one contour level. Possible types are uint8, int8, uint16, int16, int32, float32 and float64.

.. py:attribute:: overlayColorMap : str
    :noindex:

    Defines which color map should be used for the overlay channel [e.g. gray, grayMarked, falseColor, falseColorIR, hotIron, red, blue, green, viridis].

.. py:attribute:: overlayInterval : autoInterval
    :noindex:

    Range of the overlayInterval to scale the values

.. py:attribute:: overlayAlpha : int
    :noindex:

    Changes the value of the overlay channel

.. py:attribute:: overlayImage : dataObject
    :noindex:

    Set an overlay dataObject which is shown above the main dataObject and whose opacity (see 'overlayAlpha') can be controlled by a slider in the toolbar. Assign None to remove the overlay object.

.. py:attribute:: showCenterMarker : bool
    :noindex:

    Shows or hides a marker for the center of a data object.

.. py:attribute:: axisFont : font
    :noindex:

    Font for axes tick values.

.. py:attribute:: labelFont : font
    :noindex:

    Font for axes descriptions.

.. py:attribute:: titleFont : font
    :noindex:

    Font for title.

.. py:attribute:: colorMap : str
    :noindex:

    Defines which color map should be used [e.g. grayMarked, hotIron].

.. py:attribute:: colorBarVisible : bool
    :noindex:

    Defines whether the color bar should be visible.

.. py:attribute:: valueScale : ItomQwtPlotEnums::ScaleEngine
    :noindex:

    linear or logarithmic scale (various bases) can be chosen for the value axis (color bar). Please consider, that a logarithmic scale can only display values > 1e-100 while the lower limit for the double-logarithmic scale is 1+1e-100.

    The type 'ItomQwtPlotEnums::ScaleEngine' is an enumeration that can have one of the following values (str or int):

    * 'Linear' (1)
    * 'Log2' (2)
    * 'Log10' (10)
    * 'Log16' (16)
    * 'LogLog2' (1002)
    * 'LogLog10' (1010)
    * 'LogLog16' (1016)

.. py:attribute:: valueLabel : str
    :noindex:

    Label of the value axis or '<auto>' if the description should be used from data object.

.. py:attribute:: yAxisFlipped : bool
    :noindex:

    Sets whether y-axis should be flipped (default: false, zero is at the bottom).

.. py:attribute:: yAxisVisible : bool
    :noindex:

    Sets visibility of the y-axis.

.. py:attribute:: yAxisLabel : str
    :noindex:

    Label of the y-axis or '<auto>' if the description from the data object should be used.

.. py:attribute:: xAxisVisible : bool
    :noindex:

    Sets visibility of the x-axis.

.. py:attribute:: xAxisLabel : str
    :noindex:

    Label of the x-axis or '<auto>' if the description from the data object should be used.

.. py:attribute:: title : str
    :noindex:

    Title of the plot or '<auto>' if the title of the data object should be used.

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

.. py:attribute:: allowCameraParameterEditor : bool
    :noindex:

    If a live camera is connected to this plot, a camera parameter editor can be displayed as
    toolbox of the plot. If this property is false, this toolbox is not available (default: true)

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


.. py:function:: removeOverlayImage() [slot]
    :noindex:


    removes an overlay image. This is the same than assigning 'None' to the property 'overlayImage'


.. py:function:: setLinePlot(x0, y0, x1, y1, destID) [slot]
    :noindex:


    displays a line cut plot with the given bounds.

    :param x0: x-coordinate (physical units) of the first end point of the line cut.
    :type x0: int
    :param y0: y-coordinate (physical units) of the first end point of the line cut.
    :type y0: int
    :param x1: x-coordinate (physical units) of the first end point of the line cut.
    :type x1: int
    :param y1: y-coordinate (physical units) of the second end point of the line cut.
    :type y1: int
    :param destID: optional and unused
    :type destID: int


.. py:function:: getDisplayedLineCut() [slot]
    :noindex:


    returns the currently displayed line cut dataObject


.. py:function:: getDisplayed() [slot]
    :noindex:


    returns the currently displayed dataObject.


.. py:function:: replot() [slot]
    :noindex:


    Force a replot which is for instance necessary if values of the displayed data object changed and you want to update the plot, too.


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


.. py:function:: refreshPlot() [slot]
    :noindex:


    Triggers an update of the current plot window.


.. py:function:: getPlotID() [slot]
    :noindex:


    Return window ID of this plot {int}.


Signals
-------------------------


.. py:function:: planeIndexChanged(planeIndex) [signal]
    :noindex:


    This signal is emitted whenever the displayed plane in a 3D dataObject is changed

    :param plane: index of the displayed plane in the dataObject
    :type plane: int

    .. note::

        To connect to this signal use the following signature::

            yourItem.connect('planeIndexChanged(int)', yourMethod)


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


.. END plot_help_to_rst_format.py: itom2dqwtplot

Deprecated figures
==========================

The plot-dll "itom2DQWTFigure" and "itom2DGVFigure" are deprecated and have been replaced by  "Itom2dQwtPlot" and "GraphicViewPlot".
