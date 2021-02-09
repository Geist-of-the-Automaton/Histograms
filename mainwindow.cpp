#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Initializing basic window stuff.
    ui->setupUi(this);
    statusBar()->hide();
    outOf = 200;
    total = max = 0.0;
    QSize size (bins * 2, outOf * 2 + 40);
    setGeometry(QRect(QPoint(0,0), size));
    setMinimumHeight(240);
    setMaximumWidth(size.width());
    setWindowTitle("CSCE 590 Project - Auden Childress");
    menubar = new QMenuBar(this);
    menu = new QMenu("File");
    menubar->addMenu(menu);
    QAction *action1 = menu->addAction("Load Image");
    connect(action1, &QAction::triggered, this, [=]() { this->menuClick(Load); });
    // See comment regarding commented-out code at bottom of file.
    /*
    QAction *action2 = menu->addAction("Export As");
    connect(action2, &QAction::triggered, this, [=]() { this->menuClick(Export); });
    */
    ql = new QLabel();
    setCentralWidget(ql);
    setMenuBar(menubar);
}

MainWindow::~MainWindow()
{
    destroyAllWindows();
    hide();
    delete ql;
    delete menu;
    delete menubar;
    delete ui;
}

void MainWindow::menuClick(Function function) {
    if (function == Load) {
        // Check if the load was successful.
        string fileName = QFileDialog::getOpenFileName(this, tr("Load Image"), "/", tr("Image Files (*.bmp *.dib *.jpeg *.jpg *.png *.ppm *.tiff *.tif)")).toStdString();
        if (fileName == "")
            return;
        Mat image = imread(fileName);
        if (image.empty())
            return;
        // Create variables for calculating histograms, and those essential to the assignment report. Fill with default values.
        int depth = pow(2, 8 * image.elemSize1());
        int depthDivider = depth / bins;
        destroyAllWindows();
        string simpleFN = fileName.substr(fileName.find_last_of('/') + 1);
        setWindowTitle("Histograms");
        imshow(simpleFN, image);
        Mat invertImage = image.clone();
        QImage qi (QSize(bins * 2, outOf * 2 + 20), QImage::Format_ARGB32_Premultiplied);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < bins; ++j) {
                histo[i][j] = 0;
                for (int k = qi.height() - 1; k >= 0; --k) {
                    qi.setPixelColor(j, k, 0xFF000000);
                    qi.setPixelColor(j + bins, k, 0xFF000000);
                }
            }
        int minI = INT_MAX, maxI = 0, meanI = 0;
        max = 0.0;
        // Fill the array(s) tht the histograms will be constructed from. Invert color after reading for intensity inverted image.
        for (int y = 0; y < image.rows; ++y)
            for (int x = 0; x < image.cols; ++x) {
                Vec3b colors = image.at<Vec3b>(Point(x, y));
                int intensity = 0;
                for (unsigned char i = 0; i < 3; ++i) {
                    int bin = colors[i] / (depthDivider);
                    ++histo[i][bin];
                    if (histo[i][bin] > max && bin != 0 && bin != bins - 1) // I ignore the first and last because they usually have values that make the drawn histogram scale very small.
                        max = static_cast<float>(histo[i][bin]);
                    intensity += colors[i];
                    colors[i] = (depth - 1) - colors[i];
                }
                meanI += intensity;
                float intensityF = static_cast<float>(intensity) / 3.0;
                intensity /= 3;
                if (intensityF - static_cast<float>(intensity) >= 0.5)    // Calculate closest bin for intensity.
                    ++intensity;
                if (intensity > maxI)
                    maxI = intensity;
                if (intensity < minI)
                    minI = intensity;
                ++histo[3][intensity];
                invertImage.at<Vec3b>(Point(x, y)) = colors;
            }
        meanI /= static_cast<int>(3 * image.total());
        // Write inverted image to file.
        imshow("Intensity Inverted " + simpleFN, invertImage);
        string iiFile = fileName.substr(0, fileName.find_last_of('.')) + "_Intensity-Inverted" + fileName.substr(fileName.find_last_of('.'));
        imwrite(iiFile, invertImage);
        // Draw histograms.
        total = static_cast<float>(image.total());
        max = total / (max + 1.0);
        for (int x = 0; x < bins; ++x) {
            QRgb value = bins / 2 + static_cast<QRgb>(x / 2);
            for  (int j = 0; j < 4; ++j) {
                QRgb color = 0xFF000000;
                if (j != 3)
                    color += (value << (8 * (2 - j)));
                else
                    for (unsigned char i= 0; i < 3; ++i)
                        color += value << (8 * i);
                int row = j / 2;
                int yOffset = 1 + row, xTrue = (bins * (j % 2)) + x;
                int stop = yOffset * outOf - static_cast<int>((max * static_cast<float>(outOf * histo[j][x])) / total);
                if (stop < outOf * row)
                    stop = outOf * row;
                for (int y = yOffset * outOf - 1; y >= stop; --y)
                    qi.setPixelColor(xTrue, y, color);
            }
        }
        // Draw crosshair where the histograms meet. Draw image specs at bottom.
        QPixmap qpix;
        QPainter p;
        p.begin(&qi);
        p.setPen(QPen(Qt::white));
        p.setFont(QFont("Arial", 9, QFont::ExtraLight));
        info = "Dims: " + to_string(image.cols) + "x" + to_string(image.rows) + ", Intensity Min: " + to_string(minI) + ", Max: " + to_string(maxI) + ", Mean: " + to_string(meanI) + ", Depth Bytes: " + to_string(image.elemSize1());
        p.drawText(qi.rect(), Qt::AlignBottom, QString(info.c_str()));
        p.drawLine(bins - crosshairSize, outOf, bins + crosshairSize, outOf);
        p.drawLine(bins, outOf - crosshairSize, bins, outOf + crosshairSize);
        p.end();
        qpix.convertFromImage(qi);
        ql->setPixmap(qpix);
    }
    else if (function == Export) {
        // Originally, the program allowed the user to export a copy of the photo in another format, but I did not want to convolute the core functionality, so I removed it.
        /*
        string fileName = QFileDialog::getSaveFileName(this, tr("Export As"), "/", tr("Image Files (*.bmp *.dib *.jpeg *.jpg *.png *.ppm *.tiff *.tif)")).toStdString();
        if (fileName == "")
            return;
        */
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if (total == 0.0)
        return;
    outOf = (height() - 40) / 2;
    QImage qi (QSize(bins * 2, outOf * 2 + 20), QImage::Format_ARGB32_Premultiplied);
    for (int x = 0; x < qi.width(); ++x)
        for (int y = 0; y < qi.height(); ++y)
            qi.setPixelColor(x, y, 0xFF000000);
    for (int x = 0; x < bins; ++x) {
        QRgb value = bins / 2 + static_cast<QRgb>(x / 2);
        for  (int j = 0; j < 4; ++j) {
            QRgb color = 0xFF000000;
            if (j != 3)
                color += (value << (8 * (2 - j)));
            else
                for (unsigned char i= 0; i < 3; ++i)
                    color += value << (8 * i);
            int row = j / 2;
            int yOffset = 1 + row, xTrue = (bins * (j % 2)) + x;
            int stop = yOffset * outOf - static_cast<int>((max * static_cast<float>(outOf * histo[j][x])) / total);
            if (stop < outOf * row)
                stop = outOf * row;
            for (int y = yOffset * outOf - 1; y >= stop; --y)
                qi.setPixelColor(xTrue, y, color);
        }
    }
    QPixmap qpix;
    QPainter p;
    p.begin(&qi);
    p.setPen(QPen(Qt::white));
    p.setFont(QFont("Arial", 9, QFont::ExtraLight));
    p.drawText(qi.rect(), Qt::AlignBottom, QString(info.c_str()));
    p.drawLine(bins - crosshairSize, outOf, bins + crosshairSize, outOf);
    p.drawLine(bins, outOf - crosshairSize, bins, outOf + crosshairSize);
    p.end();
    qpix.convertFromImage(qi);
    ql->setPixmap(qpix);
}
