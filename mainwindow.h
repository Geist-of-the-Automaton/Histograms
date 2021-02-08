#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QFileDialog>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <QLabel>
#include <QPainter>
#include <QString>

using cv::imread;
using cv::imwrite;
using cv::imshow;
using cv::destroyAllWindows;
using cv::Mat;
using cv::Vec3b;
using cv::Point;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::to_string;

const int bins = 256;
const int crosshairSize = 5;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum Function {Load, Export};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void resizeEvent(QResizeEvent *event);

private:
    Ui::MainWindow *ui;
    QMenuBar *menubar;
    QMenu *menu;
    QLabel *ql;
    int histo[4][bins];
    string info;
    int outOf;
    float max, total;

public slots:
    void menuClick(Function function);
};
#endif // MAINWINDOW_H
