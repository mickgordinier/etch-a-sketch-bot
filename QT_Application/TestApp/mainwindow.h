#pragma once

#include <QWidget>
#include <QLabel>
#include <QChartView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
namespace QtCharts {
class QChartView;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void updateDisplay(bool shouldIncrement);

private:
    void setupLayouts();
    void setupTopLayout();
    void setupBottomLayout();

    QChartView* generateChart();

    int displayIdx = 0;

    QStackedWidget *stackedWidget;

    QLabel *originalImageLabel = nullptr;
    QLabel *connectedImageLabel = nullptr;
    QChartView *stepChartView = nullptr;

    QHBoxLayout *topLayout = nullptr;
    QHBoxLayout *bottomLayout = nullptr;
    QVBoxLayout *mainLayout = nullptr;
};
