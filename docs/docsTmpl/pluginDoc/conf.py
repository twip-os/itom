#
# plugin documentation build configuration file for use with itom.
#
# This file is execfile()d with the current directory set to its containing dir.
# <
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import sys
import os
import sphinx
import itom as itomFuncs
import __main__
import inspect

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
# sys.path.insert(0, os.path.abspath('.'))

# directory of this file
thisDir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))

sys.path.append(os.path.join(thisDir, ".." + os.sep + "sphinxext"))

# -- General configuration -----------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be extensions
# coming with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.todo",
    "sphinx.ext.ifconfig",
    "numpydoc",
    "sphinx.ext.intersphinx",
    "itom_extension",
]

pymajor = sys.version_info.major
pyminor = sys.version_info.minor

if pymajor > 3 or (pymajor == 3 and pyminor > 5):
    extensions.append("sphinx_copybutton")

if sphinx.__version__ >= "0.7":
    extensions.append("sphinx.ext.autosummary")
else:
    extensions.append("autosummary")
    extensions.append("only_directives")

if sphinx.__version__ >= "1.4":
    extensions.append("sphinx.ext.mathjax")

# Add any paths that contain templates here, relative to this directory.
templates_path = []  # ['_templates']

# The suffix of source filenames.
source_suffix = ".rst"

# The encoding of source files.
source_encoding = "utf-8"

# The master toctree document.
master_doc = "index"  # will be overwritten

# General information about the project.
project = ""  # will be overwritten
copyright = ""  # will be overwritten

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = ""  # will be overwritten
release = ""  # will be overwritten

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
# language = None

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
# today = ''
# Else, today_fmt is used as the format for a strftime call.
today_fmt = "%B %d, %Y"

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = []

# The reST default role (used for this markup: `text`) to use for all documents.
# default_role = None
default_role = "autolink"

# If true, '()' will be appended to :func: etc. cross-reference text.
# add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
# add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
show_authors = True

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = "sphinx"

# A list of ignored prefixes for module index sorting.
# modindex_common_prefix = []

# Show both class-level docstring and __init__ docstring in class
# documentation
autoclass_content = "both"
autodoc_member_order = "groupwise"
autodoc_docstring_signature = True


# -- Options for HTML output ---------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
# html_theme = 'default'
# html_theme = 'agogo'
# html_theme = 'sphinxdoc'
# html_theme = 'haiku'
# html_theme = 'itom'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
# html_theme_options = {}

# Add any paths that contain custom themes here, relative to this directory.
qt_version = itomFuncs.version(True)["itom"]["QT_Version"]

html_theme = "itom"
qthelp_theme = "itom"  # theme for QtHelp only (available since Sphinx 1.5.3)
html_theme_path = [".." + os.sep + "_themes"]

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
html_title = "itom Plugin Documentation"

# A shorter title for the navigation bar.  Default is the same as html_title.
html_short_title = "itom"

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
# html_logo = None

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
# html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = []  # ['_static']

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
html_last_updated_fmt = "%b %d, %Y"

if sphinx.__version__ < "1.6":
    # this option is deprecated from sphinx 1.6 on and can now be set by docutils.conf file (not done in our case)
    # If true, SmartyPants will be used to convert quotes and dashes to
    # typographically correct entities.
    html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
# html_sidebars = {}

# Additional templates that should be rendered to pages, maps page names to
# template names.
# html_additional_pages = {}

# If false, no module index is generated.
html_domain_indices = False

# If false, no index is generated.
html_use_index = False

# If true, the index is split into individual pages for each letter.
# html_split_index = False

# If true, links to the reST sources are added to the pages.
html_show_sourcelink = False

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
# html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
# html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
# html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
# html_file_suffix = None

# Output file base name for HTML help builder.
htmlhelp_basename = "itom_doc"


# -- Options for LaTeX output --------------------------------------------------

# The paper size ('letter' or 'a4').
# latex_paper_size = 'a4'

# The font size ('10pt', '11pt' or '12pt').
# latex_font_size = '10pt'

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title, author, documentclass [howto/manual]).

# latex_documents = []

# The name of an image file (relative to this directory) to place at the top of
# the title page.
# latex_logo = ''

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
# latex_use_parts = False

# If true, show page references after internal links.
# latex_show_pagerefs = False

# If true, show URL addresses after external links.
# latex_show_urls = False

# Additional stuff for the LaTeX preamble.
# latex_preamble = ''

# Documents to append as an appendix to all manuals.
# latex_appendices = []

# If false, no module index is generated.
# latex_domain_indices = True


# -- Options for manual page output --------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
# man_pages = []

# If true, show URL addresses after external links.
# man_show_urls = False


# -- Options for Texinfo output ------------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
# texinfo_documents = []

# Documents to append as an appendix to all manuals.
# texinfo_appendices = []

# If false, no module index is generated.
# texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
# texinfo_show_urls = 'footnote'

# -----------------------------------------------------------------------------
# Warnings
# -----------------------------------------------------------------------------
suppress_warnings = ["app.add_directive", "app.add_role", "app.add_node"]


# -----------------------------------------------------------------------------
# Autosummary
# -----------------------------------------------------------------------------
autosummary_generate = True

# -----------------------------------------------------------------------------
# Autodoc
# -----------------------------------------------------------------------------
autoclass_content = "both"
autodoc_member_order = "groupwise"
autodoc_docstring_signature = True

# -----------------------------------------------------------------------------
# Numpydoc
# -----------------------------------------------------------------------------
numpydoc_class_members_toctree = False
numpydoc_xref_param_type = True
numpydoc_show_class_members = False

# -----------------------------------------------------------------------------
# Todo
# -----------------------------------------------------------------------------
# todo_include_todos = True


# Files for auto-generating links in documentation to methods / datatypes etc. of other projects...
# to get the inv files, open the given URL/objects.inv.
intersphinx_mapping = {
    "python": (
        "https://docs.python.org/3",
        (
            None,
            "..\\..\\..\\..\\sources\\itom\\docs\\userDoc\\source\\python3.12-inv.txt",
        ),
    ),
    "numpy": (
        "https://numpy.org/doc/stable",
        (
            None,
            "..\\..\\..\\..\\sources\\itom\\docs\\userDoc\\source\\numpy1.26-inv.txt",
        ),
    ),
    "scipy": (
        "https://docs.scipy.org/doc/scipy",
        (
            None,
            "..\\..\\..\\..\\sources\\itom\\docs\\userDoc\\source\\scipy1.13.0-inv.txt",
        ),
    ),
    "matplotlib": (
        "https://matplotlib.org",
        (
            None,
            "..\\..\\..\\..\\sources\\itom\\docs\\userDoc\\source\\matplotlib3.8.4-inv.txt",
        ),
    ),
    "pandas": (
        "https://pandas.pydata.org/docs/",
        (
            None,
            "..\\..\\..\\..\\sources\\itom\\docs\\userDoc\\source\\pandas2.2.2-inv.txt",
        ),
    ),
}
