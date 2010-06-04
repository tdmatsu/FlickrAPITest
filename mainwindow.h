#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FlickrModel;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    FlickrModel* m_FlickrModel;

    void loadSettings();
    void saveSettings();
    void setProxy(bool enabled);


private slots:
    void on_grpProxy_clicked(bool checked);
    void on_txtSecret_editingFinished();
    void on_txtApiKey_editingFinished();
    void on_btnToken_clicked();
    void on_btnAuth_clicked();

};

#endif // MAINWINDOW_H
