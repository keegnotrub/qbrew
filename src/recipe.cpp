/***************************************************************************
  recipe.cpp
  -------------------
  Recipe (document) class
  -------------------
  Copyright 2001-2007, David Johnson
  Please see the header file for copyright and license information
 ***************************************************************************/

#include <math.h>

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QTextDocument>
#include <QTextStream>

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
        datafile.close();
        return false;
    }

    // parse document
    QDomDocument doc;
    bool status = doc.setContent(&datafile);
    datafile.close();
    if (!status) return false;

    // check the document type
    QDomElement root = doc.documentElement();
    if (root.tagName() != tagRecipe) return false;

    // check application
    if (root.attribute(attrApplication) != PACKAGE) {
        // check generator if no application
        if (root.attribute(attrGenerator) != PACKAGE)
            return false;
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
        datafile.close();
        return false;
    }

    // parse document
    QDomDocument doc;
    bool status = doc.setContent(&datafile);
    datafile.close();
    if (!status) return false;

    // check the document type
    QDomElement root = doc.documentElement();
    if (root.tagName() != tagRECIPES) return false;

    // check for RECIPE
    QDomElement element, sub;
    element = root.firstChildElement(tagRECIPE);
    if (!element.isNull()) {
        // check VERSION
        sub = element.firstChildElement(tagVERSION);
        if (!sub.isNull()) {
            if (sub.text() == "1") return true;
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
        qDebug() << "Warning:: bad DOM element" << tagTitle;
    } else {
        setTitle(element.text());
    }
    // get brewer
    element = root.firstChildElement(tagBrewer);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagBrewer;
    } else {
        setBrewer(element.text());
    }
    // get style
    element = root.firstChildElement(tagStyle);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagStyle;
    } else {
        // TODO: load/save entire style
        setStyle(element.text());
    }
    // get batch settings // TODO: eliminate this tag, use quantity, efficiency
    element = root.firstChildElement(tagBatch);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagBatch;
    } else {
        setSize(Volume(element.attribute(attrQuantity), Volume::gallon));
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
            addHop(Hop(sub.text(),
                       Weight(sub.attribute(attrQuantity), Weight::ounce),
                       sub.attribute(attrForm),
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
        subelement.setAttribute(attrForm, hop.form());
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
        if (grain.use() == Grain::MASHED_STRING) mash++;
        else if (grain.use() == Grain::EXTRACT_STRING) extract++;
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
        if (grain.use() == Grain::MASHED_STRING) {
            // adjust for mash efficiency
            yield *= Data::instance()->efficiency();
        } else if (grain.use() == Grain::STEEPED_STRING) {
                // steeped grains don't yield nearly as much as mashed grains
                yield *= Data::instance()->steepYield();
        }
        est += yield;
    }
    est /= size_.amount(Volume::gallon);
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
    double bitterness = 0.0;
    foreach(Hop hop, hops_) {
        bitterness += hop.HBU() * Data::instance()->utilization(hop.time());
        // TODO: we should also correct for hop form
    }
    bitterness /= size_.amount(Volume::gallon);
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
        ibu /= (size_.amount(Volume::gallon)) * COEFF4;
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
    srm /= size_.amount(Volume::gallon);

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
    double fg = FGEstimate();
    return ABW(og_, fg) * fg / 0.79;
}

double Recipe::ABV(double og, double fg) // static version
{ 
    return ABW(og, fg) * fg / 0.79;
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
    double oe, ae, re;
    oe = SgToP(og);
    ae = SgToP(fg);
    re = (0.1808 * oe) + (0.8192 * ae); // real extract
    return ((oe - re) / (2.0665 - 0.010665 * oe) / 100.0);
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
