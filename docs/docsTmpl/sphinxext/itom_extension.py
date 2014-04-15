# -*- coding: utf-8 -*-

from docutils import nodes, utils, statemachine
from docutils.parsers.rst.roles import set_classes
from docutils.parsers.rst import directives
from sphinx.util.compat import Directive
import itom

def getPluginInfo(env, plugin):
    
    if (not hasattr(env,"itom_plugin_infos")):
        env.itom_plugin_infos = {}
    
    p = str(plugin)
    if (not p in env.itom_plugin_infos):
        env.itom_plugin_infos[p] = itom.pluginHelp(p,True)
    
    return env.itom_plugin_infos[p]

def pluginAuthor_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """Author of plugin.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    try:
        pluginInfo = getPluginInfo(inliner.document.settings.env.app.builder.env, text)
    except:
        msg = inliner.reporter.error(
            'Error getting plugin information from plugin "%s".' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    set_classes(options)
    node = nodes.inline(pluginInfo["author"], pluginInfo["author"])
    return [node], []

def pluginSummary_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """Description of plugin.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    try:
        pluginInfo = getPluginInfo(inliner.document.settings.env.app.builder.env, text)
    except:
        msg = inliner.reporter.error(
            'Error getting plugin information from plugin "%s".' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    set_classes(options)
    node = nodes.inline(pluginInfo["description"], pluginInfo["description"])
    
    return [node], []

def pluginLicense_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """License of plugin.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    try:
        pluginInfo = getPluginInfo(inliner.document.settings.env.app.builder.env, text)
    except:
        msg = inliner.reporter.error(
            'Error getting plugin information from plugin "%s".' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    set_classes(options)
    node = nodes.inline(pluginInfo["license"], pluginInfo["license"])
    
    return [node], []

def pluginType_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """Type of plugin.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    try:
        pluginInfo = getPluginInfo(inliner.document.settings.env.app.builder.env, text)
    except:
        msg = inliner.reporter.error(
            'Error getting plugin information from plugin "%s".' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    set_classes(options)
    node = nodes.inline(pluginInfo["type"], pluginInfo["type"])
    
    return [node], []

def pluginVersion_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """Version of plugin.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    try:
        pluginInfo = getPluginInfo(inliner.document.settings.env.app.builder.env, text)
    except:
        msg = inliner.reporter.error(
            'Error getting plugin information from plugin "%s".' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    set_classes(options)
    node = nodes.inline(pluginInfo["version"], pluginInfo["version"])
    
    return [node], []

class PluginSummaryExtended(Directive):
    """display detailed description from plugin (the description is inserted into the loaded rst-code such that
    it is also parsed using the rst-parser."""
    has_content = False
    required_arguments = 0
    optional_arguments = 1
    option_spec = {"plugin":directives.unchanged_required}

    def run(self):
        tab_width = self.options.get('tab-width', self.state.document.settings.tab_width)
        source = self.state_machine.input_lines.source(self.lineno - self.state_machine.input_offset - 1)
        
        if (not "plugin" in self.options):
            return [self.state.document.reporter.warning('option "plugin" is missing', line=self.lineno)]
        
        try:
            pluginInfo = getPluginInfo(self.state.document.settings.env, self.options["plugin"])
        except Exception:
            return [self.state.document.reporter.warning('Error getting plugin information from plugin "%s"' % (self.options["plugin"]), line=self.lineno)]

        text = pluginInfo["detaildescription"]
        lines = statemachine.string2lines(text, tab_width, convert_whitespace=True)
        self.state_machine.insert_input(lines, source)
        return []

class PluginInitParams(Directive):
    """puts the list with all mandatory and optional parameters for initializing the dataIO or actuator
    plugin into the rst-code for further parsing"""
    has_content = False
    required_arguments = 0
    optional_arguments = 1
    option_spec = {"plugin":directives.unchanged_required}

    def run(self):
        tab_width = self.options.get('tab-width', self.state.document.settings.tab_width)
        source = self.state_machine.input_lines.source(self.lineno - self.state_machine.input_offset - 1)
        
        if (not "plugin" in self.options):
            return [self.state.document.reporter.warning('option "plugin" is missing', line=self.lineno)]
        
        try:
            pluginInfo = getPluginInfo(self.state.document.settings.env, self.options["plugin"])
        except Exception:
            return [self.state.document.reporter.warning('Error getting plugin information from plugin "%s"' % (self.options["plugin"]), line=self.lineno)]
        
        textlist = ["\n"]
        
        if ("Mandatory Parameters" in pluginInfo):
            for p in pluginInfo["Mandatory Parameters"]:
                text = "* **%s**: {%s} %s\n    %s" % (p["name"],p["type"],self.parseContent(p),p["info"])
                textlist .append(text)
        
        if ("Optional Parameters" in pluginInfo):
            for p in pluginInfo["Optional Parameters"]:
                text = "* **%s**: {%s, optional} %s\n    %s" % (p["name"],p["type"],self.parseContent(p),p["info"])
                textlist .append(text)
        
        textlist.append("\n")
        text = "\n".join(textlist)
        lines = statemachine.string2lines(text, tab_width, convert_whitespace=True)
        self.state_machine.insert_input(lines, source)
        return []
    
    def parseContent(self,param):
        if ("min" in param and "max" in param):
            if ("step" in param and not (param["step"] is None)):
                content = "[%s,%s], default: %s, step: %s" % (param["min"],param["max"],param["value"],param["step"])
            else:
                content = "[%s,%s], default: %s" % (param["min"],param["max"],param["value"])
        elif ("value" in param):
            content = "default: %s" % param["value"]
        else:
            content = ""
        return content

class PluginFilterList(Directive):
    """."""
    has_content = False
    required_arguments = 0
    optional_arguments = 1
    option_spec = {"plugin":directives.unchanged_required, "overviewonly":directives.flag}

    def run(self):
        tab_width = self.options.get('tab-width', self.state.document.settings.tab_width)
        source = self.state_machine.input_lines.source(self.lineno - self.state_machine.input_offset - 1)
        
        if (not "plugin" in self.options):
            return [self.state.document.reporter.warning('option "plugin" is missing', line=self.lineno)]
        
        try:
            pluginInfo = getPluginInfo(self.state.document.settings.env, self.options["plugin"])
        except Exception:
            return [self.state.document.reporter.warning('Error getting plugin information from plugin "%s"' % (self.options["plugin"]), line=self.lineno)]
        
        if (not "filter" in pluginInfo):
            return [self.state.document.reporter.warning('given plugin is no algorithm plugin', line=self.lineno)]
        
        textlist = []
        if (pluginInfo["filter"] is None):
            textlist.append("The plugin does not contain any filters")
        else:
            if ("overviewonly" in self.options):
                for f in pluginInfo["filter"]:
                    textlist.append("#. %s" % f)
            else:
                for f in pluginInfo["filter"]:
                    t = ".. py:function:: %s(...)" % f
                    t += "\n    \n    ...\n"
                    textlist.append(t)
        
        text = "\n".join(textlist)
        lines = statemachine.string2lines(text, tab_width, convert_whitespace=True)
        self.state_machine.insert_input(lines, source)
        return []
    


def setup(app):
    """Install the plugin.

    :param app: Sphinx application context.
    """
    app.add_role('pluginauthor', pluginAuthor_role)
    app.add_role('pluginsummary', pluginSummary_role)
    app.add_role('pluginlicense', pluginLicense_role)
    #app.add_role('pluginsummaryextended', pluginSummary2_role)
    app.add_role('plugintype', pluginType_role)
    app.add_role('pluginversion', pluginVersion_role)
    
    app.add_directive('pluginsummaryextended', PluginSummaryExtended)
    app.add_directive('plugininitparams', PluginInitParams)
    app.add_directive('pluginfilterlist', PluginFilterList)

