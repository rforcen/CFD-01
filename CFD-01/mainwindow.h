#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsPixmapItem>
#include <QMainWindow>

#include <Fluid.h>
#include <Thread.h>
#include <Timer.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 signals:
  void update_image();

 private slots:
  void on_actionrun_triggered();
  void on_update_image();

  void on_actionnew_triggered();

 private:
  Ui::MainWindow *ui;

  int sizeX = 128 * 4;
  int sizeY = sizeX;
  size_t image_size = size_t(sizeX * sizeY * 4);

  double density = 0.1;
  double timestep = 0.005;

  double time = 0;
  FluidSolver<double> *solver = nullptr;

  void iterate(), show_image(), run();
  void showEvent(QShowEvent *);

  Worker worker;
};
#endif  // MAINWINDOW_H
