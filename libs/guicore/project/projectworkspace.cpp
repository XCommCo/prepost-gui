#include <QDir>
#include <QSettings>
#include <QCryptographicHash>

#include "projectworkspace.h"
#include "workfolderinfo.h"
#include <misc/filesystemfunction.h>

const QString ProjectWorkspace::FOLDERNAME = ".iRIC_workspace";

ProjectWorkspace::ProjectWorkspace(QObject *parent) :
	QObject(parent)
{
	QDir homeDir = QDir::home();
	QString defaultWorkspace = homeDir.absoluteFilePath(ProjectWorkspace::FOLDERNAME);
	QSettings settings;
	QString workspace = settings.value("general/workspace", defaultWorkspace).value<QString>();
	m_workspace = QDir(workspace);
	// if the workspace doesn't exists, make it.
	if (! m_workspace.exists()){
		QDir parentDir = m_workspace;
		parentDir.cdUp();
		parentDir.mkdir(m_workspace.dirName());
	}
	updateWorkfolderList();
//	trashAllTrashWorkfolders();
}

void ProjectWorkspace::updateWorkfolderList()
{
	QList<WorkfolderInfo*>::iterator it;
	for (it = m_workfolderList.begin(); it != m_workfolderList.end(); ++it){
		delete *it;
	}
	m_workfolderList.clear();
	QStringList subdirs = m_workspace.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

	for (int i = 0; i < subdirs.size(); ++i){
		QString wfFull = m_workspace.absoluteFilePath(subdirs.at(i));
		WorkfolderInfo* info = new WorkfolderInfo(wfFull, this);
		m_workfolderList.push_back(info);
	}
}

QList<WorkfolderInfo*> ProjectWorkspace::trashWorkfolders()
{
	QList<WorkfolderInfo*> ret;
	QList<WorkfolderInfo*>::iterator it;
	for (it = m_workfolderList.begin(); it != m_workfolderList.end(); ++it){
		if (! (*it)->isLocked()){
			ret.push_back(*it);
		}
	}
	return ret;
}
void ProjectWorkspace::trashAllTrashWorkfolders()
{
	QList<WorkfolderInfo*> trashes = trashWorkfolders();
	QList<WorkfolderInfo*>::iterator it;
	for (it = trashes.begin(); it != trashes.end(); ++it){
		trashWorkfolder((*it)->folderName());
	}
}

bool ProjectWorkspace::trashWorkfolder(const QString& foldername)
{
	return iRIC::rmdirRecursively(m_workspace.absoluteFilePath(foldername));
}

const QString ProjectWorkspace::tmpFileName() const
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	QTime current = QTime::currentTime();
	qsrand(current.msec());
	hash.addData(QByteArray(1, qrand()));
	QDir workDir(m_workspace.absolutePath());

	QString filename = hash.result().toHex();
	while (workDir.exists(filename)){
		hash.addData(QByteArray(1, qrand()));
		filename = hash.result().toHex();
	}
	return workDir.absoluteFilePath(filename);
}

void ProjectWorkspace::removeZipTrashes()
{
	QStringList nameFilters;
	nameFilters << "zia*";
	QStringList trashFiles = m_workspace.entryList(nameFilters, QDir::Files);
	for (int i = 0; i < trashFiles.count(); ++i){
		QString trashFile = trashFiles.at(i);
		m_workspace.remove(trashFile);
	}
}