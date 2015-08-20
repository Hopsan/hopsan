#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>
#include <QLabel>

class MainWindow;
class QTimer;

class LicenseDialog : public QDialog
{
    Q_OBJECT
public:
    LicenseDialog(QWidget *pParent=0);

private slots:
    void toggleAlwaysShow(bool tf);
    void showLicenseDocs();
};

#endif // LICENSEDIALOG_H
