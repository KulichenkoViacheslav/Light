#include "light.h"
#include "./ui_light.h"
#include "configs.h"
#include <QMessageBox>
#include <QPainter>
#include <QDateTime>

Light::Light(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Light)
{
    ui->setupUi(this);

    // Розгортання вікна на весь екран
    this->showFullScreen();

    // Створення контекстного меню та його пунктів
    contextMenu = new QMenu(this);
    runAction = contextMenu->addAction("Розпочати");
    stopAction = contextMenu->addAction("Завершити");
    pauseAction = contextMenu->addAction("Зупинити");
    resumeAction = contextMenu->addAction("Продовжити");
    settingsAction = contextMenu->addAction("Налаштування");
    collapseAction = contextMenu->addAction("Згорнути");
    closeAction = contextMenu->addAction("Закрити");

    // Встановлення стилю для контекстного меню
    QString styleSheet = "QMenu { background-color: #ffffff; color: #000000; font-size: 16px; }"; // Фон - білий, текст - чорний, розмір шрифту - 16px
    styleSheet += "QMenu::item:selected { background-color: #3366cc; color: #ffffff; }"; // Виділений елемент: фон - синій, текст - білий
    styleSheet += "QMenu::item:disabled { color: #808080; }"; // Колір тексту для відключених елементів - сірий
    contextMenu->setStyleSheet(styleSheet);

    // Підключення слотів до дій меню
    connect(runAction, &QAction::triggered, this, &Light::runApp);
    connect(stopAction, &QAction::triggered, this, &Light::stopApp);
    connect(pauseAction, &QAction::triggered, this, &Light::pauseApp);
    connect(resumeAction, &QAction::triggered, this, &Light::resumeApp);
    connect(settingsAction, &QAction::triggered, this, &Light::openSettings);
    connect(collapseAction, &QAction::triggered, this, &Light::collapseApp);
    connect(closeAction, &QAction::triggered, this, &Light::closeApp);

    // Відключення пунктів меню відповідно до наявності даних
    runAction->setVisible(false); // виключений
    stopAction->setVisible(false); // виключений
    pauseAction->setVisible(false); //прихований
    resumeAction->setVisible(false); //прихований

    // Створюємо об'єкт вікна Configs
    configs = new Configs(this);
    // Підключення сигналу setNewDataMode до слоту receiveNewDataMode
    connect(configs, &Configs::setNewDataMode, this, &Light::receiveNewDataMode);
    // Завантажити останню конфігурацію для роботи
    configs->loadDefaultConfigurationFromFile();

    saveTimer.pTimer = NULL;
    pColorAnimation = NULL;
    defaultWindowBackgroundColor = windowBackgroundColor();
}

Light::~Light()
{
    delete ui;
}

void Light::receiveNewDataMode(const Data setDataMode[], int size) {
    // Очищення попередніх даних
    memset(dataMode, 0, sizeof(dataMode));

    // Запис нових даних
    memcpy(dataMode, setDataMode, size * sizeof(Data));
    this->ActiveParameters.numberStepsMode = size;

    if (size > 0) {
        // Активація пунктів меню відповідно до наявності даних
        runAction->setVisible(true);
        stopAction->setVisible(false);
        pauseAction->setVisible(false);
        resumeAction->setVisible(false);
    }
}

void Light::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        // Виклик функції, яка показує контекстне меню
        QPoint point;
        point.setX(event->globalPosition().x());
        point.setY(event->globalPosition().y());
        contextMenu->popup(point);
    }
}

void Light::runApp()
{
    if (ActiveParameters.numberStepsMode > 0) {
        // Задаємо перший крок програми
        ActiveParameters.currentStepMode = 0;

        // Задіюємо параметри для першого кроку
        selectStepMode(false);

        // Активація пунктів меню відповідно до наявності даних
        runAction->setVisible(false);
        stopAction->setVisible(true);
        pauseAction->setVisible(true);
        resumeAction->setVisible(false);

        qDebug() << "RunApp";
    }
    else {
        QMessageBox::information(this, "Увага", "Налаштування програми невірні.");
    }
}

void Light::stopApp()
{
    // Зупинка таймера, якщо він був запущений
    if (saveTimer.pTimer && saveTimer.pTimer->isActive()) {
        saveTimer.pTimer->stop();
        delete saveTimer.pTimer;
        saveTimer.pTimer = nullptr;
    }

    // Зупинка анімації, якщо вона була запущена
    if (pColorAnimation && pColorAnimation->state() == QAbstractAnimation::Running) {
        pColorAnimation->stop();
        delete pColorAnimation;
        pColorAnimation = nullptr;
    }

    // Встановлення коліру фону за замовчуванням
    setWindowBackgroundColor(defaultWindowBackgroundColor);

    // Приховування кнопок у контекстному меню
    runAction->setVisible(true);
    stopAction->setVisible(false);
    pauseAction->setVisible(false);
    resumeAction->setVisible(false);

    qDebug() << "StopApp";
}


void Light::pauseApp()
{
    // Зупинка таймерів і анімації у разі якщо вони були запущені
    if (saveTimer.pTimer != NULL) {
        saveTimer.remainingTime = saveTimer.pTimer->remainingTime();
        if (saveTimer.remainingTime > 0) {
            saveTimer.pTimer->stop();
            qDebug() << "Pause App" << "remainingTime:" << saveTimer.remainingTime;
        }
        else {
            saveTimer.pTimer = NULL;
        }
    }
    if (pColorAnimation != NULL) {
        // Зупинка анімації
        pColorAnimation->pause();
    }

    // Активація пунктів меню відповідно до наявності даних
    runAction->setVisible(false);
    stopAction->setVisible(true);
    pauseAction->setVisible(false);
    resumeAction->setVisible(true);
}

void Light::resumeApp()
{
    // Зупинка таймерів і анімації у разі якщо вони були запущені
    if (saveTimer.pTimer != NULL) {

        saveTimer.pTimer->start(saveTimer.remainingTime);
    }
    if (pColorAnimation != NULL) {
        pColorAnimation->resume();
    }

    // Активація пунктів меню відповідно до наявності даних
    runAction->setVisible(false);
    stopAction->setVisible(true);
    pauseAction->setVisible(true);
    resumeAction->setVisible(false);
}

void Light::openSettings()
{
    // Викликаємо метод show() для відображення вікна
    configs->show();
}

void Light::closeApp()
{
    this->close(); // Закрити вікно
}

void Light::collapseApp()
{
    this->showMinimized(); // Мінімізувати вікно до трею
}

void Light::selectStepMode(bool nextStep)
{
    if (nextStep) {
        ActiveParameters.currentStepMode ++;
        if (ActiveParameters.currentStepMode >= ActiveParameters.numberStepsMode) {
            ActiveParameters.currentStepMode = 0;
        }
    }

    switch (dataMode[ActiveParameters.currentStepMode].status) {
        case Active:
        case Deactivate: {
            setColor(dataMode[ActiveParameters.currentStepMode].color, dataMode[ActiveParameters.currentStepMode].interval);
            break;
        }
        case Transition: {
            QColor currentColor = windowBackgroundColor();
            QColor *nextColor;
            if ((ActiveParameters.currentStepMode + 1) < ActiveParameters.numberStepsMode) {
                nextColor = &dataMode[ActiveParameters.currentStepMode + 1].color;
            }
            else {
                nextColor = &dataMode[0].color;
            }
            startColorTransition(currentColor, *nextColor, dataMode[ActiveParameters.currentStepMode].interval);
            break;
        }
        default:
            break;
    }
}

void Light::setColor(const QColor &color, int duration)
{
    // Встановлення кольору фону для основної форми
    setWindowBackgroundColor(color);
    qDebug() << "Set new color : " << QTime::currentTime().toString();

    if (duration > 0) {
        // Перевірка, чи існує попередній таймер
        if (!saveTimer.pTimer) {
            // Створення нового таймера, якщо він ще не існує
            saveTimer.pTimer = new QTimer(this);
            connect(saveTimer.pTimer, &QTimer::timeout, this, [=]() {
                saveTimer.pTimer->stop(); // Зупиняємо таймер
                qDebug() << "Timer timeout:" << QTime::currentTime().toString();
                selectStepMode(true); // Передача параметру true
            });
            qDebug() << "Timer created:" << QTime::currentTime().toString();
        }

        // Встановлення інтервалу таймера
        saveTimer.pTimer->setInterval(duration); // 1000 мс = 1 с

        // Запуск або перезапуск таймера
        saveTimer.pTimer->start();
        qDebug() << "Timer Start:" << QTime::currentTime().toString();
    }
}


QColor Light::windowBackgroundColor() const {
    return m_windowBackgroundColor;
}

void Light::setWindowBackgroundColor(const QColor &color) {
    if (m_windowBackgroundColor != color) {
        m_windowBackgroundColor = color;
        update();
    }
}

void Light::startColorTransition(const QColor &startColor, const QColor &endColor, int duration)
{
    // Перевірка, чи існує попередня анімація
    if (!pColorAnimation) {
        // Створення нової анімації, якщо її ще не існує
        QPropertyAnimation *colorAnimation = new QPropertyAnimation(this, "windowBackgroundColor");
        pColorAnimation = colorAnimation; // Зберігаємо вказівник на анімацію для можливості зупинки на паузу

        // Підключення сигналу finished до слоту selectStepMode
        connect(colorAnimation, &QPropertyAnimation::finished, this, [=]() {
            colorAnimation->stop();
            pColorAnimation = nullptr;
            qDebug() << "Animation Finish:" << QTime::currentTime().toString();
            selectStepMode(true);
        });

        qDebug() << "Animation Created:" << QTime::currentTime().toString();
    }
    else {
        // Якщо анімація вже існує, перериваємо її
        pColorAnimation->stop();
    }

    // Налаштування нових параметрів анімації
    pColorAnimation->setStartValue(startColor);
    pColorAnimation->setEndValue(endColor);
    pColorAnimation->setDuration(duration);

    // Запуск або відновлення анімації
    pColorAnimation->start();
    qDebug() << "Animation Start:" << QTime::currentTime().toString();
}


void Light::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), m_windowBackgroundColor);
}















