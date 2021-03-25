#include <QtWidgets>

/*
#include <QDialog>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenuBar>
#include <QMainWindow>
#include <QPalette>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeView>
#include <QFileInfo>
#include <QMap>
#include <QProgressBar>
#include <QVector>
*/

#include "fenprincipale.h"
#include "core.h"
#include "aboutdialog.h"


FenPrincipale::FenPrincipale() {
    widget = new QWidget();
    c = new Core();

    setWindowTitle(qApp->applicationName());

    /*** Menus et actions ***/
    createActions();
    createMenus();

    folderSelectionGr = new QGroupBox(tr("File selection"));

    folder = new QLineEdit;
    folder->setFocus();

    nbFolder2Create = new QSpinBox();
    nbFolder2Create->setRange(1, 500);

    selectFolder = new QPushButton();
    selectFolder->setText("...");

    addSelectedFolder = new QPushButton(tr("Add folder"));

    fSelect = new QGridLayout;
    fSelect->addWidget(selectFolder, 0, 0);
    fSelect->addWidget(folder, 0, 1);
    fSelect->addWidget(nbFolder2Create, 0, 2);
    fSelect->addWidget(addSelectedFolder, 0, 3);
    folderSelectionGr->setLayout(fSelect);

    listeGr = new QGroupBox(tr("Summary"));

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(1);

    QStringList headers;
    headers << tr("Folder");

    tableWidget->setHorizontalHeaderLabels(headers);
    tableWidget->setSortingEnabled(false);
    tableWidget->setColumnCount(1);

    recap = new QGridLayout;
    recap->addWidget(tableWidget, 0, 0);
    listeGr->setLayout(recap);

    //pBar = new QProgressBar(this);
    //pBar->setRange(0, 100);
    //pBar->setValue(0);

    startBtn = new QPushButton(tr("Run", "Run the process"));

    layout = new QGridLayout;
    layout->addWidget(folderSelectionGr, 0, 0);
    layout->addWidget(listeGr, 1, 0);
    //layout->addWidget(pBar, 2, 0);
    layout->addWidget(startBtn, 2, 0);
    widget->setLayout(layout);

    /* Liste des connect */

    connect(folder, SIGNAL(textEdited(const QString&)), this, SLOT(mVerifyDir(const QString&)));
    connect(selectFolder, SIGNAL(clicked()), this, SLOT(mSelectFolder()));
    connect(addSelectedFolder, SIGNAL(clicked()), this, SLOT(mAddFolder()));
    connect(startBtn, SIGNAL(clicked()), this, SLOT(mRun()));

    setWindowIcon(QIcon(":/images/folder-open"));
    setWindowTitle(qApp->applicationName());
    setCentralWidget(widget);

    QFile file("./version.json");

    if(!file.exists()) { //New Update detected !
        c->setFirstRun();
    }

    if(file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
		stream << "{'version': '" << qApp->applicationVersion() << "'}" << Qt::endl;
        file.close();
    }

    if(c->isFirstRun()) {
        QMessageBox::information(this, tr("Welcome !"), tr("The prefix has been automatically set to a default value. Good use of software"));
    }
}

void FenPrincipale::createActions() {
    actQuitter = new QAction(tr("Exit"), this);
    actQuitter->setShortcut(QKeySequence("Ctrl+q"));
    connect(actQuitter, SIGNAL(triggered()), this, SLOT(close()));

    actSetPrefix = new QAction(tr("Set the prefixe"));
    connect(actSetPrefix, SIGNAL(triggered()), this, SLOT(mSetPrefix()));

    //actEditStatsSetting = new QAction(tr("Données de statistiques"));
    //connect(actEditStatsSetting, SIGNAL(triggered()), this, SLOT(mAnaliticData()));

	m_actionChangeLang = new QAction(tr("Change the langage"));
	//connect(m_actionChangeLang, SIGNAL(triggered()), this, SLOT(mChangeLangageDialog)); //TODO: implement langage menu selection (like at https://wiki.qt.io/How_to_create_a_multi_language_application)

    actAbout = new QAction(tr("About"), this);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(mAboutPopup()));

    actVerifyUpdate = new QAction(tr("Vérifier les mises à jour"), this);
    actVerifyUpdate->setShortcut(QKeySequence("Ctrl+u"));
    connect(actVerifyUpdate, SIGNAL(triggered()), this, SLOT(mCheckForUpdates()));

    actAboutQt = new QAction(tr("About Qt"), this);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void FenPrincipale::createMenus() {
    menuFile = menuBar()->addMenu(tr("File"));
    menuFile->addAction(actQuitter);

    menuSettings = menuBar()->addMenu(tr("Settings"));
    menuSettings->addAction(actSetPrefix);
    //menuSettings->addAction(actEditStatsSetting);
	//menuSettings->addAction(m_actionChangeLang); //TODO: Langage menu selection

    menuHelp = menuBar()->addMenu(tr("Help"));
    menuHelp->addAction(actAbout);
    menuHelp->addAction(actVerifyUpdate);
    menuHelp->addAction(actAboutQt);
    menuHelp->addAction(actVerifyUpdate);
}

void FenPrincipale::mSelectFolder() {
    QString absolutePath = QFileDialog::getExistingDirectory(this, tr("Select a folder"), QDir::toNativeSeparators(QDir::homePath()));
    folder->setText(absolutePath);
}

void FenPrincipale::mAddFolder() {
    QString temp = this->folder->text();
    int nb = this->nbFolder2Create->value();
    QString path = temp + " (" + QString::number(nb) + ")";

    totalItems += nb;

    if(!temp.isEmpty()) {
        if(QDir(temp).exists()) {
            QMap<int, QString> a;
            a.insert(nb, temp);

            liste.insert(liste.size(), a);
            a.clear();

            auto item = new QTableWidgetItem(path);
            listeItems.append(item);
            auto sz = tableWidget->rowCount();

            tableWidget->insertRow(sz);
            tableWidget->setItem(sz, 0, item);
            tableWidget->update();

            qDebug() << path;
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Unable to add the folder because it is invalid."));
        }
    }

    //unset text and number in QLineEdit
    this->folder->setText("");
    this->nbFolder2Create->setValue(0);
}

void FenPrincipale::mAboutPopup() {
    aboutDialog = new AboutDialog;
}

void FenPrincipale::mCheckForUpdates() {
    //Check for updates here
	verifyUpdateThread = new VerifyUpdateThread(this);
	verifyUpdateThread->start();
}

void FenPrincipale::mShowSuccess() {

	startBtn->setDisabled(false);

	QMessageBox::information(this, tr("Success"), tr("All files have been processed!"));
}

void FenPrincipale::mUpdateFinished() {
	QMessageBox::information(this, "Succès", "Vérification des mises à jour effectuée");
}

void FenPrincipale::mSetPrefix() {
    bool ok;
    QString p = QInputDialog::getText(this, tr("Prefix enter"), tr("Without spaces at the end."), QLineEdit::Normal, "", &ok);

    if(ok && !p.isEmpty())
        c->setPrefix(p);
}

void FenPrincipale::mRun() {
    qDebug() << "Starting ...";
    QDir d;

    startBtn->setDisabled(true);

    //pBar->setMaximum(this->totalItems);

	if(this->liste.empty()) {
		QMessageBox::critical(this, tr("Erreur"), tr("La liste est vide !"));
	} else {
		c->process(this->liste, d);

		auto prefix = c->getPrefix();

		folderCreationThread = new FolderCreateThread(this);
		folderCreationThread->setParams(this->liste, d, prefix);

		connect(folderCreationThread, SIGNAL(finished()), this, SLOT(mShowSuccess()));

		folderCreationThread->start();

		this->listeItems.clear();

		tableWidget->clearContents(); //TODO: verify that function works as expected
		liste.clear();
	}
}

void FenPrincipale::mVerifyDir(const QString &text) {
    QPalette pal = this->folder->palette();
    if(!QDir(text).exists()) {
        pal.setColor(QPalette::Text, Qt::red);
    } else {
        pal.setColor(QPalette::Text, Qt::black);
    }
    this->folder->setPalette(pal);
}