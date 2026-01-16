#include "mainwindow.h"

#include <QPushButton>
#include <QSlider>
#include <QPixmap>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QChart>
#include <QChartView>
#include <QLineSeries>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setupLayouts();
}


void MainWindow::setupLayouts()
{
    topLayout = new QHBoxLayout;
    bottomLayout = new QHBoxLayout;
    mainLayout = new QVBoxLayout(this);

    setupTopLayout();
    setupBottomLayout();

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
}


void MainWindow::setupTopLayout()
{
    auto *leftButton = new QPushButton("←");
    leftButton->setFixedSize(50, 50);
    leftButton->setStyleSheet("font-size: 30px;");

    auto *rightButton = new QPushButton("→");
    rightButton->setFixedSize(50, 50);
    rightButton->setStyleSheet("font-size: 30px;");

    connect(leftButton, &QPushButton::clicked,
            this, [this]() { updateDisplay(false); });

    connect(rightButton, &QPushButton::clicked,
            this, [this]() { updateDisplay(true); });

    originalImageLabel = new QLabel;
    originalImageLabel->setPixmap(
        QPixmap("C:/Users/Mickg/OneDrive/Desktop/hobby_projects/etch-a-sketch-bot/test_images/cat.bmp")
        );

    connectedImageLabel = new QLabel;
    connectedImageLabel->setPixmap(
        QPixmap("C:/Users/Mickg/OneDrive/Desktop/hobby_projects/etch-a-sketch-bot/test_images/cat.bmp")
        );

    stepChartView = generateChart();

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(originalImageLabel);  // index 0
    stackedWidget->addWidget(connectedImageLabel); // index 1
    stackedWidget->addWidget(stepChartView);       // index 2

    topLayout->addWidget(leftButton);
    topLayout->addWidget(stackedWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(rightButton);

    connectedImageLabel->hide();
    stepChartView->hide();
}



void MainWindow::setupBottomLayout()
{
    auto *canvas = new QLabel("Canvas / Image Area");
    canvas->setAlignment(Qt::AlignCenter);
    canvas->setStyleSheet("background-color: black; color: white;");
    canvas->setMinimumHeight(50);

    auto *slider = new QSlider(Qt::Horizontal);

    bottomLayout->addWidget(canvas);
    bottomLayout->addWidget(slider);
}



QChartView* MainWindow::generateChart()
{
    auto *series = new QLineSeries();
    series->append(0, 0);
    series->append(1, 2);
    series->append(2, 1);

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple Graph");
    chart->createDefaultAxes();

    auto *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    return chartView;
}


void MainWindow::updateDisplay(bool shouldIncrement)
{
    if ((displayIdx == 0) && !shouldIncrement) {
        displayIdx = 2;
    } else if ((displayIdx == 2) && shouldIncrement) {
        displayIdx = 0;
    } else {
        displayIdx += shouldIncrement ? 1 : -1;
    }

    stackedWidget->setCurrentIndex(displayIdx);
    qDebug() << "Display index:" << displayIdx;
}
