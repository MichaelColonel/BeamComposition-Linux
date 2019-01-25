#ifndef RUNDETAILSDIALOG_H
#define RUNDETAILSDIALOG_H

#include <QDialog>

namespace Ui {
class RunDetailsDialog;
}

class RunDetailsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RunDetailsDialog(QWidget *parent = 0);
    ~RunDetailsDialog();
    
private:
    Ui::RunDetailsDialog *ui;
};

#endif // RUNDETAILSDIALOG_H
