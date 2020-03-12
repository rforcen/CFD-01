#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(this, &MainWindow::update_image, this, &MainWindow::on_update_image);
}

void MainWindow::showEvent(QShowEvent *) { run(); }

void MainWindow::run() {
  sizeY = ui->label->size().width();
  sizeX = ui->label->size().height();

  if (solver) delete solver;
  solver = new FluidSolver<double>(sizeX, sizeY, density);

  worker.run([&] {
    solver->addInflow(0.45, 0.2, 0.1, 0.01, 1, 0, 3.0);
    solver->addInflow(0.7, 0, 0.04, 0.04, 1, 1, 1);
    solver->update_mt(timestep);
    time += timestep;

    emit update_image();
  });
}

MainWindow::~MainWindow() {
  worker.stop();
  delete solver;

  delete ui;
}

void MainWindow::on_actionrun_triggered() { worker.switch_disp(); }

void MainWindow::on_update_image() {
  ui->label->setPixmap(QPixmap::fromImage(
      QImage(solver->toImage(), sizeX, sizeY, QImage::Format_ARGB32)
          .scaled(ui->label->size())));
  ui->statusbar->showMessage(QString("sim. time:%1, fps: %2")
                                 .arg(time, 5, 'f', 3)
                                 .arg(1000. / worker.get_lap(), 0, 'f', 0));
}

void MainWindow::on_actionnew_triggered() {
  worker.stop();
  run();
}
