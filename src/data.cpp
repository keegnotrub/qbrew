/***************************************************************************
  data.cpp
  -------------------
  Brewing data
  -------------------
  Copyright (c) 2001-2007 David Johnson
  Please see the header file for copyright and license information.
 ***************************************************************************/

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QMutex>
#include <QStringList>
#include <QTextStream>

#include "qbrew.h"
#include "recipe.h"
#include "resource.h"
#include "data.h"

using namespace Resource;

Data *Data::instance_ = 0;
static QMutex instancelock;

// Construction, Destruction /////////////////////////////////////////////////

// Private constructor
Data::Data()
    : defaultsize_(Volume(5.0, Volume::gallon)), defaultstyle_(),
      defaulthopform_(Hop::PELLET_STRING), defaultgrainunit_(&Weight::pound),
      defaulthopunit_(&Weight::ounce),  defaultmiscunit_(&Quantity::generic),
      grainmap_(), hopmap_(), miscmap_(), stylemap_(), utable_(),
      steepyield_(0.5), efficiency_(0.75), tinseth_(true), morey_(true)
{ ; }

// Private destructor
Data::~Data() { ; }

// Return pointer to the data
Data *Data::instance()
{
    if (!instance_) {
        QMutexLocker lock(&instancelock);
        if (!instance_) instance_ = new Data();
    }
    return instance_;
}

// Initialize
void Data::initialize(const ConfigState &state)
{
    // set defaults
    if (state.calc.units == UNIT_METRIC) {
        setDefaultSize(Volume(state.recipe.batch, Volume::liter));
        setDefaultGrainUnit(Weight::kilogram);
        setDefaultHopUnit(Weight::gram);
        setDefaultTempUnit(Temperature::celsius);
    } else if (state.calc.units == UNIT_US) {
        setDefaultSize(Volume(state.recipe.batch, Volume::gallon));
        setDefaultGrainUnit(Weight::pound);
        setDefaultHopUnit(Weight::ounce);
        setDefaultTempUnit(Temperature::fahrenheit);
    }
    setDefaultStyle(state.recipe.style);
    setDefaultHopForm(state.recipe.hopform);
    setDefaultMiscUnit(Quantity::generic);
    setEfficiency(state.calc.efficiency);

    // load data file - try user home directory first, quietly...
    if (!loadData(QDIR_HOME + "/." + DATA_FILE, true)) {
        // then try the default data file
        if (!loadData(QBrew::instance()->dataBase() + DATA_FILE)) {
            qWarning() << "Warning: could not open data file";
            QMessageBox::warning(0, TITLE, QObject::tr("Could not open data file"));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Data Access                                                              //
//////////////////////////////////////////////////////////////////////////////

void Data::setDefaultStyle(const QString &style)
{
    if (stylemap_.contains(style)) defaultstyle_ = stylemap_.value(style);
}

//////////////////////////////////////////////////////////////////////////////
// style()
// -------
// Return a style given its name

Style Data::style(const QString &name)
{
    if (stylemap_.contains(name)) return stylemap_.value(name);
    return Style();
}

//////////////////////////////////////////////////////////////////////////////
// grain()
// -------
// Return grain given its name

Grain Data::grain(const QString &name)
{
    Grain grain;
    if (grainmap_.contains(name)) grain = grainmap_.value(name);
    grain.setWeight(Weight(1.0, *defaultgrainunit_));
    return grain;
}

//////////////////////////////////////////////////////////////////////////////
// hop()
// -----
// Return hop given its name

Hop Data::hop(const QString &name)
{
    Hop hop;
    if (hopmap_.contains(name)) hop = hopmap_.value(name);
    hop.setWeight(Weight(1.0, *defaulthopunit_));
    hop.setForm(defaulthopform_);
    return hop;
}

//////////////////////////////////////////////////////////////////////////////
// misc()
// ------
// Return misc ingredient given its name

Misc Data::misc(const QString &name)
{
    Misc misc;
    if (miscmap_.contains(name)) misc = miscmap_.value(name);
    misc.setQuantity(Quantity(1.0, *defaultmiscunit_));
    return misc;
}

//////////////////////////////////////////////////////////////////////////////
// utilization
// -----------
// Look up the utilization for the given time

double Data::utilization(unsigned time)
{
    foreach(UEntry entry, utable_) {
        if (time >= entry.time) return (double(entry.utilization));
    }
    return 0.0;
}

//////////////////////////////////////////////////////////////////////////////
// addUEntry()
// -----------
// Add an entry to the utilization table [static]

void Data::addUEntry(const UEntry &entry)
{
    // keep the list sorted from highest time to lowest
    QList<UEntry>::Iterator it;
    if (utable_.isEmpty()) {
        utable_.append(entry);
    } else {
        for (it=utable_.begin(); it != utable_.end(); ++it) {
            if ((*it).time < entry.time) break;
        }
        utable_.insert(it, entry);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Serialization                                                            //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// loadData()
// ----------
// Load the data

bool Data::loadData(const QString &filename, bool quiet)
{
    // open file
    QFile datafile(filename);
    if (!datafile.open(QFile::ReadOnly | QFile::Text)) {
        // error opening file
        if (!quiet) {
            qWarning() << "Error: Cannot read" << filename;
            QMessageBox::warning(0, TITLE,
                                 QObject::tr("Cannot read file %1:\n%2")
                                 .arg(filename)
                                 .arg(datafile.errorString()));
        }
        datafile.close();
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // open dom recipe
    QDomDocument doc;
    doc.setContent(&datafile);
    datafile.close();

    // check the doc type and stuff
    QDomElement root = doc.documentElement();
    if (root.tagName() != tagDoc) {
        // wrong file type
        qWarning() << "Error: Wrong file type" << filename;
        QMessageBox::warning(0, TITLE,
                             QObject::tr("Wrong file type for %1").arg(filename));
        QApplication::restoreOverrideCursor();
        return false;
    }

    // check file version
    if (root.attribute(attrVersion) < DATA_PREVIOUS) {
        // too old of a version
        qWarning() << "Error: Unsupported version" << filename;
        QMessageBox::warning(0, TITLE, QObject::tr("Unsupported version %1")
                                       .arg(filename));
        QApplication::restoreOverrideCursor();
        return false;
    }

    QDomNodeList nodes;
    QDomElement element, sub;

    // get all styles tags
    stylemap_.clear();
    element = root.firstChildElement(tagStyles);
    for (; !element.isNull(); element=element.nextSiblingElement(tagStyles)) {
        // get all style tags
        sub = element.firstChildElement(tagStyle);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagStyle)) {
            stylemap_.insert(sub.text(),
                             Style(sub.text(),
                                   sub.attribute(attrOGLow).toDouble(),
                                   sub.attribute(attrOGHigh).toDouble(),
                                   sub.attribute(attrFGLow, "0.0").toDouble(),
                                   sub.attribute(attrFGHigh, "0.0").toDouble(),
                                   sub.attribute(attrIBULow).toInt(),
                                   sub.attribute(attrIBUHigh).toInt(),
                                   sub.attribute(attrSRMLow).toInt(),
                                   sub.attribute(attrSRMHigh).toInt()));
        }
    }

    // get all grains tags
    grainmap_.clear();
    element = root.firstChildElement(tagGrains);
    for (; !element.isNull(); element=element.nextSiblingElement(tagGrains)) {
        // get all grain tags
        sub = element.firstChildElement(tagGrain);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagGrain)) {
            // TODO: grain type deprecates data file format (0.4.0)
            grainmap_.insert(sub.text(),
                             Grain(sub.text(),
                                   Weight(1.0, *defaultgrainunit_),
                                   sub.attribute(attrExtract).toDouble(),
                                   sub.attribute(attrColor).toDouble(),
                                   sub.attribute(attrType, Grain::OTHER_STRING),
                                   sub.attribute(attrUse)));
        }
    }

    // get all hops tags
    hopmap_.clear();
    element = root.firstChildElement(tagHops);
    for (; !element.isNull(); element=element.nextSiblingElement(tagHops)) {
        // get all hop tags
        sub = element.firstChildElement(tagHop);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagHop)) {
            hopmap_.insert(sub.text(),
                           Hop(sub.text(),
                               Weight(1.0, *defaulthopunit_), QString(),
                               sub.attribute(attrAlpha).toDouble(), 60));
        }
    }

    // get all miscingredients tags
    miscmap_.clear();
    element = root.firstChildElement(tagMiscs);
    for (; !element.isNull(); element=element.nextSiblingElement(tagMiscs)) {
        // get all hop tags
        sub = element.firstChildElement(tagMisc);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagMisc)) {
                    miscmap_.insert(sub.text(),
                                    Misc(sub.text(),
                                         Quantity(1.0, *defaultmiscunit_),
                                         sub.attribute(attrType, Misc::OTHER_STRING),
                                         sub.attribute(attrNotes)));
        }
    }

    // get all utilization tags
    utable_.clear();
    UEntry entry;
    element = root.firstChildElement(tagUtilization);
    for (; !element.isNull(); element=element.nextSiblingElement(tagUtilization)) {
        // get all entry tags
        sub = element.firstChildElement(tagEntry);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagEntry)) {
            entry.time = sub.attribute(attrTime).toUInt();
            entry.utilization = sub.attribute(attrUtil).toUInt();
            addUEntry(entry);
        }
    }

    QApplication::restoreOverrideCursor();
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// saveData()
// ------------
// Save info to data file

bool Data::saveData(const QString &filename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QDomDocument doc(tagDoc);
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));

    // create the root element
    QDomElement root = doc.createElement(doc.doctype().name());
    root.setAttribute(attrVersion, VERSION);
    doc.appendChild(root);

    QDomElement element, subelement;

    // styles elements
    element = doc.createElement(tagStyles);
    foreach(Style style, stylemap_) {
        // iterate through style list
        subelement = doc.createElement(tagStyle);
        subelement.appendChild(doc.createTextNode(style.name()));
        subelement.setAttribute(attrOGLow, style.OGLow());
        subelement.setAttribute(attrOGHigh, style.OGHi());
        subelement.setAttribute(attrFGLow, style.FGLow());
        subelement.setAttribute(attrFGHigh, style.FGHi());
        subelement.setAttribute(attrIBULow, style.IBULow());
        subelement.setAttribute(attrIBUHigh, style.IBUHi());
        subelement.setAttribute(attrSRMLow, style.SRMLow());
        subelement.setAttribute(attrSRMHigh, style.SRMHi());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // grains elements
    element = doc.createElement(tagGrains);
    foreach(Grain grain, grainmap_) {
        // iterate through grain list
        subelement = doc.createElement(tagGrain);
        subelement.appendChild(doc.createTextNode(grain.name()));
        subelement.setAttribute(attrExtract, grain.extract());
        subelement.setAttribute(attrColor, grain.color());
        subelement.setAttribute(attrType, grain.type());
        subelement.setAttribute(attrUse, grain.use());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // hops elements
    element = doc.createElement(tagHops);
    foreach(Hop hop, hopmap_) {
        // iterate through hop list
        subelement = doc.createElement(tagHop);
        subelement.appendChild(doc.createTextNode(hop.name()));
        subelement.setAttribute(attrAlpha, hop.alpha());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // miscingredients elements
    element = doc.createElement(tagMiscs);
    foreach(Misc misc, miscmap_) {
        // iterate through misc list
        subelement = doc.createElement(tagMisc);
        subelement.appendChild(doc.createTextNode(misc.name()));
        subelement.setAttribute(attrType, misc.type());
        subelement.setAttribute(attrNotes, misc.notes());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // utilization elements
    element = doc.createElement(tagUtilization);
    foreach(UEntry uentry, utable_) {
        // iterate through uentry list
        subelement = doc.createElement(tagEntry);
        subelement.setAttribute(attrTime, uentry.time);
        subelement.setAttribute(attrUtil, uentry.utilization);
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // open file
    QFile datafile(filename);
    if (!datafile.open(QFile::WriteOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open file" << filename;
        QMessageBox::warning(0, TITLE,
                             QObject::tr("Cannot write file %1:\n%2")
                             .arg(filename)
                             .arg(datafile.errorString()));
        datafile.close();
        QApplication::restoreOverrideCursor();
        return false;
    }

    // write it out
    QTextStream textstream(&datafile);
    doc.save(textstream, 2);
    datafile.close();

    QApplication::restoreOverrideCursor();
    return true;
}
