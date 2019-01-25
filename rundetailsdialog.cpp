#include "rundetailsdialog.h"
#include "ui_rundetailsdialog.h"

RunDetailsDialog::RunDetailsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunDetailsDialog)
{
    ui->setupUi(this);
}

RunDetailsDialog::~RunDetailsDialog()
{
    delete ui;
}
