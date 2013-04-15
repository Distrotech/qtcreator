#ifndef VCPROJECTBUILDOPTIONSPAGE_H
#define VCPROJECTBUILDOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QDialog>

class QLabel;
class QLineEdit;
class QProcess;
class QPushButton;
class QTableWidget;

namespace VcProjectManager {
namespace Internal {

struct VcProjectValidator {
    enum ValidationRequest {
        ValidationRequest_Add,
        ValidationRequest_Edit
    };

    QString m_originalExecutable;  // used only for ValidationRequest_Edit
    QString m_executable;
    QProcess *m_process;
    ValidationRequest m_requestType;
};

struct MsBuildInformation {
    QString m_executable;
    QString m_version;
};

class VcProjectEditMsBuildDialog : public QDialog
{
    Q_OBJECT

public:
    VcProjectEditMsBuildDialog(QWidget *parent = 0);
    ~VcProjectEditMsBuildDialog();

    QString path() const;
    void setPath(const QString &path);

private slots:
    void showBrowseFileDialog();

private:
    QLineEdit *m_pathChooser;
    QPushButton *m_browseButton;
};

class VcProjectBuildOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    VcProjectBuildOptionsWidget(QWidget *parent = 0);
    ~VcProjectBuildOptionsWidget();

    MsBuildInformation build(int index);
    int buildCount();
    MsBuildInformation currentSelectedBuild() const;
    int currentSelectedRow() const;
    bool exists(const QString &exePath);
    void insertMSBuild(const MsBuildInformation &info);
    void removeBuild(int index);
    void updateMsBuild(const QString &exePath, const MsBuildInformation &newMsBuildInfo);

private slots:
    void onTableRowIndexChange(int index);

signals:
    void addNewButtonClicked();
    void editButtonClicked();
    void deleteButtonClicked();
    void currentBuildSelectionChanged(int);

private:
    QPushButton *m_addMsBuildButton;
    QPushButton *m_editBuildButton;
    QPushButton *m_deleteBuildButton;
    QTableWidget *m_buildTableWidget;
};

class VcProjectBuildOptionsPage : public Core::IOptionsPage
{
    Q_OBJECT

public:
    VcProjectBuildOptionsPage();
    ~VcProjectBuildOptionsPage();

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

    QVector<MsBuildInformation *> msBuilds() const;
    void loadSettings();
    void saveSettings();
    void startVersionCheck();

private slots:
    void addNewMsBuild();
    void editMsBuild();
    void deleteMsBuild();
    void versionCheckFinished();

signals:
    void msBuildInfomationsUpdated();

private:
    QVector<MsBuildInformation *> m_msBuildInformations;
    VcProjectBuildOptionsWidget *m_optionsWidget;
    VcProjectValidator m_validator;
};

} // namespace Internal
} // namespace VcProjectManager

#endif // VCPROJECTBUILDOPTIONSPAGE_H
