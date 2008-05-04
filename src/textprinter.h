///////////////////////////////////////////////////////////////////////////////
// textprinter.h
// -------------------
// Copyright (c) 2007 David Johnson <david@usermode.org>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#ifndef TEXTPRINTER_H
#define TEXTPRINTER_H

#include <QObject>
#include <QPrinter>

class QTextDocument;
class QWidget;

///////////////////////////////////////////////////////////////////////////////
/// \class TextPrinter
/// \version 0.2
///
/// TextPrinter is a printing utility class. It provides methods to print,
/// preview, and export to PDF. The page format, including header and footers,
/// can be defined. The content to be printed is provided as a QTextDocument
/// object.
///
/// \todo support for page numbering and other page variables
///////////////////////////////////////////////////////////////////////////////

class TextPrinter : public QObject
{
    Q_OBJECT
public:
    /// Default constructor
    explicit TextPrinter(QObject *parent=0);
    /// Destructor
    ~TextPrinter();

    /// Print the document
    void print(const QTextDocument *document,
               const QString &caption=QString());
    /// Export the document to PDF
    void exportPdf(const QTextDocument *document,
                   const QString &caption=QString(),
                   const QString &filename=QString());
    /// Display the document in a preview dialog
    void preview(const QTextDocument *document,
                 const QString &caption=QString());

    /// Get page size
    QPrinter::PageSize pageSize();
    /// Set page size
    void setPageSize(QPrinter::PageSize size);
    /// Get page orientation
    QPrinter::Orientation orientation();
    /// Set page orientation
    void setOrientation(QPrinter::Orientation orientation);

    /// Get left margin width
    double leftMargin();
    /// Set left margin width
    void setLeftMargin(double margin);
    /// Get right margin width
    double rightMargin();
    /// Set right margin width
    void setRightMargin(double margin);
    /// Get top margin width
    double topMargin();
    /// Set top margin width
    void setTopMargin(double margin);
    /// Get bottom margin width
    double bottomMargin();
    /// Set bottom margin width
    void setBottomMargin(double margin);
    /// Set all margins
    void setMargins(double margin);

    /// Get spacing between content and header and footer
    double spacing();
    /// Set spacing between content and header and footer
    void setSpacing(double spacing);

    /// Get header size
    double headerSize();
    /// Set header size
    void setHeaderSize(double size);
    /// Is header rule enabled
    bool headerRule();
    /// Enable header rule
    void setHeaderRule(bool on);
    /// Get header text
    const QString &headerText();
    /// Set header text
    void setHeaderText(const QString &text);

    /// Get footer size
    double footerSize();
    /// Set footer size
    void setFooterSize(double size);
    /// Is footer rule enabled
    bool footerRule();
    /// Enable footer rule
    void setFooterRule(bool on);
    /// Get footer text
    const QString &footerText();
    /// Set footer text
    void setFooterText(const QString &text);

private:
    // not copyable
    TextPrinter(const TextPrinter&);
    TextPrinter &operator=(const TextPrinter&);

    // return paper rect
    QRectF paperRect(QPaintDevice *device);
    // return printable rects
    QRectF contentRect(QPaintDevice *device);
    QRectF headerRect(QPaintDevice *device);
    QRectF footerRect(QPaintDevice *device);

    // paint specific page
    void paintPage(QPainter *painter,
                   QTextDocument *document,
                   int pagenum);
private slots:
    // common print routine
    void print(QPrinter *printer);

private:
    QWidget *parent_;
    QPrinter *printer_;

    QTextDocument *tempdoc_;

    double leftmargin_;
    double rightmargin_;
    double topmargin_;
    double bottommargin_;
    double spacing_;

    double headersize_;
    bool headerrule_;
    QString headertext_;
    double footersize_;
    bool footerrule_;
    QString footertext_;
};

#endif // TEXTPRINTER_H
