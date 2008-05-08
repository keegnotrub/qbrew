/***************************************************************************
  export.cpp
  -------------------
  Export methods for the Recipe class
  -------------------
  Copyright 2005-2008 David Johnson
  Please see the header file for copyright and license information.
 ***************************************************************************/

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QInputDialog>
#include <QLocale>
#include <QMessageBox>
#include <QPrinter>
#include <QTextDocument>
#include <QTextStream>
#include <QXmlStreamWriter>

#include "textprinter.h"
#include "resource.h"
#include "data.h"
#include "recipe.h"

using namespace Resource;

QString escape(const QString &s);
QString underline(const QString &line);

//////////////////////////////////////////////////////////////////////////////
// escape()
// --------
// Escape special xml/html characters

QString escape(const QString &s)
{
    QString x = s;
    x.replace("&", "&amp;");
    x.replace("<", "&lt;");
    x.replace(">", "&gt;");
    x.replace("'", "&apos;");
    x.replace("\"", "&quot;");
    return x;
}

QString underline(const QString &line)
{
    QString text;
    text = line + '\n' + text.fill('-', line.length()) + '\n';
    return text;
}

//////////////////////////////////////////////////////////////////////////////
// exportHTML()
// ------------
// Export recipe as html

bool Recipe::exportHtml(const QString &filename)
{
    if (!filename.isEmpty()) {
        QFile datafile(filename);
        if (!datafile.open(QFile::WriteOnly | QFile::Text)) {
            // error opening file
            qWarning() << "Error: Cannot open file" << filename;
            QMessageBox::warning(0, TITLE,
                                 QObject::tr("Cannot write file %1:\n%2")
                                 .arg(filename)
                                 .arg(datafile.errorString()));
            datafile.close();
            return false;
        }

        QTextStream data(&datafile);
        QApplication::setOverrideCursor(Qt::WaitCursor);

        data << recipeHTML();

        QApplication::restoreOverrideCursor();
        datafile.close();
        return true;
    }  
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// recipeHTML()
// ------------
// Get the html of the recipe for printing and exporting

const QString Recipe::recipeHTML()
{
    const QString header = "<big><strong>%1</strong></big>\n";
    const QString table = "<table summary=\"%1\" border=0 cellpadding=0 cellspacing=%2>\n";
    const QString th = "<td><strong>%1</strong></td>\n";
    
    // heading
    QString html = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
    html += "<html>\n<head>\n";
    html += "<meta name=\"generator\" content=\"" + TITLE + " " + VERSION + " \">\n";
    html += "<title>" + escape(title_) + "</title>\n";
    html += "<meta name=\"author\" content=\"" + escape(brewer_) + "\">\n";
    html += "<style type='text/css'>\n";
    html += "body { font-family: sans-serif; }\n";
    html += "</style>\n";
    html += "</head>\n\n";

    html += "<body>\n";

    html += table.arg("header").arg("5 bgcolor=\"#CCCCCC\" width=\"100%\"");
    html += "<tr><td>\n" + header.arg(escape(title_)) + "</td></tr>\n";
    html += "</table>\n<p>\n\n";

    // recipe table
    html += table.arg("recipe").arg(0);
    html += "<tbody>\n<tr>\n";
    html += th.arg(tr("Recipe"));
    html += "<td width=\"15\"></td>\n";
    html += "<td>" + escape(title_) + "</td>\n";
    html += "<td width=\"25\"></td>\n";

    html += th.arg(tr("Style"));
    html += "<td width=\"15\"></td>\n";
    html += "<td>" + escape(style_.name()) + "</td>\n";
    html += "</tr>\n<tr>\n";

    html += th.arg(tr("Brewer"));
    html += "<td></td>\n";
    html += "<td>" + escape(brewer_) + "</td>\n";
    html += "<td></td>\n";

    html += th.arg(tr("Batch"));
    html += "<td></td>\n";
    html += "<td>" + size_.toString(2) + "</td>\n";

    html += "</tr>\n<tr>\n" + th.arg(tr(method().toUtf8()));
    html += "</tr>\n</tbody>\n</table>\n<p>\n\n";

    // characteristics table
    html += header.arg(tr("Recipe Characteristics"));
    html += table.arg("characteristics").arg(0);
    html += "<tbody>\n<tr>\n";

    html += th.arg(tr("Recipe Gravity"));
    html += "<td width=\"15\"></td>\n";
    html += "<td>" + QString::number(og_, 'f', 3) + tr(" OG</td>\n");
    html += "<td width=\"25\"></td>\n";

    html += th.arg(tr("Estimated FG"));
    html += "<td width=\"15\"></td>\n";
    html += "<td>" + QString::number(FGEstimate(), 'f', 3) + tr(" FG</td>\n");
    html += "</tr>\n<tr>\n";

    html += th.arg(tr("Recipe Bitterness"));
    html += "<td></td>\n";
    html += "<td>" + QString::number(ibu_, 'f', 0) + tr(" IBU</td>\n");
    html += "<td></td>\n";

    html += th.arg(tr("Alcohol by Volume"));
    html += "<td></td>\n";
    html += "<td>" + QString::number(ABV() * 100.0, 'f', 1) + tr("%</td>\n");
    html += "</tr>\n<tr>\n";

    html += th.arg(tr("Recipe Color"));
    html += "<td></td>\n";
    html += "<td>" + QString::number(srm_, 'f', 0) + DEGREE + tr(" SRM</td>\n");
    html += "<td></td>\n";

    html += th.arg(tr("Alcohol by Weight"));
    html += "<td></td>\n";
    html += "<td>" + QString::number(ABW() * 100.0, 'f', 1) + tr("%</td>\n");
    html += "</tr>\n</tbody>\n</table>\n<p>\n\n";

    // ingredients table
    html += header.arg(tr("Ingredients"));
    html += table.arg("ingredients").arg(0);
    html += "<tbody>\n";

    // grains
    html += "<tr>\n" + th.arg(tr("Quantity"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Grain"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Type"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Use"));
    html += "</tr>\n\n";

    foreach (Grain grain, grains_) {
        html += "<tr>\n<td>" + grain.weight().toString(2) + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + escape(grain.name()) + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + grain.type() + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + grain.use() + "</td>\n</tr>\n\n";
    }

    // hops
    html += "<tr>\n" + th.arg(tr("Quantity"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Hop"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Type"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Time"));
    html += "</tr>\n\n";

    foreach (Hop hop, hops_) {
        html += "<tr>\n<td>" + hop.weight().toString(2) + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + escape(hop.name()) + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + hop.type() + "</td>\n";
        html += "<td></td>\n";
        html += "<td colspan=2>" + QString::number(hop.time()) + tr(" minutes</td>\n</tr>\n\n");
    }

    // misc ingredients
    html += "<tr>\n" + th.arg(tr("Quantity"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Misc"));
    html += "<td width=\"15\"></td>\n";
    html += th.arg(tr("Notes"));
    html += "</tr>\n\n";

    foreach (Misc misc, miscs_) {
        html += "<tr>\n<td>" + misc.quantity().toString(2) + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + escape(misc.name()) + "</td>\n";
        html += "<td></td>\n";
        html += "<td>" + misc.type() + "</td>\n";
        html += "<td></td>\n";
        html += "<td colspan=3>" + escape(misc.notes()) + "</td>\n</tr>\n\n";
    }

    html += "</tbody>\n</table>\n<p>\n\n";

    // notes
    // TODO: using replace() might be dangerous if we ever use richtext in notes
    html += header.arg(tr("Recipe Notes")) + "\n";
    html += "<p>" + escape(recipenotes_).replace('\n', "<br>\n") + "\n</p>\n<p>\n";

    html += header.arg(tr("Batch Notes")) + "\n";
    html += "<p>" + escape(batchnotes_).replace('\n', "<br>\n") + "\n</p>\n";

    html += "</body>\n</html>\n";

    return html;
}

//////////////////////////////////////////////////////////////////////////////
// exportText()
// ------------
// Export recipe as text

bool Recipe::exportText(const QString &filename)
{
    if (!filename.isEmpty()) {
        QFile datafile(filename);
        if (!datafile.open(QFile::WriteOnly | QFile::Text)) {
        // error opening file
            qWarning() << "Error: Cannot open file" << filename;
            QMessageBox::warning(0, TITLE,
                                 QObject::tr("Cannot write file %1:\n%2")
                                 .arg(filename)
                                 .arg(datafile.errorString()));
            datafile.close();
            return false;
        }

        QTextStream data(&datafile);
        QApplication::setOverrideCursor(Qt::WaitCursor);

        data << recipeText();

        QApplication::restoreOverrideCursor();
        datafile.close();
        return true;
    }  
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// recipeText()
// ------------
// Get the ascii text of the recipe for exporting

const QString Recipe::recipeText()
{
    // title stuff
    QString text = underline(title());
    text += tr("Brewer: ") + brewer_ + '\n';
    text += tr("Style: ") +  style_.name() + '\n';
    text += tr("Batch: ") + size_.toString(2);
    text += tr(method().toUtf8());
    text += "\n\n";

    // style stuff
    text += underline(tr("Characteristics"));
    text += tr("Recipe Gravity: ") +
        QString::number(og_, 'f', 3) + tr(" OG\n");
    text += tr("Recipe Bitterness: ") +
        QString::number(ibu_, 'f', 0) + tr(" IBU\n");
    text += tr("Recipe Color: ") +
        QString::number(srm_, 'f', 0) + DEGREE + tr(" SRM\n");
    text += tr("Estimated FG: ") +
        QString::number(FGEstimate(), 'f', 3) + '\n';
    text += tr("Alcohol by Volume: ") +
        QString::number(ABV() * 100.0, 'f', 1) + tr("%\n");
    text += tr("Alcohol by Weight: ") +
        QString::number(ABW() * 100.0, 'f', 1) + tr("%\n\n");

    // ingredients
    text += underline(tr("Ingredients"));

    // grains
    // using a map will sort the grains
    QMultiMap<QString, Grain> gmap;
    foreach (Grain grain, grains_) gmap.insert(grain.name(), grain);

    foreach (Grain grain, gmap.values()) {
        text += grain.name().leftJustified(30, ' ');
        text += grain.weight().toString(2) + ", ";
        text += grain.type() + ", ";
        text += grain.use() + '\n';
    }
    text += '\n';

    // hops
    // using a map will sort the hops
    QMultiMap<QString, Hop> hmap;
    foreach (Hop hop, hops_) hmap.insert(hop.name(), hop);

    foreach (Hop hop, hmap.values()) {
        text += hop.name().leftJustified(30, ' ');
        text += hop.weight().toString(2) + ", ";
        text += hop.type() + ", ";
        text += QString::number(hop.time()) + tr(" minutes\n");
    }
    text += '\n';

    // misc ingredients
    // using a map will sort the ingredients
    QMultiMap<QString, Misc> mmap;
    foreach (Misc misc, miscs_) mmap.insert(misc.name(), misc);

    foreach (Misc misc, mmap.values()) {
        text += misc.name().leftJustified(30, ' ');
        text += misc.quantity().toString(2) + ", ";
        text += misc.type() + ", ";
        text += misc.notes() + '\n';
    }
    text += '\n';

    // notes
    text += underline(tr("Notes"));

    // TODO: wrap long notes
    text += tr("Recipe Notes:\n") + recipenotes_ + "\n\n";
    text += tr("Batch Notes:\n") + batchnotes_ + "\n\n";

    return text;
}

//////////////////////////////////////////////////////////////////////////////
// exportBeerXML()
// ---------------
// Export recipe to BeerXML format

bool Recipe::exportBeerXML(const QString &filename)
{
    // open file
    QFile datafile(filename);
    if (!datafile.open(QFile::WriteOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open file" << filename;
        QMessageBox::warning(0, TITLE,
                             QObject:: tr("Cannot write file %1:\n%2")
                             .arg(filename)
                             .arg(datafile.errorString()));
        datafile.close();
        return false;
    }


    // write out xml
    QString buf;
    QXmlStreamWriter xml(&datafile);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);
#endif
    xml.writeStartDocument(); // BeerXML 1 doesn't have a doctype
    xml.writeComment(QString("BeerXML generated by %1 %2")
                     .arg(PACKAGE).arg(VERSION));
    xml.writeStartElement(tagRECIPES);

    // write recipe information
    xml.writeStartElement(tagRECIPE);
    xml.writeTextElement(tagVERSION, beerXMLVersion);
    xml.writeTextElement(tagNAME, title_);
    xml.writeTextElement(tagTYPE, method());
    xml.writeTextElement(tagBREWER, brewer_);
    double size = size_.amount(Volume::liter);
    xml.writeTextElement(tagBATCHSIZE, QString::number(size,'f',8));
    xml.writeTextElement(tagEFFICIENCY,
        QString::number(Data::instance()->efficiency()*100.0,'f',1));

    // write style
    // TODO: unfinished...
    // category
    // category number
    // style letter
    // style guide
    // type
    // og
    xml.writeStartElement(tagSTYLE);
    xml.writeTextElement(tagVERSION, beerXMLVersion);
    xml.writeTextElement(tagNAME, style_.name());
    xml.writeTextElement(tagOGMIN, QString::number(style_.OGLow(),'f',4));
    xml.writeTextElement(tagOGMAX, QString::number(style_.OGHi(),'f',4));
    xml.writeTextElement(tagFGMIN, QString::number(style_.FGLow(),'f',4));
    xml.writeTextElement(tagFGMAX, QString::number(style_.FGHi(),'f',4));
    xml.writeTextElement(tagIBUMIN, QString::number(style_.IBULow(),'f',2));
    xml.writeTextElement(tagIBUMAX, QString::number(style_.IBUHi(),'f',2));
    xml.writeTextElement(tagCOLORMIN, QString::number(style_.SRMLow(),'f',2));
    xml.writeTextElement(tagCOLORMAX, QString::number(style_.SRMHi(),'f',2));
    xml.writeEndElement(); // tagSTYLE

    // fermentables list
    xml.writeStartElement(tagFERMENTABLES);
    foreach (Grain grain, grains_) {
        xml.writeStartElement(tagFERMENTABLE);
        xml.writeTextElement(tagVERSION, beerXMLVersion);
        xml.writeTextElement(tagNAME, grain.name());
        size = grain.weight().amount(Weight::kilogram);
        xml.writeTextElement(tagAMOUNT, QString::number(size,'f',8));
        xml.writeTextElement(tagCOLOR, QString::number(grain.color(),'f',2));
        double yield = extractToYield(grain.extract()) * 100.0;
        xml.writeTextElement(tagYIELD, QString::number(yield,'f',2));
        xml.writeTextElement(tagTYPE, grain.type());
        xml.writeEndElement(); // tagFERMENTABLE
    }
    xml.writeEndElement(); // tagFERMENTABLES

    // hop list
    xml.writeStartElement(tagHOPS);
    foreach (Hop hop, hops_) {
        xml.writeStartElement(tagHOP);
        xml.writeTextElement(tagVERSION, beerXMLVersion);
        xml.writeTextElement(tagNAME, hop.name());
        size = hop.weight().amount(Weight::kilogram);
        xml.writeTextElement(tagAMOUNT, QString::number(size,'f',8));
        xml.writeTextElement(tagALPHA, QString::number(hop.alpha(),'f',2));
        // TODO: note that I'm using "Boil" for all hops...
        xml.writeTextElement(tagUSE, "Boil");
        xml.writeTextElement(tagTIME, QString::number(hop.time()));
        buf = "Leaf";
        if (hop.type() == Hop::PELLET_STRING) buf = "Pellet";
        if (hop.type() == Hop::PLUG_STRING) buf = "Plug";
        xml.writeTextElement(tagFORM, buf);
        xml.writeEndElement(); // tagHOP
    }
    xml.writeEndElement(); // tagHOPS

    // yeast list
    xml.writeStartElement(tagYEASTS);
    foreach (Misc misc, miscs_) {
        // iterate through list looking for yeasts
        if (misc.type() == Misc::YEAST_STRING) {
            xml.writeStartElement(tagYEAST);
            xml.writeTextElement(tagVERSION, beerXMLVersion);
            xml.writeTextElement(tagNAME, misc.name());
            xml.writeTextElement(tagAMOUNT, "0.00"); // TODO: fixup for miscs
            xml.writeTextElement(tagTYPE, Misc::YEAST_STRING);
            // TODO: form???
            xml.writeTextElement(tagNOTES, misc.notes());
            xml.writeEndElement(); // tagYEAST
        }
    }
    xml.writeEndElement(); // tagYEASTS

    // misc list
    xml.writeStartElement(tagMISCS);
    foreach (Misc misc, miscs_) {
        // iterate through list looking for yeasts
        if (misc.type() != Misc::YEAST_STRING) {
            xml.writeStartElement(tagMISC);
            xml.writeTextElement(tagVERSION, beerXMLVersion);
            xml.writeTextElement(tagNAME, misc.name());
            xml.writeTextElement(tagAMOUNT, "0.00"); // TODO: fixup for miscs
            xml.writeTextElement(tagTYPE, misc.type());
            xml.writeTextElement(tagNOTES, misc.notes());
            // use ???
            // time ???
            xml.writeEndElement(); // tagMISC
        }
    }
    xml.writeEndElement(); // tagMISCS

    // waters
    // NOTE: not currently supporting water
    xml.writeStartElement(tagWATERS);
    xml.writeEndElement();

    // mash
    // NOTE: not currently supporting mash

    // notes
    if ((recipenotes_.length() + batchnotes_.length()) > 0) {
        buf = recipenotes_;
        if (batchnotes_.length() > 0) {
            if (buf.length() > 0) buf += "\n\n";
            buf += batchnotes_;
        }
        xml.writeTextElement(tagNOTES, buf);
    }

    xml.writeEndElement(); // tagRECIPE
    xml.writeEndElement(); // tagRECIPES
    datafile.close();

    QApplication::restoreOverrideCursor();
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// importBeerXml()
// ---------------
// Import recipe from BeerXML format

bool Recipe::importBeerXml(const QString &filename)
{
    // open file
    QFile datafile(filename);
    if (!datafile.open(QFile::ReadOnly | QFile::Text)) {
        // error opening file
        qWarning() << "Error: Cannot open" << filename;
        QMessageBox::warning(0, TITLE,
                             QObject::tr("Cannot read file %1:\n%2")
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

    // check the doc type and root tag
    QDomElement root = doc.documentElement();
    if (root.tagName() != tagRECIPES) {
        // wrong file type
        qWarning() << "Error: Wrong file type" << filename;
        QMessageBox::warning(0, TITLE,
                             QObject::tr("Wrong file type for %1").arg(filename));
        QApplication::restoreOverrideCursor();
        return false;
    }

    QDomNodeList nodes;
    QDomElement element, sub;

    // BeerXML may have more than one recipe
    QMap<QString, QDomElement> recipes;
    element = root.firstChildElement(tagRECIPE);
    for (; !element.isNull(); element=element.nextSiblingElement(tagRECIPE)) {
        sub = element.firstChildElement(tagNAME);
        if (!sub.isNull()) {
            recipes[sub.text()] = element;
        }
    }
    if (recipes.count() == 0) {
        qWarning() << "Error: No recipes found in" << filename;
        QMessageBox::warning(0, TITLE,
            QObject::tr("No recipes found in %1").arg(filename));
        QApplication::restoreOverrideCursor();
        return false;
    }

    if (recipes.count() > 1) {
        // select just one recipe
        bool ok;
        QString item = QInputDialog::getItem(0, TITLE,
            QObject::tr("Multiple recipes found. Please select one:"),
            recipes.keys(), 0, false, &ok);
        if (ok) {
            root = recipes[item];
        } else {
            // bail
            QApplication::restoreOverrideCursor();
            return false;
        }
    } else {
        root = recipes.begin().value();
    }
    recipes.clear();

    // check recipe version
    element = root.firstChildElement(tagVERSION);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element";
    } else {
        if (element.text() != "1") {
            qWarning() << "Error: Wrong BeerXML version in" << filename;
            QMessageBox::warning(0, TITLE,
                QObject::tr("Cannot process file %1").arg(filename));
            QApplication::restoreOverrideCursor();
            return false;
        }
    }

    // get title
    element = root.firstChildElement(tagNAME);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagNAME;
    } else {
        setTitle(element.text());
    }
    // get brewer
    element = root.firstChildElement(tagBREWER);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagBREWER;
    } else {
        setBrewer(element.text());
    }
    // get style information
    element = root.firstChildElement(tagSTYLE);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagSTYLE;
    } else {
        // TODO: load/save entire style
        sub = element.firstChildElement(tagNAME);
        if (sub.isNull()) {
            qDebug() << "Warning:: bad DOM element" << tagNAME;
        } else {
            setStyle(sub.text());
        }
    }

    // get batch settings
    bool mashed = false;
    element = root.firstChildElement(tagTYPE);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagTYPE;
    } else {
        mashed = (element.text() != EXTRACT_STRING);
    }
    element = root.firstChildElement(tagBATCHSIZE);
    if (element.isNull()) {
        qDebug() << "Warning:: bad DOM element" << tagBATCHSIZE;
    } else {
        setSize(Volume(element.text(), Volume::liter));
    }

    // get notes
    recipenotes_.clear();
    batchnotes_.clear();
    element = root.firstChildElement(tagNOTES);
    if (!element.isNull()) {
        setRecipeNotes(element.text());
    }
    element = root.firstChildElement(tagTASTENOTES);
    if (!element.isNull()) {
        setBatchNotes(element.text());
    }

    // get fermentables
    grains_.clear();
    element = root.firstChildElement(tagFERMENTABLES);
    for (; !element.isNull(); element=element.nextSiblingElement(tagFERMENTABLES)) {
        sub = element.firstChildElement(tagFERMENTABLE);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagFERMENTABLE)) {
            Grain grain;
            QDomElement el = sub.firstChildElement(tagNAME);
            grain.setName(el.text());
            el = sub.firstChildElement(tagAMOUNT);
            grain.setWeight(Weight(el.text().toDouble(), Weight::kilogram));
            el = sub.firstChildElement(tagYIELD);
            grain.setExtract(yieldToExtract(el.text().toDouble() / 100.0));
            el = sub.firstChildElement(tagCOLOR);
            grain.setColor(el.text().toDouble());
            
            // set type and guess at use
            el = sub.firstChildElement(tagTYPE);
            grain.setType(Grain::OTHER_STRING);
            grain.setUse(Grain::OTHER_STRING);
            if (el.text() == "Grain") {
                grain.setType(Grain::GRAIN_STRING);
                if (mashed) grain.setUse(Grain::MASHED_STRING);
                else        grain.setUse(Grain::STEEPED_STRING);
            }
            if (el.text() == "Sugar") {
                grain.setType(Grain::SUGAR_STRING);
                grain.setUse(Grain::OTHER_STRING);
            }
            if ((el.text() == "Extract") ||
                (el.text() == "Dry Extract")) {
                grain.setType(Grain::EXTRACT_STRING);
                grain.setUse(Grain::EXTRACT_STRING);
            }
            if (el.text() == "Adjunct") {
                grain.setType(Grain::ADJUNCT_STRING);
                grain.setUse(Grain::MASHED_STRING);
            }
            addGrain(grain);
        }
    }

    // get hops
    hops_.clear();
    element = root.firstChildElement(tagHOPS);
    for (; !element.isNull(); element=element.nextSiblingElement(tagHOPS)) {
        sub = element.firstChildElement(tagHOP);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagHOP)) {
            Hop hop;
            QDomElement el = sub.firstChildElement(tagNAME);
            hop.setName(el.text());
            el = sub.firstChildElement(tagAMOUNT);
            hop.setWeight(Weight(el.text().toDouble(), Weight::kilogram));
            el = sub.firstChildElement(tagALPHA);
            hop.setAlpha(el.text().toDouble() / 100.0);
            // TODO: add USE to future hop notes
            el = sub.firstChildElement(tagTIME);
            hop.setTime(el.text().toUInt());
            el = sub.firstChildElement(tagFORM);
            if (el.text() == "Pellet") {
                hop.setType(Hop::PELLET_STRING);
            } else if (el.text() == "Plug") {
                hop.setType(Hop::PLUG_STRING);
            } else {
                hop.setType(Hop::WHOLE_STRING);
            }
            addHop(hop);
        }
    }

    // get miscs
    miscs_.clear();
    element = root.firstChildElement(tagMISCS);
    for (; !element.isNull(); element=element.nextSiblingElement(tagMISCS)) {
        sub = element.firstChildElement(tagMISC);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagMISC)) {
            Misc misc;
            QDomElement el = sub.firstChildElement(tagNAME);
            // TODO: put TYPE and USE in notes
            misc.setName(el.text());
            bool aiw = (el.text().toLower() == "true");
            el = sub.firstChildElement(tagAMOUNT);
            if (aiw) {
                misc.setQuantity(Weight(el.text().toDouble(), Weight::kilogram));
            } else {
                misc.setQuantity(Volume(el.text().toDouble(), Volume::liter));
            }

            // set type
            el = sub.firstChildElement(tagTYPE);
            misc.setType(Misc::OTHER_STRING);
            if (el.text() == "Spice") misc.setType(Misc::SPICE_STRING);
            if (el.text() == "Fining") misc.setType(Misc::FINING_STRING);
            if (el.text() == "Herb") misc.setType(Misc::HERB_STRING);
            if (el.text() == "Flavor") misc.setType(Misc::FLAVOR_STRING);
            if (el.text() == "Water Agent") misc.setType(Misc::ADDITIVE_STRING);

            el = sub.firstChildElement(tagNOTES);
            misc.setNotes(el.text());

            addMisc(misc);
        }
    }

    // get yeasts
    element = root.firstChildElement(tagYEASTS);
    for (; !element.isNull(); element=element.nextSiblingElement(tagYEASTS)) {
        sub = element.firstChildElement(tagYEAST);
        for (; !sub.isNull(); sub=sub.nextSiblingElement(tagYEAST)) {
            Misc misc;
            QDomElement el = sub.firstChildElement(tagNAME);
            misc.setName(el.text());
            el = sub.firstChildElement(tagAMOUNTISWEIGHT);
            bool aiw = (el.text().toLower() == "true");
            el = sub.firstChildElement(tagAMOUNT);
            if (aiw) {
                misc.setQuantity(Weight(el.text().toDouble(), Weight::kilogram));
            } else {
                misc.setQuantity(Volume(el.text().toDouble(), Volume::liter));
            }
            misc.setType(Misc::YEAST_STRING);
            QString notes;
            el = sub.firstChildElement(tagFORM);
            if (!el.isNull()) notes = QString("%1 yeast. ").arg(el.text());
            el = sub.firstChildElement(tagNOTES);
            notes += el.text();
            misc.setNotes(notes);
            addMisc(misc);
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
// exportPdf()
// ------------
// Export recipe as pdf

bool Recipe::exportPdf(TextPrinter *textprinter, const QString &filename)
{
    if (textprinter && (!filename.isEmpty())) {
        QTextDocument document;
        document.setHtml(recipeHTML());
        textprinter->exportPdf(&document, QString(), filename);
        return true;
    }
    return false;
}
