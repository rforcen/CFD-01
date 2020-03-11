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
  if (image_data) delete[] image_data;
  image_data = new uchar[image_size];
  if (solver) delete solver;
  solver = new FluidSolver<double>(sizeX, sizeY, density);

  worker.run([&] {
    solver->addInflow(0.45, 0.2, 0.1, 0.01, 1.0, 0.0, 3.0);
    solver->update_mt(timestep);
    time += timestep;

    emit update_image();
  });
}

MainWindow::~MainWindow() {
  worker.stop();

  delete ui;
  delete solver;
  delete[] image_data;
}

void MainWindow::on_actionrun_triggered() { worker.switch_disp(); }

void MainWindow::on_update_image() {
  ui->label->setPixmap(QPixmap::fromImage(
      QImage(solver->toImage(image_data), sizeX, sizeY, QImage::Format_ARGB32)
          .scaled(ui->label->size())));
  ui->statusbar->showMessage(QString("sim. time:%1, lap: %2 ms")
                                 .arg(time, 5, 'f', 3)
                                 .arg(worker.get_lap(), 4));
}

void MainWindow::on_actionnew_triggered() {
  worker.stop();
  run();
}
