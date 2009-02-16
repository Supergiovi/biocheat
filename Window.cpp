#include "Window.h"
#include "Capture.h"
#include "Classifier.h"
#include "Recognizer.h"
#include "ui_Window.h"

#include <QImage>
#include <QMouseEvent>
#include <QDesktopWidget>

#define DEFAULT_WIDTH 252
#define DEFAULT_HEIGHT 330

Window::Window( QWidget *parent )
    : QWidget( parent )
    , ui( new Ui::WindowForm )
{
    // create ui
    ui->setupUi( this );
    ui->visualizer->installEventFilter( this );
    QDesktopWidget dw;
    ui->xOffset->setMaximum( dw.width() );
    ui->yOffset->setMaximum( dw.height() );
    ui->regionWidth->setMaximum( dw.width() );
    ui->regionHeight->setMaximum( dw.height() );
    ui->xOffset->setValue( (dw.width() - DEFAULT_WIDTH) / 2 );
    ui->yOffset->setValue( (dw.height() - DEFAULT_HEIGHT) / 2 );
    ui->regionWidth->setValue( DEFAULT_WIDTH );
    ui->regionHeight->setValue( DEFAULT_HEIGHT );
    connect( ui->xOffset, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->yOffset, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->regionWidth, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->regionHeight, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->frequency, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->hBlocks, SIGNAL(valueChanged(int)), this, SLOT(slotRecParamsChanged()) );
    connect( ui->vBlocks, SIGNAL(valueChanged(int)), this, SLOT(slotRecParamsChanged()) );

    // create and train the classifier
    m_classifier = new Classifier( QSize( 30, 30 ), this );
    for ( int i = 0; i < 7; i++ )
        m_classifier->addClass( i, QImage( QString( ":/data/class%1.png" ).arg( i ), "PNG" ) );

    // create the recognizer
    m_recognizer = new Recognizer( m_classifier, this );
    slotRecParamsChanged();

    // create the capture
    m_capture = new Capture( this );
    connect( m_capture, SIGNAL(gotPixmap(const QPixmap &)),
             this, SLOT(slotProcessPixmap(const QPixmap &)) );
    slotCapParamsChanged();
}

Window::~Window()
{
    delete m_recognizer;
    delete m_capture;
    delete m_classifier;
    delete ui;
}

bool Window::eventFilter( QObject * object, QEvent * event )
{
    // intercept clicks on the Original image label
    if ( object == ui->visualizer && event->type() == QEvent::MouseButtonPress ) {
        QMouseEvent * me = static_cast<QMouseEvent *>( event );

        QPixmap p = ui->visualizer->originalPixmap().copy( me->pos().x() - 15, me->pos().y() - 15, 30, 30 );
        m_classifier->classify( p.toImage() );

    }


    return false;
}

void Window::slotCapParamsChanged()
{
    QRect captureRect( ui->xOffset->value(), ui->yOffset->value(), ui->regionWidth->value(), ui->regionHeight->value() );
    int adjW = captureRect.width() % 30;
    int adjH = captureRect.height() % 30;
    m_capture->setGeometry( captureRect.adjusted( 0, 0, adjW, adjH ) );
    m_capture->setFrequency( ui->frequency->value() );
}

void Window::slotRecParamsChanged()
{
    m_recognizer->setup( ui->hBlocks->value(), ui->vBlocks->value() );
}

void Window::slotProcessPixmap( const QPixmap & pixmap )
{
    // show original image
    ui->visualizer->setMinimumSize( pixmap.size() );
    if ( ui->display1->isChecked() )
        ui->visualizer->setOriginalPixmap( pixmap );

    // process image
    bool displayRec = ui->display2->isChecked();
    m_recognizer->recognize( pixmap, displayRec );
    if ( displayrec )
        ui->visualizer->setOriginalPixmap( m_recognizer->output() );

    // show results
    if ( ui->display3->isChecked() ) {

    }
}
