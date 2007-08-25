///////////////////////////////////////////////////////////////////////////////
// previewdialog.h
// -------------------
// Copyright (c) 2007 David Johnson <david@usermode.org>
// Please see the header file for copyright and license information.
///////////////////////////////////////////////////////////////////////////////

#include "previewdialog.h"
#include "textprinter.h"

#include <QAbstractScrollArea>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPageSetupDialog>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextFrame>
#include <QToolButton>
#include <QVBoxLayout>

const int spacing = 30;

///////////////////////////////////////////////////////////////////////////////
// PreviewDialog                                                             //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PreviewDialog()
///////////////////////////////////////////////////////////////////////////////
/// Construct a PreviewDialog object. "document" is the QTextDocument to
/// print, and "textprinter" is the TextPrinter object defining the page.
///////////////////////////////////////////////////////////////////////////////

PreviewDialog::PreviewDialog(const QTextDocument *document,
                             TextPrinter *textprinter,
                             QWidget *parent)
    : QDialog(parent), view(0)
{
    // widgets
    view = new PreviewWidget(document, textprinter, this);

    QToolButton *zoominbutton = new QToolButton(this);
    zoominbutton->setText(tr("Zoom in"));
    zoominbutton->setToolTip(tr("Zoom in"));
    zoominbutton->setIcon(QIcon(":/pics/icons/viewzoomin.png"));

    QToolButton *zoomoutbutton = new QToolButton(this);
    zoomoutbutton->setText(tr("Zoom out"));
    zoomoutbutton->setToolTip(tr("Zoom out"));
    zoomoutbutton->setIcon(QIcon(":/pics/icons/viewzoomout.png"));

    QToolButton *setupbutton = new QToolButton(this);
    setupbutton->setText(tr("Page setup"));
    setupbutton->setToolTip(tr("Page setup"));
    setupbutton->setIcon(QIcon(":/pics/icons/filepagesetup.png"));

    QDialogButtonBox *buttonbox = new QDialogButtonBox(this);
    buttonbox->setOrientation(Qt::Horizontal);
    buttonbox->addButton(tr("Print"), QDialogButtonBox::AcceptRole);
    buttonbox->addButton(QDialogButtonBox::Cancel);

    // layouts
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    QHBoxLayout *buttonlayout = new QHBoxLayout();
    
    buttonlayout->addWidget(zoominbutton);
    buttonlayout->addWidget(zoomoutbutton);
    buttonlayout->addStretch(10);
    buttonlayout->addWidget(setupbutton);

    mainlayout->addLayout(buttonlayout);
    mainlayout->addWidget(view, 10);
    mainlayout->addWidget(buttonbox);

    resize(800, 600);

    connect(zoominbutton, SIGNAL(clicked()), view, SLOT(zoomIn()));
    connect(zoomoutbutton, SIGNAL(clicked()), view, SLOT(zoomOut()));
    connect(setupbutton, SIGNAL(clicked()), view, SLOT(pageSetup()));
    connect(buttonbox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(reject()));
}

///////////////////////////////////////////////////////////////////////////////
// ~PreviewDialog()
///////////////////////////////////////////////////////////////////////////////
/// Destroy the dialog.
///////////////////////////////////////////////////////////////////////////////

PreviewDialog::~PreviewDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
// PreviewWidget                                                             //
///////////////////////////////////////////////////////////////////////////////

// PreviewWidget() ////////////////////////////////////////////////////////////
// constructor

PreviewWidget::PreviewWidget(const QTextDocument *document,
                             TextPrinter *textprinter,
                             QWidget *parent)
    : QAbstractScrollArea(parent), doc(document->clone(this)),
      text(textprinter), zoom(1.0), mousepos(), scrollpos()
{
    viewport()->setBackgroundRole(QPalette::Dark);
    verticalScrollBar()->setSingleStep(25);
    horizontalScrollBar()->setSingleStep(25);
    setupDocument();
}

// ~PreviewWidget() ///////////////////////////////////////////////////////////
// destructor

PreviewWidget::~PreviewWidget()
{
}

// setupDocument() ////////////////////////////////////////////////////////////
// setup the document

void PreviewWidget::setupDocument()
{
    doc->setUseDesignMetrics(true);
    doc->documentLayout()->setPaintDevice(viewport());

    // set document size
    doc->setPageSize(text->contentRect(viewport()).size());

    // dump existing margin (if any)
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(0);
    doc->rootFrame()->setFrameFormat(fmt);
}

// updateView() ///////////////////////////////////////////////////////////////
// update the view
void PreviewWidget::updateView()
{
    resizeEvent(0);
    viewport()->update();
}

// pageSetup() ////////////////////////////////////////////////////////////////
// display page setup dialog

void PreviewWidget::pageSetup()
{
    QPageSetupDialog *dialog;
    dialog = new QPageSetupDialog(text->printer_,
                                  qobject_cast<QWidget*>(parent()));
    if (dialog->exec() == QDialog::Accepted) {
        setupDocument();
        updateView();
    }
    delete dialog;
}

// zoomIn() ///////////////////////////////////////////////////////////////////
// zoom in to view

void PreviewWidget::zoomIn()
{
    zoom += 0.2;
    // update
    resizeEvent(0);
    viewport()->update();

}

// zoomOut() //////////////////////////////////////////////////////////////////
// zoom out from view

void PreviewWidget::zoomOut()
{
    zoom = qMax(zoom - 0.2, 0.2);
    // update
    resizeEvent(0);
    viewport()->update();
}

// mousePressEvent() //////////////////////////////////////////////////////////
// click to start view drag

void PreviewWidget::mousePressEvent(QMouseEvent *e)
{
    mousepos = e->pos();
    scrollpos.rx() = horizontalScrollBar()->value();
    scrollpos.ry() = verticalScrollBar()->value();
    e->accept();
}

// mouseMoveEvent() ///////////////////////////////////////////////////////////
// drag view

void PreviewWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (mousepos.isNull()) {
        e->ignore();
        return;
    }

    horizontalScrollBar()->setValue(scrollpos.x() - e->pos().x() + mousepos.x());
    verticalScrollBar()->setValue(scrollpos.y() - e->pos().y() + mousepos.y());
    horizontalScrollBar()->update();
    verticalScrollBar()->update();
    e->accept();
}

// mouseReleaseEvent() ////////////////////////////////////////////////////////
// release finished view drag

void PreviewWidget::mouseReleaseEvent(QMouseEvent *e)
{
    mousepos = QPoint();
    e->accept();
}

// paintEvent() ///////////////////////////////////////////////////////////////
// paint document on widget

void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());

    painter.translate(-horizontalScrollBar()->value(),
                      -verticalScrollBar()->value());
    painter.translate(spacing, spacing);

    for (int n = 1; n <= doc->pageCount(); n++) {
        painter.save();
        painter.scale(zoom, zoom);

        double paperwidth = text->paperRect(viewport()).width();
        double paperheight = text->paperRect(viewport()).height();

        // draw outline and shadow
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        painter.drawRect(QRectF(0, 0, paperwidth, paperheight));
        painter.setBrush(Qt::NoBrush);

        painter.drawLine(QLineF(paperwidth+1, 2, paperwidth+1, paperheight+2));
        painter.drawLine(QLineF(paperwidth+2, 2, paperwidth+2, paperheight+2));
        painter.drawLine(QLineF(2, paperheight+1, paperwidth, paperheight+1));
        painter.drawLine(QLineF(2, paperheight+2, paperwidth, paperheight+2));

        text->paintPage(&painter, doc, n);

        painter.restore();
        painter.translate(0, spacing + text->paperRect(viewport()).height() * zoom);
    }
}

// resizeEvent() //////////////////////////////////////////////////////////////
// view has resized

void PreviewWidget::resizeEvent(QResizeEvent *)
{
    QSize docsize;
    const QSize viewsize = viewport()->size();

    docsize.setWidth(qRound(text->paperRect(viewport()).width() *
                            zoom + 2 * spacing));
    int pagecount = doc->pageCount();
    docsize.setHeight(qRound(pagecount * text->paperRect(viewport()).height() *
                             zoom + (pagecount + 1) * spacing));

    horizontalScrollBar()->setRange(0, docsize.width() - viewsize.width());
    horizontalScrollBar()->setPageStep(viewsize.width());

    verticalScrollBar()->setRange(0, docsize.height() - viewsize.height());
    verticalScrollBar()->setPageStep(viewsize.height());
}


