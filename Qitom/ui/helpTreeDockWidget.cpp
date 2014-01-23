#include "helpTreeDockWidget.h"

#include <qdebug.h>
#include "../organizer/addInManager.h"
#include <AppManagement.h>
#include <qdesktopservices.h>
#include <qdiriterator.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qsortfilterproxymodel.h>
#include <qstandarditemmodel.h>
#include <qstringlistmodel.h>
#include <qtconcurrentrun.h>
#include <qtextdocument.h>
#include <qtextstream.h>
#include <QThread>
#include <qtimer.h>
#include <qtreeview.h>
#include <stdio.h>

#include "../widgets/helpDockWidget.h"
#include "../models/leafFilterProxyModel.h"
#include "../AppManagement.h"




// on_start
HelpTreeDockWidget::HelpTreeDockWidget(QWidget *parent, ito::AbstractDockWidget *dock, Qt::WFlags flags)
    : QWidget(parent, flags),
    m_historyIndex(-1),
    m_pMainModel(NULL),
    m_dbPath(qApp->applicationDirPath()+"/help"),
    m_pParent(dock)
{
    ui.setupUi(this);

    connect(AppManagement::getMainApplication(), SIGNAL(propertiesChanged()), this, SLOT(propertiesChanged()));

    // Initialize Variables
    m_treeVisible = false;

    connect(&dbLoaderWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(dbLoaderFinished(int)));

    m_pMainFilterModel = new LeafFilterProxyModel(this);
    m_pMainModel = new QStandardItemModel(this);
    m_pMainFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    //Install Eventfilter
    ui.commandLinkButton->setVisible(false);
    //ui.commandLinkButton->installEventFilter(this);
    ui.treeView->installEventFilter(this);
    ui.textBrowser->installEventFilter(this);

    m_previewMovie = new QMovie(":/application/icons/loader32x32trans.gif", QByteArray(), this);
    ui.lblProcessMovie->setMovie( m_previewMovie );
    ui.lblProcessMovie->setVisible(false);
    ui.lblProcessText->setVisible(false);

    loadIni();
    m_forced = true;
    propertiesChanged();
    //reloadDB();

    QStringList iconAliasesName;
    QList<int> iconAliasesNumb;
    iconAliasesName << "class" << "const" << "routine" << "module" << "package" << "unknown" << "link_unknown" << "link_class" << "link_const" << "link_module" << "link_package" << "link_routine";
    iconAliasesNumb << 04      << 06      << 05        << 03       << 02        << 00        << 11             << 14           << 16           << 13            << 12             << 15;
    int i = 0;
    foreach(const QString &icon, iconAliasesName)
    {
        m_iconGallery[iconAliasesNumb[i]] = QIcon(":/helpTreeDockWidget/"+icon);
        i++;
    }
    //ui.textBrowser->setLineWrapMode( QTextEdit::NoWrap );
}

//----------------------------------------------------------------------------------------------------------------------------------
// GUI-on_close
HelpTreeDockWidget::~HelpTreeDockWidget()
{
    saveIni();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Get The Filters and put them into a node of the Tree
void HelpTreeDockWidget::createFilterNode(QStandardItemModel* model)
{
    // Map der Plugin-Namen und Zeiger auf das Node des Plugins
    QMap <QString, QStandardItem*> plugins;

    // AddInManager einbinden
    ito::AddInManager *aim = static_cast<ito::AddInManager*>(AppManagement::getAddInManager());

    // Main Node zusammenbauen
    QStandardItem *mainNode = new QStandardItem("Filter");
    mainNode->setIcon(QIcon(":/helpTreeDockWidget/filter"));

    // Listen aller DLLs und Filter abholen
    const QHash  <QString, ito::AddInAlgo::FilterDef *> *filterHashTable = aim->getFilterList();
    
    // Ueber die Liste itterieren
    QHash<QString , ito::AddInAlgo::FilterDef *>::const_iterator i = filterHashTable->constBegin();
    while (i != filterHashTable->constEnd()) 
    {
        if (!plugins.contains(i.value()->m_pBasePlugin->objectName()))
        {// // Plugin existiert noch nicht, erst das Plugin-Node erstellen um dann das Filter-Node anzuh�ngen
            QStandardItem *plugin = new QStandardItem(i.value()->m_pBasePlugin->objectName());
            plugin->setEditable(false);
            plugin->setIcon(QIcon(":/helpTreeDockWidget/dll"));
            plugin->setToolTip(i.value()->m_pBasePlugin->getFilename()+"; v"+QString::number(i.value()->m_pBasePlugin->getVersion()));
            plugins.insert(i.value()->m_pBasePlugin->objectName(), plugin);
            mainNode->appendRow(plugin);
        }
        // Filter-Node anh�ngen
        QStandardItem *filter = new QStandardItem(i.value()->m_name);
        filter->setEditable(false);
        filter->setIcon(QIcon(":/helpTreeDockWidget/singlefilter"));
        filter->setToolTip(i.value()->m_pBasePlugin->getAuthor());
        QStandardItem *test = plugins[i.value()->m_pBasePlugin->objectName()];
        test->appendRow(filter);
        ++i;
    }
    model->insertRow(0,mainNode);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Filter the events for showing and hiding the treeview
bool HelpTreeDockWidget::eventFilter(QObject *obj, QEvent *event)
{
    // = qobject_cast<ito::AbstractDockWidget*>(parent());

    if (obj == ui.commandLinkButton && event->type() == QEvent::Enter)
    {
        showTreeview();
    }
    else if (obj == ui.treeView && event->type() == QEvent::Enter)
    {    
        if (m_pParent && !m_pParent->isFloating())
        {
            showTreeview();
        }
    }
    else if (obj == ui.textBrowser && event->type() == QEvent::Enter)
    {
        if (m_pParent && !m_pParent->isFloating())
        {
            unshowTreeview();
            return true;
        }    
    }
    return QObject::eventFilter(obj, event);
 }

//----------------------------------------------------------------------------------------------------------------------------------
// Save Gui positions to Main-ini-File
void HelpTreeDockWidget::saveIni()
{
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("HelpScriptReference");
    settings.setValue("percWidthVi", m_percWidthVi);
    settings.setValue("percWidthUn", m_percWidthUn);
    settings.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Load Gui positions to Main-ini-File
void HelpTreeDockWidget::loadIni()
{
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("HelpScriptReference");
    m_percWidthVi = settings.value("percWidthVi", "50").toDouble();
    m_percWidthUn = settings.value("percWidthUn", "50").toDouble();
    settings.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Load SQL-DatabasesList in m_ Variable when properties changed
void HelpTreeDockWidget::propertiesChanged()
{ // Load the new list of DBs with checkstates from the INI-File
    
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("HelpScriptReference");
    // Read the other Options
    m_openLinks = settings.value("OpenExtLinks", true).toBool();
    m_plaintext = settings.value("Plaintext", false).toBool();
    m_showSelection.Filters = settings.value("ShowFilters", true).toBool();
    m_showSelection.Modules = settings.value("ShowModules", true).toBool();

    // if the setting of the loaded DBs has changed:
    // This setting exists only from the time when the property dialog was open till this routine is done!
    if (settings.value("reLoadDBs", false).toBool() | m_forced)
    {
        // Read the List
        m_includedDBs.clear();
        int size = settings.beginReadArray("Databases");
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString dbName = settings.value("DB", QString()).toString();
            if (dbName.startsWith("$"))    
            {// This was checked and will be used
                dbName.remove(0,2);
                //Add to m_pMainlist
                m_includedDBs.append(dbName);
            }
        }
        settings.endArray();
        reloadDB();
    }
    settings.remove("reLoadDBs");
    settings.endGroup();
    m_forced = false;
}

//----------------------------------------------------------------------------------------------------------------------------------
// Build Tree - Bekommt das Model, das zuletzt erstellte Item und eine Liste mit dem Pfad
/*static*/ void HelpTreeDockWidget::createItemRek(QStandardItemModel* model, QStandardItem& parent, const QString parentPath, QList<SqlItem> &items, const QMap<int,QIcon> *iconGallery)
{
    SqlItem firstItem;
    int MyR = Qt::UserRole;

    while( items.count() > 0)
    {
        firstItem = items[0];
        //splitt = firstItem.split(':');

        if(firstItem.prefix == parentPath) //first item is direct child of parent
        {    
            items.removeFirst();
            QStandardItem *node = new QStandardItem(firstItem.name);
            if (firstItem.type > 11) //splitt[0].startsWith(1))
            {
                // diese Zeile k�nnte man auch durch Code ersetzen der das Link Icon automatisch zeichnet... das waere flexibler
                node->setIcon(iconGallery->value(firstItem.type));
            }
            else
            { // Kein Link Normales Bild
                node->setIcon((*iconGallery)[firstItem.type]); //Don't load icons here from file since operations on QPixmap are not allowed in another thread
            }
            node->setEditable(false);
            node->setData(firstItem.path,MyR+1);
            node->setToolTip(firstItem.path);
            createItemRek(model, *node, firstItem.path, items, iconGallery);
            parent.appendRow(node);
        }
        else if(firstItem.prefix.indexOf( parentPath ) == 0) //parentPath is the first part of path
        {
            items.removeFirst();
            int li = firstItem.prefix.lastIndexOf(".");
            QStandardItem *node = new QStandardItem(firstItem.prefix.mid(li+1));
            if (firstItem.type > 11) // Siehe 19 Zeilen vorher
            { //ist ein Link (vielleicht wie oben Icon dynamisch zeichnen lassen
                node->setIcon(iconGallery->value(firstItem.type));
            }
            else
            { // Kein Link Normales Bild
                node->setIcon(iconGallery->value(firstItem.type));
            }
            node->setEditable(false);
            node->setData(firstItem.prefix,MyR+1);                
            createItemRek(model, *node, firstItem.prefix, items, iconGallery);  
            parent.appendRow(node);
        }
        else
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// Get Data from SQL File and store it in a table
/*static*/ ito::RetVal HelpTreeDockWidget::readSQL(/*QList<QSqlDatabase> &DBList,*/ const QString &filter, const QString &file, QList<SqlItem> &items)
{
    ito::RetVal retval = ito::retOk;
    QFile f( file );
    SqlItem item;
  
    if( f.exists() )
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE",file); //important to have variables database and query in local scope such that removeDatabase (outside of this scope) can securly free all resources! -> see docs about removeDatabase
        database.setDatabaseName(file);
        bool ok = database.open();
        if(ok)
        {
            //QSqlQuery query("SELECT type, prefix, prefixL, name FROM itomCTL ORDER BY prefix", database);
            QSqlQuery query("SELECT type, prefix, name FROM itomCTL ORDER BY prefix", database);
            query.exec();
            while (query.next())
            {
                item.type = query.value(0).toInt();
                item.path = query.value(1).toString();
                int li = query.value(1).toString().lastIndexOf(".");
                if(li >= 0)
                {
                    item.prefix = query.value(1).toString().left(li);
                    item.name = query.value(1).toString().mid(li+1);
                }
                else
                {
                    item.prefix = "";
                    item.name = query.value(1).toString();
                }

                items.append(item);
            }
        }
        else
        {
            retval += ito::RetVal::format(ito::retWarning,0,"Database %s could not be opened", file.toAscii().data());
        }
        database.close();
    }
    else
    {
        retval += ito::RetVal::format(ito::retWarning,0,"Database %s could not be found", file.toAscii().data());
    }    
    QSqlDatabase::removeDatabase(file);
    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------
// Reload Database and clear search-edit and start the new Thread
void HelpTreeDockWidget::reloadDB()
{
    //Create and Display Mainmodel
    m_pMainModel->clear();
    ui.treeView->reset();
    
    m_pMainFilterModel->setSourceModel(NULL);
    m_previewMovie->start();
    ui.lblProcessMovie->setVisible(true);
    ui.lblProcessText->setVisible(true);
    ui.treeView->setVisible(false);
    ui.splitter->setVisible(false);
    ui.lblProcessText->setText(tr("Help database is loading..."));


    // THREAD START QtConcurrent::run
    QFuture<ito::RetVal> f1 = QtConcurrent::run(loadDBinThread, m_dbPath, m_includedDBs, m_pMainModel/*, m_pDBList*/, &m_iconGallery, m_showSelection);
    dbLoaderWatcher.setFuture(f1);
    //f1.waitForFinished();
    // THREAD END  
}

//----------------------------------------------------------------------------------------------------------------------------------
void HelpTreeDockWidget::dbLoaderFinished(int /*index*/)
{
    ito::RetVal retval = dbLoaderWatcher.future().resultAt(0);

    m_pMainFilterModel->setSourceModel(m_pMainModel);

    //model has been 
    ui.treeView->setModel(m_pMainFilterModel);

    m_previewMovie->stop();
    ui.lblProcessMovie->setVisible(false);

    if ((m_includedDBs.size() > 0 && m_showSelection.Modules) | m_showSelection.Filters)
    {
        ui.lblProcessText->setVisible(false);
        ui.treeView->setVisible(true);
        ui.splitter->setVisible(true);
    }
    else
    {
        ui.lblProcessText->setVisible(true);
        ui.treeView->setVisible(false);
        ui.splitter->setVisible(false);
        ui.lblProcessText->setText(tr("No help database available"));
    }

    
}

//----------------------------------------------------------------------------------------------------------------------------------
// Load the Database in different Thread
/*static*/ ito::RetVal HelpTreeDockWidget::loadDBinThread(const QString &path, const QStringList &includedDBs, QStandardItemModel *mainModel, const QMap<int,QIcon> *iconGallery, const DisplayBool &show)
{
    QList<SqlItem> sqlList;
    ito::RetVal retval;
    if (show.Modules)
    {
        for (int i = 0; i < includedDBs.length(); i++)
        {
            sqlList.clear();
            QString temp;
            temp = path+'/'+includedDBs.at(i);
            retval = readSQL(/*DBList,*/ "", temp, sqlList);
            QCoreApplication::processEvents();
            if (!retval.containsWarningOrError())
            {
                createItemRek(mainModel, *(mainModel->invisibleRootItem()), "", sqlList, iconGallery);
            }
            else
            {/* The Database named: m_pIncludedDBs[i] is not available anymore!!! show Error*/}
        }
    }
    if (show.Filters)
        createFilterNode(mainModel);
    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------
// Highlight (parse) the Helptext to make it nice and readable for non docutils Docstrings
// ERROR decides whether it�s already formatted by docutils (Error = 0) or it must be parsed by this function (Error != 0)
ito::RetVal HelpTreeDockWidget::highlightContent(const QString &prefix , const QString &name , const QString &param , const QString &shortDesc, const QString &helpText, const QString &error, QTextDocument *document, bool htmlNotPlainText /*= true*/)
{
    QString errorS = error.left(error.indexOf(" ",0));
    int errorCode = errorS.toInt();
    QStringList errorList;

    /*********************************/
    // Allgemeine HTML sachen anf�gen /
    /*********************************/ 
    QString rawContent = helpText;
    QString html = "<html><head>"
                   "<link rel='stylesheet' type='text/css' href='itom_help_style.css'>"
                   "</head><body>%1"
                   "</body></html>";
    // Set Label.Text above Textbrowser
    // -------------------------------------
    if (errorCode == 1)
    { // not needed anymore (as long as the Documentation is right)
        ui.label->setText("Parser: ITO-Parser (Martin)");  
    }
    else if (errorCode == 0)
    {
        ui.label->setText("Parser: docutils");
    }
    else if (errorCode == -1)
    {
        ui.label->setText("Parser: No help-text available");
    }

    // Insert Shortdescription
    // -------------------------------------
    if (shortDesc != "")
        rawContent.insert(0,shortDesc+"");

    // �berschrift (Funktionsname) einfuegen
    // -------------------------------------
    rawContent.insert(0,"<h1 id=\"FunctionName\">"+name+param+"</h1>"+"");

    // Prefix als Navigations-Links einfuegen
    // -------------------------------------
    QStringList splittedLink = prefix.split(".");
    rawContent.insert(0,">>"+splittedLink[splittedLink.length()-1]);
    for (int i = splittedLink.length()-2; i > -1; i--)
    {
        QString linkPath;
        for (int j = 0; j<=i; j++)
            linkPath.append(splittedLink.mid(0,i+1)[j]+".");
        if (linkPath.right(1) == ".")
            linkPath = linkPath.left(linkPath.length()-1);
        rawContent.insert(0,">> <a id=\"HiLink\" href=\"itom://"+linkPath+"\">"+splittedLink[i]+"</a>");
    }

    // Insert docstring
    // -------------------------------------
    if (htmlNotPlainText)
    {
        QFile file(":/helpTreeDockWidget/help_style");
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray cssData = file.readAll();
            document->addResource( QTextDocument::StyleSheetResource, QUrl("itom_help_style.css"), QString(cssData) );
            file.close();
        }
        // Remake "See Also"-Section so that the links work
        // -------------------------------------
        // Alte "See Also" Section kopieren
        QRegExp seeAlso("(<div class=\"seealso\">).*(</div>)");
        seeAlso.setMinimal(true);
        seeAlso.indexIn(rawContent);
        QString oldSec = seeAlso.capturedTexts()[0];

        // Extract Links (names) from old Section
        QRegExp links("`(.*)`");
        links.setMinimal(true);
        int offset = 0;
        QStringList texts;
        while (links.indexIn(oldSec, offset) > -1)
        {
            texts.append(links.capturedTexts()[1]);
            offset = links.pos()+links.matchedLength();
        }

        // Build the new Section with Headings, Links, etc
        QString newSection = "<p class=\"rubric\">See Also</p><p>";
        for (int i = 0; i<texts.length(); i++)
        {
            newSection.append("\n<a id=\"HiLink\" href=\"itom://"+prefix.left(prefix.lastIndexOf('.'))+"."+texts[i]+"\">"+texts[i].remove('`')+"</a>, ");
        }
        newSection = newSection.left(newSection.length()-2);
        newSection.append("\n</p>");

        // Exchange old Section against new one
        rawContent.remove(seeAlso.pos(), seeAlso.matchedLength());
        rawContent.insert(seeAlso.pos(), newSection);

        document->setHtml( html.arg(rawContent) );
        
        //dummy output (write last loaded Plaintext into html-File)
        QFile file2("helpOutput.html");
        file2.open(QIODevice::WriteOnly);
        file2.write(html.arg(rawContent).toAscii());
        file2.close();
    }
    else
    {   // Only for debug reasons! Displays the Plaintext instead of the html
        rawContent.replace("<br/>","<br/>\n");
        document->setPlainText( html.arg(rawContent) );
    }
    return ito::retOk;
}

//----------------------------------------------------------------------------------------------------------------------------------
// Display the Help-Text
ito::RetVal HelpTreeDockWidget::displayHelp(const QString &path, const int newpage)
{ 
    ito::RetVal retval = ito::retOk;

    ui.textBrowser->clear();
    bool ok = false;
    bool found = false;

    // Das ist ein kleiner workaround mit dem if 5 Zeilen sp�ter. Man k�nnt euahc direkt �ber die includeddbs list iterieren
    // dann w�re folgende Zeile hinf�llig
    QDirIterator it(m_dbPath, QStringList("*.db"), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while(it.hasNext() && !found)
    {
        QString temp = it.next();
        if (m_includedDBs.contains(temp.right(temp.length()-m_dbPath.length()-1)))
        {
            QFile file(temp);
        
            if (file.exists())
            {
                { //important to have variables database and query in local scope such that removeDatabase (outside of this scope) can securly free all resources! -> see docs about removeDatabase
                    // display the help: Run through all the files in the directory
                    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE",temp);
                    database.setDatabaseName(temp);
                    ok = database.open();
                    if (ok)
                    {
                        QSqlQuery query("SELECT type, prefix, name, param, sdesc, doc, htmlERROR  FROM itomCTL WHERE LOWER(prefix) IS '"+path.toUtf8().toLower()+"'", database);
                        query.exec();
                        found = query.next();
                        if (found)
                        {
                            QByteArray docCompressed = query.value(5).toByteArray();
                            QString doc;
                            if (docCompressed.size() > 0)
                            {
                                doc = qUncompress(docCompressed);
                            }

                            if (!m_plaintext)
                            {
                                highlightContent(query.value(1).toString(), query.value(2).toString(), 
                                    query.value(3).toString(), query.value(4).toString(), doc, query.value(6).toString(), ui.textBrowser->document(), true);
                            }
                            else
                            {
                                highlightContent(query.value(1).toString(), query.value(2).toString(), 
                                    query.value(3).toString(), query.value(4).toString(), doc, query.value(6).toString(), ui.textBrowser->document(), false);             
                            }

                            if (newpage == 1)
                            {
                                m_historyIndex++;
                                m_history.insert(m_historyIndex, path.toUtf8());
                                for (int i = m_history.length(); i > m_historyIndex; i--)
                                {
                                    m_history.removeAt(i);
                                }
                            }
                        }
                        database.close();
                    }
                }
                QSqlDatabase::removeDatabase(temp);
            }
        }
    }

    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------
// finds a Modelindex belonging to an Itemname
QModelIndex HelpTreeDockWidget::findIndexByName(const QString &modelName)
{
    QStringList path = modelName.split('.');
    QStandardItem *current = m_pMainModel->invisibleRootItem();
    QStandardItem *temp;
    int counts;
    QString tempString;
    QString firstPath;
    bool found;

    while (path.length() > 0)
    {
        firstPath = path.takeFirst();
        counts = current->rowCount();
        found = false;

        for (int j = 0; j < counts; ++j)
        {
            temp = current->child(j,0);
            tempString = temp->data().toString();

            if (tempString.endsWith(firstPath) && tempString.split(".").last() == firstPath) //fast implementation, first compare will mostly fail, therefore the split is only executed few times
            {
                current = temp;
                found = true;
                break;
            }

            if (!found)
            {
                return QModelIndex(); //nothing found
            }
        }
    }

    return current->index();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Filter the mainmodel
void HelpTreeDockWidget::liveFilter(const QString &filterText)
{
    showTreeview();
    m_pMainFilterModel->setFilterRegExp(filterText);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Returns a list containing the protocol[0] and the real link[1]
// http://thisisthelink
// prot|||....link.....        
QStringList HelpTreeDockWidget::separateLink(const QUrl &link)
{
    QStringList result;

    QRegExp maillink(QString("^(mailto):(.*)"));
    QRegExp itomlink(QString("^([A-Za-z0-9]+)://(.*)"));
    
    if (maillink.indexIn(link.toEncoded(),0) != -1)
    {
        result.append(maillink.cap(1));
        result.append(link.toEncoded());
    }
    else if (itomlink.indexIn(link.toEncoded(),0) != -1)
    {
        result.append(itomlink.cap(1));
        result.append(itomlink.cap(2));
    }
    else
        result.append("-1");
    return result;
}



/*************************************************************/
/*****************GUI related methods*************************/
/*************************************************************/

//----------------------------------------------------------------------------------------------------------------------------------
// Expand all TreeNodes
void HelpTreeDockWidget::expandTree()
{
    ui.treeView->expandAll();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Collapse all TreeNodes
void HelpTreeDockWidget::collapseTree()
{
    ui.treeView->collapseAll();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Link inside Textbrowser is clicked
void HelpTreeDockWidget::on_textBrowser_anchorClicked(const QUrl & link)
{
    QStringList parts = separateLink(link.toString());
    if (parts[0] == "itom")
    {
        displayHelp( parts[1], 1);
        QModelIndex filteredIndex = m_pMainFilterModel->mapFromSource( findIndexByName(parts[1]) );
        ui.treeView->setCurrentIndex(filteredIndex);
    }
    else if (parts[0] == "http")
    {
        QDesktopServices::openUrl(link);
    }
    else if (parts[0] == "mailto")
    {
        QDesktopServices::openUrl(parts[1]);
    }
    else if (parts[0] == "-1")
        ui.label->setText("invalid Link");
    else
    {
        ui.label->setText("unknown protocol");
        QMessageBox msgBox;
        msgBox.setText("The protocol of the link is unknown. ");
        msgBox.setInformativeText("Do you want to try with the external browser?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) 
        {
            case QMessageBox::Yes:
                QDesktopServices::openUrl(link);
            case QMessageBox::No:
                break;
        }
    }    
}

//----------------------------------------------------------------------------------------------------------------------------------
// Saves the position of the splitter depending on the use of the tree or the textbox
void HelpTreeDockWidget::on_splitter_splitterMoved ( int pos, int index )
{
    double width = ui.splitter->width();
    if (m_treeVisible == true)
        m_percWidthVi = pos/width*100;
    else
        m_percWidthUn = pos/width*100;
    if (m_percWidthVi < m_percWidthUn)
        m_percWidthVi = m_percWidthUn+10;
    if (m_percWidthVi == 0)
        m_percWidthVi = 30;
    // Verhaltnis testweise anzeigen lassen
    //ui.label->setText(QString("vi %1 un %2").arg(percWidthVi).arg(percWidthUn));
}

//----------------------------------------------------------------------------------------------------------------------------------
// Show the Help in the right Memo
void HelpTreeDockWidget::on_treeView_clicked(QModelIndex i)
{
    int MyR = Qt::UserRole;
    displayHelp(QString(i.data(MyR+1).toString()), 1);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Back-Button
void HelpTreeDockWidget::navigateBackwards()
{
    if (m_historyIndex > 0)
    {
        m_historyIndex--;
        displayHelp(m_history.at(m_historyIndex), 0);

        // Highlight the entry in the tree
        QModelIndex filteredIndex = m_pMainFilterModel->mapFromSource( findIndexByName(m_history.at(m_historyIndex)) );
        ui.treeView->setCurrentIndex(filteredIndex);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// Forward-Button
void HelpTreeDockWidget::navigateForwards()
{
    if (m_historyIndex < m_history.length()-1)
    {
        m_historyIndex++;
        displayHelp(m_history.at(m_historyIndex), 0);

        // Highlight the entry in the tree
        QModelIndex filteredIndex = m_pMainFilterModel->mapFromSource( findIndexByName(m_history.at(m_historyIndex)) );
        ui.treeView->setCurrentIndex(filteredIndex);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// show tree
void HelpTreeDockWidget::showTreeview()
{
    m_treeVisible = true;
    QList<int> intList;
    intList  <<  ui.splitter->width()*m_percWidthVi/100  <<  ui.splitter->width()*(100-m_percWidthVi)/100;
    ui.splitter->setSizes(intList);
}

//----------------------------------------------------------------------------------------------------------------------------------
// hide tree
void HelpTreeDockWidget::unshowTreeview()
{
    m_treeVisible = false;
    QList<int> intList;
    intList  <<  ui.splitter->width()*m_percWidthUn/100  <<  ui.splitter->width()*(100-m_percWidthUn)/100;
    ui.splitter->setSizes(intList);
}
