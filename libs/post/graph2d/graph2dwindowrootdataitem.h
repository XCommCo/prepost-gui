#ifndef GRAPH2DWINDOWROOTDATAITEM_H
#define GRAPH2DWINDOWROOTDATAITEM_H

#include "graph2d_global.h"
#include "graph2dwindowdataitem.h"
#include <QList>
#include <QMap>

class Graph2dWindowDataModel;
class QStandardItemModel;
class QMainWindow;

class GRAPH2D_EXPORT Graph2dWindowRootDataItem : public Graph2dWindowDataItem
{
public:
	/// Constructor
	Graph2dWindowRootDataItem(QMainWindow* window, ProjectDataItem* parent);
	~Graph2dWindowRootDataItem();

	void handleItemChange(QStandardItem* changedItem);
	void handleItemClick(QStandardItem* clickedItem);
	void handleItemDoubleClick(QStandardItem* clickedItem);
	void deleteItem(QStandardItem* item);
	void moveUpItem(QStandardItem* item);
	void moveDownItem(QStandardItem* item);
	void updateItemMap();
	Graph2dWindowDataItem* modelItemFromItem(QStandardItem* item);
	void loadFromCgnsFile(const int fn);
	void closeCgnsFile();
protected:
	bool isAncientChecked(){return true;}
	void doLoadFromProjectMainFile(const QDomNode& /*node*/){}
	void doSaveToProjectMainFile(QXmlStreamWriter& /*writer*/){}
	Graph2dWindowDataModel* dataModel(){return m_dataModel;}
protected:
	QMainWindow* m_mainWindow;
	Graph2dWindowDataModel* m_dataModel;
private:
	QMap<QStandardItem*, Graph2dWindowDataItem*> m_itemMap;
public:
	friend class Graph2dWindowDataModel;
};

#endif // GRAPH2DWINDOWROOTDATAITEM_H