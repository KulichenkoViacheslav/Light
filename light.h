#ifndef LIGHT_H
#define LIGHT_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QMenu>
#include <QColor>
#include <QPropertyAnimation>
#include <QTimer>
#include "configs.h"

struct Parameters {
    int numberStepsMode;
    int currentStepMode;
    QColor color;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class Light;
}
QT_END_NAMESPACE

class Light : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QColor windowBackgroundColor READ windowBackgroundColor WRITE setWindowBackgroundColor)

public:
    Light(QWidget *parent = nullptr);
    ~Light();

    QColor windowBackgroundColor() const;
    void setWindowBackgroundColor(const QColor &color);
    void startColorTransition(const QColor &startColor, const QColor &endColor, int duration);

public slots:
    void receiveNewDataMode(const Data newDataMode[], int size);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void runApp();
    void stopApp();
    void pauseApp();
    void resumeApp();
    void openSettings();
    void closeApp();
    void collapseApp();
    void selectStepMode(bool nextStep);
    void setColor(const QColor &color, int duration);
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::Light *ui;
    Configs *configs;
    Data dataMode[100];
    Parameters ActiveParameters;

    // Контекстне меню та дії
    QMenu* contextMenu;
    QAction *runAction;
    QAction *stopAction;
    QAction *pauseAction;
    QAction *resumeAction;
    QAction *settingsAction;
    QAction *collapseAction;
    QAction *closeAction;

    QColor m_windowBackgroundColor;
    QColor defaultWindowBackgroundColor;
    struct {
        QTimer *pTimer;
        int remainingTime;
    } saveTimer;

    QPropertyAnimation *pColorAnimation;
};

#endif // LIGHT_H
