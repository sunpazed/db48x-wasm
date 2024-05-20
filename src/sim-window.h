#ifndef SIM_WINDOW_H
#define SIM_WINDOW_H
// ****************************************************************************
//  sim-window.h                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Main window of the simulator
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

// #include "sim-rpl.h"
// #include "ui_sim-window.h"

// #include <QAbstractEventDispatcher>
// #include <QAudio>
// #include <QAudioSink>
// #include <QBuffer>
// #include <QByteArray>
// #include <QComboBox>
// #include <QFile>
// #include <QIODevice>
// #include <QLabel>
// #include <QMainWindow>
// #include <QMediaDevices>
// #include <QObject>
// #include <QPushButton>
// #include <QScopedPointer>
// #include <QSlider>
// #include <QTimer>


// class TestsThread : public QThread
// // ----------------------------------------------------------------------------
// //   A thread to run the automated tests
// // ----------------------------------------------------------------------------
// {
// public:
//     TestsThread(QObject *parent): QThread(parent), onlyCurrent() {}
//     ~TestsThread()
//     {
//         if (isRunning())
//             while (isFinished())
//                 terminate();
//     }
//     void run()
//     {
//         tests TestSuite;
//         TestSuite.run(onlyCurrent);
//     }
//     bool onlyCurrent;
// };


// class Highlight : public QWidget
// // ----------------------------------------------------------------------------
// //   Highlight of a key
// // ----------------------------------------------------------------------------
// {
//     Q_OBJECT;
// public:
//     Highlight(QWidget *parent): QWidget(parent) {}
//     void paintEvent(QPaintEvent *);

// public slots:
//     void keyResizeSlot(const QRect &rect);
// };



// class AudioGenerator : public QIODevice
// // ----------------------------------------------------------------------------
// //   Generating the samples on demand
// // ----------------------------------------------------------------------------
// {
//     Q_OBJECT

// public:
//     AudioGenerator(const QAudioFormat &format, qint64 durationUs, uint freq);

//     void start();
//     void stop();

//     qint64 readData(char *data, qint64 maxlen) override;
//     qint64 writeData(const char *data, qint64 len) override;
//     qint64 bytesAvailable() const override;
//     qint64 size() const override { return buffer.size(); }
//     uint frequency() { return freq; }


// private:
//     void generateData(const QAudioFormat &format, qint64 durUs, uint freq);

// private:
//     qint64     pos = 0;
//     QByteArray buffer;
//     int        freq;
// };


// class MainWindow : public QMainWindow
// // ----------------------------------------------------------------------------
// //   Main window for the simulator
// // ----------------------------------------------------------------------------
// {
//     Q_OBJECT;

//     static MainWindow             *mainWindow;
//     Ui::MainWindow                 ui;
//     RPLThread                      rpl;
//     TestsThread                    tests;
//     Highlight                     *highlight;

//     // Audio support
//     // QMediaDevices                 *devices = nullptr;
//     // QScopedPointer<AudioGenerator> generator;
//     // QScopedPointer<QAudioSink>     audio;
//     volatile bool                  playing;

//     enum { SAMPLE_RATE = 20000, SAMPLE_COUNT = SAMPLE_RATE };
// public:
//     static qreal       devicePixelRatio;

// public:
//     explicit MainWindow(QWidget *parent = 0);
//     ~MainWindow();

//     void pushKey(int key);
//     QPixmap &screen() { return ui.screen->mainPixmap; }
//     static MainWindow * theMainWindow() { return mainWindow; }
//     static QPixmap &    theScreen()     { return mainWindow->screen(); }
//     static void         screenshot(cstring basename = "screens/",
//                                    int     x = 0,
//                                    int     y = 0,
//                                    int     w = LCD_W,
//                                    int     h = LCD_H);

//     void                startBuzzer();
//     void                stopBuzzer();
//     bool                buzzerPlaying() { return playing; }

//   protected:
//     virtual void keyPressEvent(QKeyEvent *ev);
//     virtual void keyReleaseEvent(QKeyEvent *ev);
//     bool         eventFilter(QObject *obj, QEvent *ev);
//     void         resizeEvent(QResizeEvent *event);

// signals:
//     void        keyResizeSignal(const QRect &rect);

// private:
//     // Audio support
//     // void        initializeAudio(const QAudioDevice &deviceInfo, uint freq);

// private slots:
//     void        updateAudioDevices();

// };


// template <typename F>
// static void postToThread(F && fun, QThread *thread = qApp->thread())
// // ----------------------------------------------------------------------------
// //   Post something on another thread, typically the main thread
// // ----------------------------------------------------------------------------
// {
//    auto *obj = QAbstractEventDispatcher::instance(thread);
//    Q_ASSERT(obj);
//    QMetaObject::invokeMethod(obj, std::forward<F>(fun));
// }



// ============================================================================
//
//   Audio support (lifted from QT audiooutput example)
//
// ============================================================================

#endif // SIM_WINDOW_H
