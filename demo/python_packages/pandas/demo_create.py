"""Create and view an object
============================

"""
import pandas as pd
import numpy as np

# sphinx_gallery_thumbnail_path = '11_demos/_static/_thumb/demoPandas.png'

###############################################################################
# **Create an object**
series = pd.Series([1, 3, 5, np.nan, 6, 8])

###############################################################################
dates = pd.date_range("20220501", periods=6)

###############################################################################
dataFrame = pd.DataFrame(np.random.randn(6, 4), index=dates, columns=list("ABCD"))

###############################################################################
dataFrame2 = pd.DataFrame(
    {
        "A": 1.0,
        "B": pd.Timestamp("20220501"),
        "C": pd.Series(1, index=list(range(4)), dtype="float32"),
        "D": np.array([3] * 4, dtype="int32"),
        "E": pd.Categorical(["test", "train", "test", "train"]),
        "F": "foo",
    }
)

###############################################################################
# **View an object**
dataFrame2.head()

###############################################################################
dataFrame2.tail()

###############################################################################
dataFrame2.dtypes

###############################################################################
dataFrame2.index

###############################################################################
dataFrame2.columns

###############################################################################
dataFrame2.describe()

###############################################################################
# Convert to numpy
dataFrame.to_numpy()

###############################################################################
# **Transpose, sorting data**
dataFrame.T

###############################################################################
dataFrame.sort_index(axis=1, ascending=False)

###############################################################################
dataFrame.sort_values(by="B")