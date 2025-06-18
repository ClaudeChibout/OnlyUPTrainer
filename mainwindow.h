#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void processLoaded();
    void refreshList();

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_tp();

    void on_pushButton_save_clicked();

    void on_pushButton_tp_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
