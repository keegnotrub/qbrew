///////////////////////////////////////////////////////////////////////////////
// textprinter.h
// -------------------
// Copyright (c) 2007 David Johnson <david@usermode.org>
// Please see the header file for copyright and license information.
///////////////////////////////////////////////////////////////////////////////

#include "textprinter.h"
#include "previewdialog.h"

#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QFileDialog>
#include <QLocale>
#include <QPainter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextFrame>

static inline double mmToInches(double mm) { return mm * 0.039370147; }

///////////////////////////////////////////////////////////////////////////////
// TextPrinter()
///////////////////////////////////////////////////////////////////////////////
/// Constructs a TextPrinter object.
/// Any related dialogs will be displayed centered over the parent, if it is
/// a QWidget object.
///
/// Note that printing will be done with QPrinter::ScreenResolution printer
/// mode. This is to ensure that preview and printed output have identical
/// layouts.
///////////////////////////////////////////////////////////////////////////////

TextPrinter::TextPrinter(QObject *parent)
    : QObject(parent), parent_(0),
      printer_(new QPrinter(QPrinter::HighResolution)), leftmargin_(15.0),
      rightmargin_(15.0), topmargin_(15.0), bottommargin_(15.0), spacing_(5.0),
      headersize_(0.0), headerrule_(true), headertext_(QString()),
      footersize_(0.0), footerrule_(true), footertext_(QString())
{
    if (parent) parent_ = qobject_cast<QWidget*>(parent);

    printer_->setFullPage(true);
    printer_->setOrientation(QPrinter::Portrait);
    
    // for convenience, default to US_Letter for C/US/Canada
    // NOTE: bug in Qt, this value is not loaded by QPrintDialog
    switch (QLocale::system().country()) {
      case QLocale::AnyCountry:
      case QLocale::Canada:
      case QLocale::UnitedStates:
      case QLocale::UnitedStatesMinorOutlyingIslands:
          printer_->setPageSize(QPrinter::Letter); break;
      default:
          printer_->setPageSize(QPrinter::A4); break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// ~TextPrinter()
///////////////////////////////////////////////////////////////////////////////
/// Destroys the TextPrinter object.
///////////////////////////////////////////////////////////////////////////////

TextPrinter::~TextPrinter()
{
    delete printer_;
}

///////////////////////////////////////////////////////////////////////////////
// pageSize()
///////////////////////////////////////////////////////////////////////////////
/// Return the page size. This is defined by the QPrinter::PageSize
/// enumeration.
///////////////////////////////////////////////////////////////////////////////

QPrinter::PageSize TextPrinter::pageSize()
{
    return printer_->pageSize(); 
}

///////////////////////////////////////////////////////////////////////////////
// setPageSize()
///////////////////////////////////////////////////////////////////////////////
/// Set the page size. Size is defined by the QPrinter::PageSize enumeration.
/// By default, this is QPrinter::Letter in US and Canada locales, and
/// QPrinter::A4 in other locales.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setPageSize(QPrinter::PageSize size)
{
    printer_->setPageSize(size);
}

///////////////////////////////////////////////////////////////////////////////
// orientation()
///////////////////////////////////////////////////////////////////////////////
/// Return the page orientation. This is defined by the QPrinterOrientation
/// enumeration.
///////////////////////////////////////////////////////////////////////////////

QPrinter::Orientation TextPrinter::orientation()
{
    return printer_->orientation();
}

///////////////////////////////////////////////////////////////////////////////
// setOrientation()
///////////////////////////////////////////////////////////////////////////////
/// Set the page orientation. Orientation is defined by the
/// QPrinter::Orientation enumeration, and can be QPrinter::Portrait (the
/// default) or QPrinter::Landscape.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setOrientation(QPrinter::Orientation orientation)
{
    printer_->setOrientation(orientation);
}

///////////////////////////////////////////////////////////////////////////////
// leftMargin()
///////////////////////////////////////////////////////////////////////////////
/// Return the left page margin width in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::leftMargin()
{
    return leftmargin_;
}

///////////////////////////////////////////////////////////////////////////////
// setLeftMargin()
///////////////////////////////////////////////////////////////////////////////
/// Set the left margin width in millimeters. The default value is 15mm.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setLeftMargin(double margin)
{
    if ((margin > 0) && (margin < printer_->paperRect().width() / 2)) {
        leftmargin_ = margin;
    } else {
        leftmargin_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// rightMargin()
///////////////////////////////////////////////////////////////////////////////
/// Return the right page margin width in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::rightMargin()
{
    return rightmargin_;
}

///////////////////////////////////////////////////////////////////////////////
// setRightMargin()
///////////////////////////////////////////////////////////////////////////////
/// Set the right margin width in millimeters. The default value is 15mm.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setRightMargin(double margin)
{
    if ((margin > 0) && (margin < printer_->paperRect().width() / 2)) {
        rightmargin_ = margin;
    } else {
        rightmargin_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// topMargin()
///////////////////////////////////////////////////////////////////////////////
/// Return the top page margin size in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::topMargin()
{
    return topmargin_;
}

///////////////////////////////////////////////////////////////////////////////
// setTopMargin()
///////////////////////////////////////////////////////////////////////////////
/// Set the top margin size in millimeters. The default value is 15mm.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setTopMargin(double margin)
{
    if ((margin > 0) && (margin < printer_->paperRect().height() / 4)) {
        topmargin_ = margin;
    } else {
        topmargin_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// bottomMargin()
///////////////////////////////////////////////////////////////////////////////
/// Return the bottom page margin size in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::bottomMargin()
{
    return bottommargin_;
}

///////////////////////////////////////////////////////////////////////////////
// setBottomMargin()
///////////////////////////////////////////////////////////////////////////////
/// Set the bottom margin size in millimeters. The default value is 15mm.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setBottomMargin(double margin)
{
    if ((margin > 0) && (margin < printer_->paperRect().height() / 4)) {
        bottommargin_ = margin;
    } else {
        bottommargin_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// setMargins()
///////////////////////////////////////////////////////////////////////////////
/// Set all margins to have the identical size. The default value is 15mm.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setMargins(double margin)
{
    if ((margin > 0)
        && (margin < printer_->paperRect().height() / 2)
        && (margin < printer_->paperRect().width() / 2)) {
        leftmargin_ = rightmargin_ = topmargin_ = bottommargin_ = margin;
    } else {
        leftmargin_ = rightmargin_ = topmargin_ = bottommargin_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// spacing()
///////////////////////////////////////////////////////////////////////////////
/// Return the spacing between the page content and the header and footer
/// blocks. This is defined in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::spacing()
{
    return spacing_;
}

///////////////////////////////////////////////////////////////////////////////
// setSpacing()
///////////////////////////////////////////////////////////////////////////////
/// Set the spacing between the page content and the header and footer blocks.
/// The default value is 5mm.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setSpacing(double spacing)
{
    if ((spacing > 0) && (spacing <= printer_->paperRect().height() / 8)) {
        spacing_ = spacing;
    } else {
        spacing_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// headerSize()
///////////////////////////////////////////////////////////////////////////////
/// Return the height of the header block  in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::headerSize()
{
    return headersize_;
}

///////////////////////////////////////////////////////////////////////////////
// setHeaderSize()
///////////////////////////////////////////////////////////////////////////////
/// Set the height of the header in millimeters. The default is zero (no
/// header).
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setHeaderSize(double size)
{
    if ((size > 0) && (size <= printer_->paperRect().height() / 8)) {
        headersize_ = size;
    } else {
        headersize_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// headerRule()
///////////////////////////////////////////////////////////////////////////////
/// Return whether the rule for the header is enabled.
///////////////////////////////////////////////////////////////////////////////

bool TextPrinter::headerRule()
{
    return headerrule_;
}

///////////////////////////////////////////////////////////////////////////////
// setHeaderRule()
///////////////////////////////////////////////////////////////////////////////
/// Enable or disable the header rule. If enabled (default), the rule will be
/// drawn below the header.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setHeaderRule(bool on)
{
    headerrule_ = on;
}

///////////////////////////////////////////////////////////////////////////////
// headerText()
///////////////////////////////////////////////////////////////////////////////
/// Return the text for the header.
///////////////////////////////////////////////////////////////////////////////

const QString &TextPrinter::headerText()
{
    return headertext_;
}

///////////////////////////////////////////////////////////////////////////////
// setHeaderText()
///////////////////////////////////////////////////////////////////////////////
/// Set the text for the header. Rich text is supported. HTML tags may be used
/// to format the text and align elements.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setHeaderText(const QString &text)
{
    headertext_ = text;
}

///////////////////////////////////////////////////////////////////////////////
// footerSize()
///////////////////////////////////////////////////////////////////////////////
/// Return the height of the footer block in millimeters.
///////////////////////////////////////////////////////////////////////////////

double TextPrinter::footerSize()
{
    return footersize_;
}

///////////////////////////////////////////////////////////////////////////////
// setFooterSize()
///////////////////////////////////////////////////////////////////////////////
/// Set the height of the footer in millimeters. The default is zero (no
/// header).
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setFooterSize(double size)
{
    if ((size > 0) && (size <= printer_->paperRect().height() / 8)) {
        footersize_ = size;
    } else {
        footersize_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// footerRule()
///////////////////////////////////////////////////////////////////////////////
/// Return whether the rule for the footer is set.
///////////////////////////////////////////////////////////////////////////////

bool TextPrinter::footerRule()
{
    return footerrule_;
}

///////////////////////////////////////////////////////////////////////////////
// setFooterRule()
///////////////////////////////////////////////////////////////////////////////
/// Enable or disable the footer rule. If enabled (default), the rule will be
/// drawn above the footer.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setFooterRule(bool on)
{
    footerrule_ = on;
}

///////////////////////////////////////////////////////////////////////////////
// footerText()
///////////////////////////////////////////////////////////////////////////////
/// Return the text for the footer.
///////////////////////////////////////////////////////////////////////////////

const QString &TextPrinter::footerText()
{
    return footertext_;
}

///////////////////////////////////////////////////////////////////////////////
// setFooterText()
///////////////////////////////////////////////////////////////////////////////
/// Set the text for the footer. Rich text is supported. HTML tags may be used
/// to format the text and align elements.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setFooterText(const QString &text)
{
    footertext_ = text;
}

///////////////////////////////////////////////////////////////////////////////
// print()
///////////////////////////////////////////////////////////////////////////////
/// Print a document. A standard print dialog will be displayed. If the
/// caption parameter is not empty, it will be used as the title for the print
/// dialog. TextPrinter does not take ownership of the document, but will
/// make a clone. The wysiwyg parameter forces the printer resolution down to
/// screen resolution.
///
/// \todo eliminate the wysiwyg hack
/// \todo add overload taking html string
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::print(const QTextDocument *document,
                        const QString &caption,
                        bool wysiwyg)
{
    if (!document) return;

    // setup printer
    printer_->setOutputFormat(QPrinter::NativeFormat);
    printer_->setOutputFileName(QString());

    // show print dialog
    QPrintDialog dialog(printer_, parent_);
    dialog.setWindowTitle(caption.isEmpty() ? "Print Document" : caption);
    if (dialog.exec() == QDialog::Rejected) return;

    // setup document
    QTextDocument *doc = document->clone();
    doc->setUseDesignMetrics(true);
    // TODO: wysiwyg hack
    if (wysiwyg && parent_) printer_->setResolution(parent_->logicalDpiY());
    doc->documentLayout()->setPaintDevice(printer_);
    doc->setPageSize(contentRect(printer_).size());
    // dump existing margin (if any)
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(0);
    doc->rootFrame()->setFrameFormat(fmt);

    // to iterate through pages we have to worry about
    // copies, collation, page range, and print order

    // get num copies
    int doccopies;
    int pagecopies;
    if (printer_->collateCopies()) {
        doccopies = 1;
        pagecopies = printer_->numCopies();
    } else {
        doccopies = printer_->numCopies();
        pagecopies = 1;
    }

    // get page range
    int firstpage = printer_->fromPage();
    int lastpage = printer_->toPage();
    if (firstpage == 0 && lastpage == 0) { // all pages
        firstpage = 1;
        lastpage = doc->pageCount();
    }

    // print order
    bool ascending = true;
    if (printer_->pageOrder() == QPrinter::LastPageFirst) {
        int tmp = firstpage;
        firstpage = lastpage;
        lastpage = tmp;
        ascending = false;
    }

    // loop through and print pages
    QPainter painter(printer_);
    for (int dc=0; dc<doccopies; dc++) {
        int pagenum = firstpage;
        while (true) {
            for (int pc=0; pc<pagecopies; pc++) {
                if (printer_->printerState() == QPrinter::Aborted ||
                    printer_->printerState() == QPrinter::Error) {
                    goto breakout;
                }
                // print page
                paintPage(&painter, doc, pagenum);
                if (pc < pagecopies-1) printer_->newPage();
            }
            if (pagenum == lastpage) break;
            if (ascending) pagenum++;
            else           pagenum--;
            printer_->newPage();
        }

        if (dc < doccopies-1) printer_->newPage();
    }

 breakout:
    delete doc;
}

///////////////////////////////////////////////////////////////////////////////
// exportPDF()
///////////////////////////////////////////////////////////////////////////////
/// Export the document in PDF format. If caption is not empty, it will be used
/// as the title for the dialog. If filename is empty, a standard file
/// selection dialog will be displayed. TextPrinter does not take ownership of
/// the document, but will make a clone. The wysiwyg parameter forces the
/// printer resolution down to screen resolution.
///
/// \todo eliminate the wysiwyg hack
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::exportPdf(const QTextDocument *document,
                            const QString &caption,
                            const QString &filename,
                            bool wysiwyg)
{
    if (!document) return;

    // file save dialog
    QString dialogcaption = caption.isEmpty() ? "Export PDF" : caption;
    QString exportname;
    if (QFile::exists(filename)) {
        exportname = filename;
    } else {
        exportname = QFileDialog::getSaveFileName(parent_, dialogcaption,
                                                  filename, "*.pdf");
    }
    if (exportname.isEmpty()) return;
    if (QFileInfo(exportname).suffix().isEmpty())
        exportname.append(".pdf");

    // setup printer
    printer_->setOutputFormat(QPrinter::PdfFormat);
    printer_->setOutputFileName(exportname);

    // setup document
    QTextDocument *doc = document->clone();
    // TODO: hack to get wysiwyg
    if (wysiwyg && parent_) printer_->setResolution(parent_->logicalDpiY());
    doc->documentLayout()->setPaintDevice(printer_);
    doc->setPageSize(contentRect(printer_).size());
    // dump existing margin (if any)
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(0);
    doc->rootFrame()->setFrameFormat(fmt);

    // print pdf
    QPainter painter(printer_);
    for (int pagenum=1; pagenum<=doc->pageCount(); pagenum++) {
        paintPage(&painter, doc, pagenum);
        if (pagenum < doc->pageCount()) printer_->newPage();
    }
    delete doc;
}

///////////////////////////////////////////////////////////////////////////////
// preview()
///////////////////////////////////////////////////////////////////////////////
/// Displays a print preview dialog. If caption is not empty, it will be used
/// as the title of the dialog. The preview dialog will have buttons for
/// zooming and page setup. If the user accepts the dialog, the print()
/// method will be called.
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::preview(const QTextDocument *document,
                          const QString &caption)
{
    if (!document) return;

    PreviewDialog *dialog = new PreviewDialog(document, this, parent_);
    dialog->setWindowTitle(caption.isEmpty() ? "Print Preview" : caption);

    if (dialog->exec() == QDialog::Rejected) return;

    print(document);

    delete dialog;
}

///////////////////////////////////////////////////////////////////////////////
// private methods
///////////////////////////////////////////////////////////////////////////////

// paperRect() ////////////////////////////////////////////////////////////////
// Return the size of the paper, adjusted for DPI

QRectF TextPrinter::paperRect(QPaintDevice *device)
{
    // calculate size of paper
    QRectF rect = printer_->paperRect();
    // adjust for DPI
    rect.setWidth(rect.width() *
                  device->logicalDpiX() / printer_->logicalDpiX());
    rect.setHeight(rect.height() *
                  device->logicalDpiY() / printer_->logicalDpiY());

    return rect;
}

// contentRect() //////////////////////////////////////////////////////////////
// calculate the rect for the content block

QRectF TextPrinter::contentRect(QPaintDevice *device)
{
    // calculate size of content (paper - margins)
    QRectF rect = paperRect(device);

    rect.adjust(mmToInches(leftmargin_) * device->logicalDpiX(),
                mmToInches(topmargin_) * device->logicalDpiY(),
                -mmToInches(rightmargin_) * device->logicalDpiX(),
                -mmToInches(bottommargin_) * device->logicalDpiY());

    // header
    if (headersize_ > 0) {
        rect.adjust(0, mmToInches(headersize_) * device->logicalDpiY(), 0, 0);
        rect.adjust(0, mmToInches(spacing_) * device->logicalDpiY(), 0, 0);
    }
    // footer
    if (footersize_ > 0) {
        rect.adjust(0, 0, 0, -mmToInches(footersize_) * device->logicalDpiY());
        rect.adjust(0, 0, 0, -mmToInches(spacing_) * device->logicalDpiY());
    }

    return rect;
}

// headerRect() //////////////////////////////////////////////////////////////
// calculate the rect for the header block

QRectF TextPrinter::headerRect(QPaintDevice *device)
{
    QRectF rect = paperRect(device);
    rect.adjust(mmToInches(leftmargin_) * device->logicalDpiX(),
                mmToInches(topmargin_) * device->logicalDpiY(),
                -mmToInches(rightmargin_) * device->logicalDpiX(), 0);

    rect.setBottom(rect.top() +
                   mmToInches(headersize_) * device->logicalDpiY());

    return rect;
};

// footerRect() ///////////////////////////////////////////////////////////////
// calculate the rect for the footer block

QRectF TextPrinter::footerRect(QPaintDevice *device)
{
    QRectF rect = paperRect(device);
    rect.adjust(mmToInches(leftmargin_) * device->logicalDpiX(), 0,
                -mmToInches(rightmargin_) * device->logicalDpiX(),
                -mmToInches(bottommargin_) * device->logicalDpiY());

    rect.setTop(rect.bottom() -
                mmToInches(headersize_) * device->logicalDpiY());

    return rect;
};

// paintPage() ////////////////////////////////////////////////////////////////
// paint an individual page of the document to the painter

void TextPrinter::paintPage(QPainter *painter,
                           QTextDocument *document,
                           int pagenum)
{
    QRectF rect;

    // header
    if (headersize_ > 0) {
        painter->save();

        rect = headerRect(painter->device());
        if (headerrule_) {
            painter->setPen(QPen(Qt::black, 1.0));
            painter->drawLine(rect.bottomLeft(), rect.bottomRight());
        }

        painter->translate(rect.left(), rect.top());
        QRectF clip(0, 0, rect.width(), rect.height());
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(headertext_);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());

        // align text to bottom
        double newtop = clip.bottom() - doc.size().height();
        clip.setHeight(doc.size().height());
        painter->translate(0, newtop);

        doc.drawContents(painter, clip);
        
        painter->restore();
    }

    // footer
    if (footersize_ > 0) {
        painter->save();

        rect = footerRect(painter->device());
        if (footerrule_) {
            painter->setPen(QPen(Qt::black, 1.0));
            painter->drawLine(rect.topLeft(), rect.topRight());
        }

        painter->translate(rect.left(), rect.top());
        QRectF clip(0, 0, rect.width(), rect.height());
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(footertext_);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());
        doc.drawContents(painter, clip);
        
        painter->restore();
    }

    // content
    painter->save();

    rect = contentRect(painter->device());
    painter->translate(rect.left(), rect.top() - (pagenum-1) * rect.height());
    QRectF clip(0, (pagenum-1) * rect.height(), rect.width(), rect.height());

    document->drawContents(painter, clip);

    painter->restore();
}
