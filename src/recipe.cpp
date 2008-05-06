/***************************************************************************
  recipe.cpp
  -------------------
  Recipe (document) class
  -------------------
  Copyright 2001-2008, David Johnson
  Please see the header file for copyright and license information
 ***************************************************************************/

#include <math.h>

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QTextDocument>
#include <QTextStream>
#include <QXmlStreamReader>

#include "textprinter.h"
#include "data.h"
#include "recipe.h"
#include "resource.h"

#ifndef HAVE_ROUND
#define round(x) floor(x+0.5)
#endif

using namespace Resource;

const QByteArray Recipe::EXTRACT_STRING = QT_TRANSLATE_NOOP("recipe", "Extract");
const QByteArray Recipe::PARTIAL_STRING = QT_TRANSLATE_NOOP("recipe", "Partial Mash");
const QByteArray Recipe::ALLGRAIN_STRING = QT_TRANSLATE_NOOP("recipe", "All Grain");

//////////////////////////////////////////////////////////////////////////////
// Construction, destruction                                                //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Recipe()
// --------
// Default constructor
Recipe::Recipe(QObject *parent)
        : QObject(parent), modified_(false), title_(), brewer_(),
          size_(5.0, Volume::gallon), style_(), grains_(),  hops_(), miscs_(),
          recipenotes_(), batchnotes_(), og_(0.0), ibu_(0), srm_(0)
{ ; }

Recipe::~Recipe()
{ ; }

//////////////////////////////////////////////////////////////////////////////
// Serialization                                                            //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// newRecipe()
// -------------
// Clears recipe (new document)

void Recipe::newRecipe()
{
    title_.clear();
    brewer_.clear();
    size_ = Data::instance()->defaultSize();
    style_ = Data::instance()->defaultStyle();
    grains_.clear();
    hops_.clear();
    miscs_.clear();
    recipenotes_.clear();
    batchnotes_.clear();

    og_ = 0.0;
    ibu_ = 0;
    srm_ = 0;

    setModified(false); // new documents are not in a modified state
    emit (recipeChanged());
}

/////////////////////////////////////////////////////////////////////////////
// nativeFormat()
// -------------
// Is the recipe in native format?
// Defined as "recipe" doctype with generator or application as "qbrew"

bool Recipe::nativeFormat(const QString &filename)
{
    QFile datafile(filename);
    if (!datafile.open(QFile::ReadOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open" << filename;
        return false;
    }
    // note: datafile will close on return

    // find root/first element
    QXmlStreamReader xml(&datafile);
    do {
        xml.readNext();
    } while (!xml.atEnd() && !xml.isStartElement());

    if (xml.hasError()) return false;

    // check the document type
    if (xml.name() != tagRecipe) return false;

    // check application
    if (xml.attributes().value(attrApplication) != PACKAGE) {
        // check generator if no application
        if (xml.attributes().value(attrGenerator) != PACKAGE) {
            return false;
        }
    }

    return true;
}

// TODO: make BeerXML stuff a plugin?

/////////////////////////////////////////////////////////////////////////////
// beerXmlFormat()
// ---------------
// Is the recipe in BeerXML format?
// Defined as XML with "<RECIPES>" and "<RECIPE>", VERSION 1
// Note that BeerXML 1.0 is poorly designed format

bool Recipe::beerXmlFormat(const QString &filename)
{
    QFile datafile(filename);
    if (!datafile.open(QFile::ReadOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open" << filename;
        return false;
    }
    // note: datafile will close on return

    // find root/first element
    QXmlStreamReader xml(&datafile);
    do {
        xml.readNext();
    } while (!xml.atEnd() && !xml.isStartElement());
    if (xml.hasError()) return false;

    // check the document type
    if (xml.name() != tagRECIPES) return false;

    // check for RECIPE
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == tagRECIPE) {
                // check VERSION
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement()) {
                        if (xml.name() == tagVERSION) {
                            if (xml.readElementText() == "1") return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// loadRecipe()
// --------------
// Load a recipe

bool Recipe::loadRecipe(const QString &filename)
{
    // open file
    QFile datafile(filename);
    if (!datafile.open(QFile::ReadOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open" << filename;
        QMessageBox::warning(0, TITLE,
                             tr("Cannot read file %1:\n%2")
                             .arg(filename)
                             .arg(datafile.errorString()));
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
    if (root.tagName() != tagRecipe) {
        // wrong file type
        qWarning() << "Error: Wrong file type" << filename;
        QMessageBox::warning(0, TITLE,
                             tr("Wrong file type for %1").arg(filename));
        QApplication::restoreOverrideCursor();
         return false;
    }

    // check application
    if (root.attribute(attrApplication) != PACKAGE) {
        // check generator if no application
        if (root.attribute(attrGenerator) != PACKAGE) {
            qWarning() << "Not a recipe file for" << TITLE;
            QMessageBox::warning(0, TITLE,
                                 tr("Not a recipe for %1").arg(TITLE.data()));
            QApplication::restoreOverrideCursor();
             return false;
        }
    }

    // check file version
    if (root.attribute(attrVersion) < RECIPE_PREVIOUS) {
        // too old of a version
        qWarning() << "Error: Unsupported version" << filename;
        QMessageBox::warning(0, TITLE,
                             tr("Unsupported version %1").arg(filename));
        QApplication::restoreOverrideCursor();
         return false;
    }

    // Note: only use first tag if multiple single-use tags in doc
    QDomNodeList nodes;
    QDomElement element, sub;

    // get title
    element = root.firstChildElement(tagTitle);
    if (element.isNull()) {
        qDebug() << "Warning:: missing DOM element" << tagTitle;
    } else {
        setTitle(element.text());
    }
    // get brewer
    element = root.firstChildElement(tagBrewer);
    if (element.isNull()) {
        qDebug() << "Warning:: missing DOM element" << tagBrewer;
    } else {
        setBrewer(element.text());
    }
    // get style
    element = root.firstChildElement(tagStyle);
    if (element.isNull()) {
        qDebug() << "Warning:: missing DOM element" << tagStyle;
    } else {
        // TODO: load/save entire style
        setStyle(element.text());
    }
    // get batch settings // TODO: eliminate this tag, use quantity, efficiency
    element = root.firstChildElement(tagBatch);
    if (!element.isNull()) {
        if (element.attribute(attrQuantity) != QString()) {
            setSize(Volume(element.attribute(attrQuantity), Volume::gallon));
        } else if (element.attribute(attrSize) != QString()) {
            // deprecated tag
            setSize(Volume(element.attribute(attrSize), Volume::gallon));
        }
    }

    // get notes
    recipenotes_.clear();
    batchnotes_.clear();
    element = root.firstChildElement(tagNotes);
    for (; !element.isNull(); element=element.nextSiblingElement(tagNotes)) {
        if (element.hasAttribute(attrClass)) {
            if (element.attribute(attrClass) == classRecipe) {
                if (!recipenotes_.isEmpty()) recipenotes_.append('\n');
                recipenotes_.append(element.text());
            } else if (element.attribute(attrClass) == classBatch) {
                if (!batchnotes_.isEmpty()) batchnotes_.append('\n');
                batchnotes_.append(element.text());
            }
        }
    }

    // get all grains tags
    grains_.clear();
    element = root.firstChildElement(tagGrains);
    for (; !element.isNull(); element=element.nextSiblingElement(tagGrains)) {
        // get all grain tags
        sub = element.firstChildElement(tagGrain);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagGrain)) {
            addGrain(Grain(sub.text(),
                           Weight(sub.attribute(attrQuantity), Weight::pound),
                           sub.attribute(attrExtract).toDouble(),
                           sub.attribute(attrColor).toDouble(),
                           sub.attribute(attrType, Grain::OTHER_STRING),
                           sub.attribute(attrUse)));
        }
    }

    // get all hops tags
    hops_.clear();
    element = root.firstChildElement(tagHops);
    for (; !element.isNull(); element=element.nextSiblingElement(tagHops)) {
            // get all hop tags
        sub = element.firstChildElement(tagHop);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagHop)) {
            // TODO: attrForm is deprecated 0.4.0
            QString hoptype = sub.attribute(attrType);
            if (hoptype == QString()) hoptype = sub.attribute(attrForm);
            addHop(Hop(sub.text(),
                       Weight(sub.attribute(attrQuantity), Weight::ounce),
                       hoptype,
                       sub.attribute(attrAlpha).toDouble(),
                       sub.attribute(attrTime).toUInt()));
        }
    }

    // get all misc tags
    miscs_.clear();
    element = root.firstChildElement(tagMiscs);
    for (; !element.isNull(); element=element.nextSiblingElement(tagMiscs)) {
        // get all hop tags
        sub = element.firstChildElement(tagMisc);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagMisc)) {
            addMisc(Misc(sub.text(),
                         Quantity(sub.attribute(attrQuantity),
                                  Quantity::generic),
                         sub.attribute(attrType, Misc::OTHER_STRING),
                         sub.attribute(attrNotes)));
        }
    }

    // calculate the numbers
    recalc();

    // just loaded recipes  are not modified
    setModified(false);
    emit (recipeChanged());

    QApplication::restoreOverrideCursor();
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// saveRecipe()
// ---------------
// Save a recipe

bool Recipe::saveRecipe(const QString &filename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QDomDocument doc(tagRecipe);
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));

    // create the root element
    QDomElement root = doc.createElement(doc.doctype().name());
    root.setAttribute(attrApplication, PACKAGE);
    root.setAttribute(attrVersion, VERSION);
    doc.appendChild(root);

    // title
    QDomElement element = doc.createElement(tagTitle);
    element.appendChild(doc.createTextNode(title_));
    root.appendChild(element);
    // brewer
    element = doc.createElement(tagBrewer);
    element.appendChild(doc.createTextNode(brewer_));
    root.appendChild(element);
    // style
    // TODO: load/save entire style
    element = doc.createElement(tagStyle);
    element.appendChild(doc.createTextNode(style_.name()));
    root.appendChild(element);
    root.appendChild(element);
    // batch settings
    element = doc.createElement(tagBatch);
    element.setAttribute(attrQuantity, size_.toString());
    root.appendChild(element);
    // notes
    if (!recipenotes_.isEmpty()) {
        element = doc.createElement(tagNotes);
        element.setAttribute(attrClass, classRecipe);
        element.appendChild(doc.createTextNode(recipenotes_));
        root.appendChild(element);
    }
    if (!batchnotes_.isEmpty()) {
        element = doc.createElement(tagNotes);
        element.setAttribute(attrClass, classBatch);
        element.appendChild(doc.createTextNode(batchnotes_));
        root.appendChild(element);
    }

    // grains elements
    element = doc.createElement(tagGrains);
    QDomElement subelement;
    foreach(Grain grain, grains_) {
        // iterate through grain list
        subelement = doc.createElement(tagGrain);
        subelement.appendChild(doc.createTextNode(grain.name()));
        subelement.setAttribute(attrQuantity, grain.weight().toString());
        subelement.setAttribute(attrExtract, grain.extract());
        subelement.setAttribute(attrColor, grain.color());
        subelement.setAttribute(attrType, grain.type());
        subelement.setAttribute(attrUse, grain.use());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // hops elements
    element = doc.createElement(tagHops);
    foreach(Hop hop, hops_) {
        // iterate through hop list
        subelement = doc.createElement(tagHop);
        subelement.appendChild(doc.createTextNode(hop.name()));
        subelement.setAttribute(attrQuantity, hop.weight().toString());
        subelement.setAttribute(attrType, hop.type());
        subelement.setAttribute(attrAlpha, hop.alpha());
        subelement.setAttribute(attrTime, hop.time());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // miscingredients elements
    element = doc.createElement(tagMiscs);
    foreach(Misc misc, miscs_) {
        // iterate through misc list
        subelement = doc.createElement(tagMisc);
        subelement.appendChild(doc.createTextNode(misc.name()));
        subelement.setAttribute(attrQuantity, misc.quantity().toString());
        subelement.setAttribute(attrType, misc.type());
        subelement.setAttribute(attrNotes, misc.notes());
        element.appendChild(subelement);
    }
    root.appendChild(element);

    // open file
    QFile datafile(filename);
    if (!datafile.open(QFile::WriteOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open file" << filename;
        QMessageBox::warning(0, TITLE,
                             tr("Cannot write file %1:\n%2")
                             .arg(filename)
                             .arg(datafile.errorString()));
        datafile.close();
        QApplication::restoreOverrideCursor();
        return false;
    }

    // write it out
    QTextStream data(&datafile);
    doc.save(data, 2);
    datafile.close();

    // recipe is saved, so set flags accordingly
    setModified(false);
    QApplication::restoreOverrideCursor();
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// previewRecipe()
// ---------------
// Preview the recipe (assumes textprinter has been setup)

void Recipe::previewRecipe(TextPrinter *textprinter)
{
    // TODO: waiting for Qt 4.4.0, and hopefully a better print infrastructure
    if (!textprinter) return;
    QTextDocument document;
    document.setHtml(recipeHTML());
    textprinter->preview(&document);
}

    void previewRecipe(TextPrinter *textprinter, QWidget *wparent);
//////////////////////////////////////////////////////////////////////////////
// printRecipe()
// ---------------
// Print the recipe (assumes textprinter has been setup)

void Recipe::printRecipe(TextPrinter *textprinter)
{
    if (!textprinter) return;
    QTextDocument document;
    document.setHtml(recipeHTML());
    textprinter->print(&document);
}

//////////////////////////////////////////////////////////////////////////////
// Miscellaneous                                                            //
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// setStyle()
// ----------
// Set style from string

void Recipe::setStyle(const QString &s)
{
    if (Data::instance()->hasStyle(s))
        style_ = Data::instance()->style(s);
    else
        style_ = Style();
    setModified(true);
}

//////////////////////////////////////////////////////////////////////////////
// addGrain()
// ----------
// Add a grain ingredient to the recipe

void Recipe::addGrain(const Grain &g)
{
    grains_.append(g);
    recalc();
    setModified(true);
}

//////////////////////////////////////////////////////////////////////////////
// addHop()
// ----------
// Add a hop ingredient to the recipe

void Recipe::addHop(const Hop &h)
{
    hops_.append(h);
    recalc();
    setModified(true);
}

//////////////////////////////////////////////////////////////////////////////
// addMisc()
// ----------
// Add a misc ingredient to the recipe

void Recipe::addMisc(const Misc &m)
{
    miscs_.append(m);
    recalc();
    setModified(true);
}

//////////////////////////////////////////////////////////////////////////////
// recipeType()
// ------------
// Return type of recipe

QString Recipe::method()
{
    int extract = 0;
    int mash = 0;

    foreach(Grain grain, grains_) {
        if (grain.use().toLower() == Grain::MASHED_STRING.toLower()) mash++;
        else if (grain.use().toLower() == Grain::EXTRACT_STRING.toLower()) extract++;
    }

    if (mash > 0) {
        if (extract > 0) return PARTIAL_STRING;
        else             return ALLGRAIN_STRING;
    }
    return EXTRACT_STRING;
}

//////////////////////////////////////////////////////////////////////////////
// Calculations                                                             //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// recalc()
// -------
// Recalculate recipe values

void Recipe::recalc()
{
    og_ = calcOG();
    ibu_ = calcIBU();
    srm_ = calcSRM();
}

//////////////////////////////////////////////////////////////////////////////
// calcOG()
// --------
// Calculate the original gravity

double Recipe::calcOG()
{
    double yield;
    double est = 0.0;
    foreach(Grain grain, grains_) {
        yield = grain.yield();
        if (grain.use().toLower() == Grain::MASHED_STRING.toLower()) {
            // adjust for mash efficiency
            yield *= Data::instance()->efficiency();
        } else if (grain.use().toLower() == Grain::STEEPED_STRING.toLower()) {
                // steeped grains don't yield nearly as much as mashed grains
                yield *= Data::instance()->steepYield();
        }
        est += yield;
    }
    if (size_.amount()) {
        est /= size_.amount(Volume::gallon);
    } else {
        est = 0.0;
    }
    return est + 1.0;
}

//////////////////////////////////////////////////////////////////////////////
// calcIBU()
// ---------
// Calculate the bitterness

int Recipe::calcIBU()
{
    // switch between two possible calculations
    if (Data::instance()->tinseth())
        return calcTinsethIBU();
    else
        return calcRagerIBU();
}

//////////////////////////////////////////////////////////////////////////////
// calcRagerIBU()
// --------------
// Calculate the bitterness based on Rager's method (table method)

int Recipe::calcRagerIBU()
{
// TODO: update this (and other) hop calculations
    double bitterness = 0.0;
    foreach(Hop hop, hops_) {
        bitterness += hop.HBU() * Data::instance()->utilization(hop.time());
        // TODO: we should also correct for hop type
    }
    if (size_.amount()) {
        bitterness /= size_.amount(Volume::gallon);
    } else {
        bitterness = 0.0;
    }
    // correct for boil gravity
    if (og_ > 1.050) bitterness /= 1.0 + ((og_ - 1.050) / 0.2);
    return (int)round(bitterness);
}

//////////////////////////////////////////////////////////////////////////////
// calcTinsethIBU()
// ----------------
// Calculate the bitterness based on Tinseth's method (formula method)
// The formula used is:
// (1.65*0.000125^(gravity-1))*(1-EXP(-0.04*time))*alpha*mass*1000
// ---------------------------------------------------------------
// (volume*4.15)

// TODO: recheck this formula

int Recipe::calcTinsethIBU()
{
    const double GPO = 28.3495; // grams per ounce
    const double LPG = 3.785;   // liters per gallon

    const double COEFF1 = 1.65;
    const double COEFF2 = 0.000125;
    const double COEFF3 = 0.04;
    const double COEFF4 = 4.15;

    double ibu;
    double bitterness = 0.0;
    foreach(Hop hop, hops_) {
        ibu = (COEFF1 * pow(COEFF2, (og_ - 1.0))) *
            (1.0 - exp(-COEFF3 * hop.time())) *
            (hop.alpha()) * hop.weight().amount(Weight::ounce) * 1000.0;
        if (size_.amount()) {
            ibu /= (size_.amount(Volume::gallon) * COEFF4);
        } else {
            ibu = 0.0;
        }
        bitterness += ibu;
    }
    bitterness *= (GPO / LPG) / 100.0;
    return (int)round(bitterness);
}

//////////////////////////////////////////////////////////////////////////////
// calcSRM()
// ---------
// Calculate the color

int Recipe::calcSRM()
{
    double srm = 0.0;
    foreach(Grain grain, grains_) {
        srm += grain.HCU();
    }
    if (size_.amount()) {
        srm /= size_.amount(Volume::gallon);
    } else {
        srm = 0.0;
    }

    // switch between two possible calculations
    if (Data::instance()->morey()) {
        // power model (morey) [courtesy Rob Hudson <rob@tastybrew.com>]
        srm = (pow(srm, 0.6859)) * 1.4922;
        if (srm > 50) srm = 50;
    } else {
        // linear model (daniels)
        if (srm > 8.0) {
            srm *= 0.2;
            srm += 8.4;
        }
    }
    return (int)round(srm);
}

// TODO: following formulas need to use constants

//////////////////////////////////////////////////////////////////////////////
// FGEstimate()
// ------------
// Return estimated final gravity

double Recipe::FGEstimate()
{
    if (og_ <= 0.0) return 0.0;
    return (((og_ - 1.0) * 0.25) + 1.0);
}

//////////////////////////////////////////////////////////////////////////////
// ABV()
// -----
// Calculate alcohol by volume

double Recipe::ABV() // recipe version
{
    return (ABV(og_, FGEstimate()));
}

double Recipe::ABV(double og, double fg) // static version
{ 
    return (og - fg) * 1.29;
}

//////////////////////////////////////////////////////////////////////////////
// ABW()
// -----
// Calculate alcohol by weight
// NOTE: Calculations were taken from http://hbd.org/ensmingr/

double Recipe::ABW() // recipe version
{
    return (ABW(og_, FGEstimate()));
}
 
double Recipe::ABW(double og, double fg)  // static version
{
    return (ABV(og, fg) * 0.785);
}

//////////////////////////////////////////////////////////////////////////////
// SgToP()
// -------
// Convert specific gravity to degrees plato

double Recipe::SgToP(double sg)
{
    return ((-463.37) + (668.72*sg) - (205.35 * sg * sg));
}

//////////////////////////////////////////////////////////////////////////////
// extractToYield()
// ----------------
// Convert extract potential to percent yield

// TODO: need to double check this, as well as terms
double Recipe::extractToYield(double extract)
{
    const double SUCROSE = 46.21415;
    return (((extract-1.0)*1000.0) / SUCROSE);
}

//////////////////////////////////////////////////////////////////////////////
// yieldToExtract()
// ----------------
// Convert percent yield to extract potential

double Recipe::yieldToExtract(double yield)
{
    const double SUCROSE = 46.21415;
    return ((yield*SUCROSE)/1000.0)+1.0;
}

//////////////////////////////////////////////////////////////////////////////
// Miscellaneous                                                            //
//////////////////////////////////////////////////////////////////////////////

void Recipe::setModified(bool mod)
{
    modified_ = mod;
    if (mod) emit recipeModified();
}

bool Recipe::modified() const
{
    return modified_;
}

