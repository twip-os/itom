# coding=iso-8859-15

#DELETED:# -*- coding: utf-8 -*-
#
# itom documentation build configuration file
#
# This file is execfile()d with the current directory set to its containing dir.
#<
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import sys, os
import sphinx
import itom as itomFuncs
import __main__
import inspect
import quark_sphinx_theme

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#sys.path.insert(0, os.path.abspath('.'))
sys.path.append(os.path.abspath('sphinxext'))
sys.path.append(os.path.abspath('ext'))

thisDir = os.path.dirname( os.path.abspath (inspect.getfile(inspect.currentframe())))

# -- General configuration -----------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be extensions
# coming with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = ['sphinx.ext.autodoc', 'sphinx.ext.todo', 'sphinx.ext.pngmath', 'sphinx.ext.ifconfig', 'numpydoc', 'breathe', 'sphinx.ext.intersphinx', 'itomext.designerplugindoc']
#extensions = ['sphinx.ext.autodoc', 'sphinx.ext.todo', 'sphinx.ext.pngmath', 'sphinx.ext.ifconfig', 'breathe', 'sphinx.ext.intersphinx']

if sphinx.__version__ >= "0.7":
    extensions.append('sphinx.ext.autosummary')
else:
    extensions.append('autosummary')
    extensions.append('only_directives')



# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix of source filenames.
source_suffix = '.rst'

# The encoding of source files.
source_encoding = 'iso-8859-15' #'iso-8859-15' #'utf-8-sig'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = 'itom Documentation'
copyright = '2011-2017, Institut fuer Technische Optik (ITO), University Stuttgart. Bug report: https://bitbucket.org/itom/itom/issues'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = itomFuncs.version(1)["itom"]["itom_Version"]
# The full version, including alpha/beta/rc tags. / Instead of release we will use the SNV-Revision
d = itomFuncs.version(1)
if("itom_SVN_Rev" in d["itom"] and d["itom"]["itom_SVN_Rev"] != ""):
    release = d["itom"]["itom_SVN_Rev"]
elif("itom_GIT_Rev_Abbrev" in d["itom"] and d["itom"]["itom_GIT_Rev_Abbrev"] != ""):
    release = d["itom"]["itom_GIT_Rev_Abbrev"]
else:
    release = "unknown"

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#language = None

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
#today = ''
# Else, today_fmt is used as the format for a strftime call.
today_fmt = '%B %d, %Y'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = []

# The reST default role (used for this markup: `text`) to use for all documents.
#default_role = None
default_role = "autolink"

# If true, '()' will be appended to :func: etc. cross-reference text.
#add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
#add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# A list of ignored prefixes for module index sorting.
#modindex_common_prefix = []

# Show both class-level docstring and __init__ docstring in class
# documentation
autoclass_content = 'both'
autodoc_member_order = 'groupwise'
autodoc_docstring_signature = True


# -- Options for HTML output ---------------------------------------------------

#get Qt version. Qt 5.6.0 only has a reduced subset of css commands in the assistant.
#Therefore, a reduced style theme has to be used (theme quark_spinx_theme from https://bitbucket.org/fk/quark-sphinx-theme)
#This theme was locally copied to the itom-packages folder.
qt_version = itomFuncs.version(True)["itom"]["QT_Version"]

#choose itom theme as main theme
html_theme = 'itom'
qthelp_theme = 'itom' #theme for QtHelp only (available since Sphinx 1.5.3)
html_theme_path = ['_themes']

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#html_theme = 'default'
#html_theme = 'agogo'
#html_theme = 'sphinxdoc'
#html_theme = 'haiku'
#html_theme = 'itom'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#html_theme_options = {}

# Add any paths that contain custom themes here, relative to this directory.
#html_theme_path = ['_themes']

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
html_title = 'itom Documentation'

# A shorter title for the navigation bar.  Default is the same as html_title.
html_short_title = 'itom'

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
#html_logo = None

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
#html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = [] #['_static']

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
html_last_updated_fmt = '%b %d, %Y'

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
#html_sidebars = {}

# Additional templates that should be rendered to pages, maps page names to
# template names.
#html_additional_pages = {}

# If false, no module index is generated.
#html_domain_indices = True

# If false, no index is generated.
html_use_index = True

# If true, the index is split into individual pages for each letter.
#html_split_index = False

# If true, links to the reST sources are added to the pages.
#html_show_sourcelink = True

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
#html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
#html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
#html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
#html_file_suffix = None

# Output file base name for HTML help builder.
htmlhelp_basename = 'itom_doc'


# -- Options for LaTeX output --------------------------------------------------

# The paper size ('letter' or 'a4').
latex_paper_size = 'a4'

# The font size ('10pt', '11pt' or '12pt').
#latex_font_size = '10pt'

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title, author, documentclass [howto/manual]).

latex_documents = [
  # Gesam: FEHLER
  ('index', 'itom_doc.tex', u'itom Documentation', u'Institut fuer Technische Optik (ITO), University Stuttgart', 'manual'),
  
  # script-language: FEHLER
  #('script-language/script-language', 'itom_scriptLanguage.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # Reference: FEHLER
  #('reference/reference', 'itom_reference.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # Installation: viele FEHLER
  #('02_installation/install', 'install.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # AboutItom: OK
  #('AboutItom/aboutItom','AbouItom.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'), 

  # getting-Started: OK
  #('getting-started/getting-started', 'getting-started.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # gui: OK
  #('gui/gui', 'gui.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # miscellaneous: OK
  #('miscellaneous/miscellaneous', 'miscellaneous.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # test-scripts: OK
  #('test-scripts/test-scripts', 'test-scripts.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
  # whats-new: OK (leer)
  #('whats-new/whats-new', 'whats-new.tex', u'itom Documentation', u'Institut for Technical Optics (ITO), University Stuttgart', 'manual'),
  
]

#latex_documents = [
#  ('index', 'itom_doc.tex', u'ITOM Documentation', u'Institut fuer Technische Optik (ITO), Universitaet Stuttgart', 'manual'),
#  ('plugins/plugins', 'plugins.tex', u'ITOM Documentation', u'Institut fuer Technische Optik (ITO), Universitaet Stuttgart', 'manual'),
#  ('getting-started/getting-started', 'getting-started.tex', u'ITOM Documentation', u'Institut für Technische Optik (ITO), Universitaet Stuttgart', 'manual'),
#]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
latex_logo = 'ITO_Logo.pdf'

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
#latex_use_parts = False

# If true, show page references after internal links.
#latex_show_pagerefs = False

# If true, show URL addresses after external links.
#latex_show_urls = False

# Additional stuff for the LaTeX preamble.
#latex_preamble = ''

# Documents to append as an appendix to all manuals.
#latex_appendices = []

# If false, no module index is generated.
#latex_domain_indices = True


# -- Options for manual page output --------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('index', 'itom', 'itom Documentation',
     ['Institut fuer Technische Optik (ITO), Universitaet Stuttgart'], 1)
]

# If true, show URL addresses after external links.
#man_show_urls = False


# -- Options for Texinfo output ------------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
  ('index', 'itom', 'itom Documentation',
   'ITO, Universität Stuttgart', 'itom', 'One line description of project.',
   'Miscellaneous'),
]

# Documents to append as an appendix to all manuals.
#texinfo_appendices = []

# If false, no module index is generated.
#texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
#texinfo_show_urls = 'footnote'



# -----------------------------------------------------------------------------
# Autosummary
# -----------------------------------------------------------------------------
autosummary_generate = True

# -----------------------------------------------------------------------------
# Breathe
# -----------------------------------------------------------------------------
doxygenXML_folder = __main__.__dict__["__doxygen__xml"]
breathe_projects = {"itom": doxygenXML_folder} #os.path.abspath(".\\..\\..\\xml")}
breathe_default_project = "itom"

# -----------------------------------------------------------------------------
# Todo
# -----------------------------------------------------------------------------
todo_include_todos = True


intersphinx_mapping = {'python': ('http://docs.python.org/3.5', None),
                       'numpy': ('http://docs.scipy.org/doc/numpy', None),
                       'scipy': ('http://docs.scipy.org/doc/scipy/reference', None)}
