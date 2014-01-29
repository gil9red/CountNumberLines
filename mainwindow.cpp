#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <QFileDialog>
#include <QTime>
#include <QPrinter>
#include <QTextDocumentWriter>
#include <QTextCodec>

// Код взят с http://www.prog.org.ru/topic_13542_0.html
qint64 lineCount(const char* FileName) {
  QFile File(FileName);
  if (!File.open(QFile::ReadOnly)) return -1;

  char   buff[1024*128];
  qint64 lcount = 0;
  int    len;

  char prev = 0;
  char cur  = 0;
  for(;;) {
    len = File.read(buff, sizeof(buff));
    if (File.error()) return -1;
    if(!len) {break;}

    for (int i=0; i<len; ++i) {
      cur = buff[i];
      if      (cur  == 10) {++lcount;}
      else if (prev == 13) {++lcount;}
      prev = cur;
    }
  }
  if (cur == 13) {++lcount;}
  return lcount + 1;
}

/// PUBLIC
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->files->setModel( &model );

    connect( ui->lEditRootDir, SIGNAL( textChanged(QString) ), SLOT( updateStates() ) );

    QDir dir( qApp->applicationDirPath() );
    dir.cdUp();
    ui->lEditRootDir->setText( dir.path() );

    updateStates();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/// PRIVATE SLOTS
void MainWindow::updateStates()
{
    bool isEmpty = model.rowCount() == 0;
    bool isDir = QFileInfo( ui->lEditRootDir->text() ).isDir();

    ui->tButtonRun->setEnabled( !isEmpty );
    ui->tButtonFind->setEnabled( isDir );
}
void MainWindow::on_tButtonSelectRootDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory( this );
    if ( dir.isEmpty() )
        return;

    ui->lEditRootDir->setText( dir );
    updateStates();
}
void MainWindow::on_tButtonFind_clicked()
{       
    const QStringList & filters = ui->lEditFilters->text().split( " " );
    const QString & path = ui->lEditRootDir->text();

    qApp->setOverrideCursor( Qt::WaitCursor );
    model.clear();
    QDirIterator dir( path, filters, QDir::Files, QDirIterator::Subdirectories );
    while( dir.hasNext() )
    {
        QStandardItem * item = new QStandardItem( dir.next() );
        item->setCheckable( true );
        item->setCheckState( Qt::Checked );
        model.appendRow( item );
    }
    qApp->restoreOverrideCursor();

    updateStates();
}
void MainWindow::on_tButtonRun_clicked()
{
    QString html;
    html += "<table border=1>";
    html += "<caption align=center>Количество строк в файлах</caption>";
    html += "<tr> <th>Строк</th> <th>Файл</th> </tr>";

    qApp->setOverrideCursor( Qt::WaitCursor );
    uint sum = 0;
    uint files = 0;
    QTime time;
    time.start();
    for ( int row = 0; row < model.rowCount(); row++ )
    {
        QStandardItem * item = model.item( row, 0 );

        if ( item->checkState() == Qt::Checked )
        {
            files++;
            QString path = item->text();
            int count = lineCount( path.toAscii() );
            sum += count;
            html += QString( "<tr> <td>%1</td> <td>%2</td> </tr>" ).arg( count ).arg( path );
        }
    }
    html += "</table>";
    html += "<br><br>";
    html += "<font size=4><b>Результат:</b></font><br>";
    html += QString( "Файлов: %1<br>" ).arg( files );
    html += QString( "Всего строк: %1<br>" ).arg( sum );
    html += QString( "Время выполнения: %1 миллисекунд" ).arg( time.elapsed() );

    ui->tBrowserInfo->setText( html );
    qApp->restoreOverrideCursor();

    updateStates();
}
void MainWindow::on_tButtonSaveAs_clicked()
{
    QString filters;
    filters += "Html ( *.html )\n";
    filters += "Pdf ( *.pdf )\n";
    filters += "Все ( *.* )";

    QString path = QFileDialog::getSaveFileName( this, "Сохранить как...", QString( "Отчет_%1" ).arg( QTime::currentTime().toString( "hh-mm-ss" ) ), filters );
    if ( path.isEmpty() )
        return;

    const QString & suffix = QFileInfo( path ).suffix();
    if ( suffix.contains( "pdf", Qt::CaseInsensitive ) )
    {
        QPrinter printer;
        printer.setOutputFileName( path );
        ui->tBrowserInfo->print( &printer );

    }else
    {
        QTextDocumentWriter textDocumentWriter;
        textDocumentWriter.setFormat( suffix.toAscii() );
        textDocumentWriter.setCodec( QTextCodec::codecForName( "utf8" ) );
        textDocumentWriter.setFileName( path );
        textDocumentWriter.write( ui->tBrowserInfo->document() );
    }
}
