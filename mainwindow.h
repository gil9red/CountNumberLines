#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void updateStates();

    void on_tButtonSelectRootDir_clicked();
    void on_tButtonFind_clicked();
    void on_tButtonRun_clicked();
    void on_tButtonSaveAs_clicked();

private:
    Ui::MainWindow * ui;
    QStandardItemModel model;
};

#endif // MAINWINDOW_H
