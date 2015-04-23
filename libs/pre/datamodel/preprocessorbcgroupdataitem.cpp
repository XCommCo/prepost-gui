#include "preprocessorbcgroupdataitem.h"
#include "preprocessorbcdataitem.h"
#include "preprocessorgridtypedataitem.h"
#include "preprocessorgriddataitem.h"
#include <guicore/project/projectdata.h>
#include <guicore/solverdef/solverdefinitionboundarycondition.h>

#include <guicore/pre/grid/grid.h>
#include <misc/stringtool.h>
#include <guicore/project/colorsource.h>
#include <guibase/objectbrowserview.h>
#include <misc/versionnumber.h>

#include <QMenu>

#include <iriclib.h>

PreProcessorBCGroupDataItem::PreProcessorBCGroupDataItem(PreProcessorDataItem* parent)
	: PreProcessorDataItem(tr("Boundary Condition"), QIcon(":/libs/guibase/images/iconFolder.png"), parent)
{
	m_isDeletable = false;
	m_standardItem->setCheckable(true);
	m_standardItem->setCheckState(Qt::Checked);

	m_standardItemCopy = m_standardItem->clone();
	m_subFolder = "bc";

	m_colorSource = new ColorSource(this);

	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent->parent()->parent());
	SolverDefinitionGridType* gtype = gtItem->gridType();
	for (int i = 0; i < gtype->boundaryConditions().count(); ++i){
		SolverDefinitionBoundaryCondition* bc = gtype->boundaryConditions().at(i);
		QString str(tr("Add %1"));
		QAction* addAction = new QAction(str.arg(bc->caption()), this);
		connect(addAction, SIGNAL(triggered()), this, SLOT(addCondition()));
		m_addActions.append(addAction);
	}

	m_dummyEditAction = new QAction(tr("&Edit Condition..."), this);
	m_dummyEditAction->setDisabled(true);
	m_dummyDeleteAction = new QAction(tr("&Delete..."), this);
	m_dummyDeleteAction->setDisabled(true);

	m_dummyAssignAction = new QAction(tr("&Assign Condition"), this);
	m_dummyAssignAction->setDisabled(true);
	m_dummyReleaseAction = new QAction(tr("&Release Condition"), this);
	m_dummyReleaseAction->setDisabled(true);

	m_bcMenu = new QMenu(tr("&Boundary Condition"), mainWindow());
}

PreProcessorBCGroupDataItem::~PreProcessorBCGroupDataItem()
{

}

void PreProcessorBCGroupDataItem::loadFromCgnsFile(const int fn)
{
	cg_iRIC_Init_BC_Names();
	QList<GraphicsWindowDataItem*>::iterator it;
	Grid* grid = dynamic_cast<PreProcessorGridDataItem*>(parent())->grid();

	if (m_projectBuildNumber > 3507){
		for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
			PreProcessorBCDataItem* bcItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
			bcItem->setGrid(grid);
			bcItem->loadFromCgnsFile(fn);
		}
	} else {
		// for backward compatibility.
		PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
		const QList<SolverDefinitionBoundaryCondition*>& conditions = gtItem->gridType()->boundaryConditions();
		for (int i = 0; i < conditions.count(); ++i){
			SolverDefinitionBoundaryCondition* bc = conditions.at(i);
			int number;
			cg_iRIC_Read_BC_Count(const_cast<char*>(iRIC::toStr(bc->name()).c_str()), &number);
			for (int j = 0; j < number; ++j){
				PreProcessorBCDataItem* bcItem = new PreProcessorBCDataItem(projectData()->solverDefinition(), bc, this);
				bcItem->setProjectNumber(j + 1);
				bcItem->setCgnsNumber(j + 1);
				bcItem->loadFromCgnsFile(fn);
				m_childItems.append(bcItem);
			}
		}
		assignActionZValues(m_zDepthRange);
		updateItemMap();
		emit itemsUpdated();
		emit itemsLoaded();
	}
}

void PreProcessorBCGroupDataItem::renumberItemsForProject()
{
	// set numbers.
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	const QList<SolverDefinitionBoundaryCondition*>& conditions = gtItem->gridType()->boundaryConditions();

	int number = 1;
	int condIndex = -1;
	QList <GraphicsWindowDataItem*>::iterator it = m_childItems.begin();
	while (it != m_childItems.end()){
		PreProcessorBCDataItem* tmpItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
		SolverDefinitionBoundaryCondition* tmpbc = tmpItem->condition();
		int tmpindex = conditions.indexOf(tmpbc);
		if (tmpindex != condIndex){
			number = 1;
			condIndex = tmpindex;
		} else {
			++ number;
		}
		tmpItem->setProjectNumber(number);
		++ it;
	}
}

void PreProcessorBCGroupDataItem::renumberItemsForCgns()
{
	// set numbers again.
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	const QList<SolverDefinitionBoundaryCondition*>& conditions = gtItem->gridType()->boundaryConditions();

	int number = 1;
	int condIndex = -1;
	QList <GraphicsWindowDataItem*>::iterator it = m_childItems.begin();
	while (it != m_childItems.end()){
		PreProcessorBCDataItem* tmpItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
		if (tmpItem->isValid()){
			SolverDefinitionBoundaryCondition* tmpbc = tmpItem->condition();
			int tmpindex = conditions.indexOf(tmpbc);
			if (tmpindex != condIndex){
				number = 1;
				condIndex = tmpindex;
			} else {
				++ number;
			}
			tmpItem->setCgnsNumber(number);
		} else {
			tmpItem->setCgnsNumber(0);
		}
		++ it;
	}
}

void PreProcessorBCGroupDataItem::saveToCgnsFile(const int fn)
{
	renumberItemsForCgns();
	cg_iRIC_Clear_BC();
	for (int i = 0; i < m_childItems.count(); ++i){
		m_childItems[i]->saveToCgnsFile(fn);
	}
}

void PreProcessorBCGroupDataItem::informGridUpdate()
{

}

void PreProcessorBCGroupDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	m_projectBuildNumber = projectData()->version().build();
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	QDomNodeList childNodes = node.childNodes();
	for (int i = 0; i < childNodes.count(); ++i){
		QDomElement childElem = childNodes.at(i).toElement();
		QString condType = childElem.attribute("type");
		SolverDefinitionBoundaryCondition* bc = gtItem->gridType()->boundaryCondition(condType);
		PreProcessorBCDataItem* bcItem = new PreProcessorBCDataItem(projectData()->solverDefinition(), bc, this);
		bcItem->loadFromProjectMainFile(childElem);
		m_childItems.append(bcItem);
	}
	assignActionZValues(m_zDepthRange);
	updateItemMap();

	emit itemsUpdated();
	emit itemsLoaded();
}

void PreProcessorBCGroupDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	renumberItemsForProject();
	QList<GraphicsWindowDataItem*>::iterator it;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		GraphicsWindowDataItem* item = *it;
		writer.writeStartElement("BoundaryCondition");
		item->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
}

void PreProcessorBCGroupDataItem::addCustomMenuItems(QMenu* menu)
{
	for (int i = 0; i < m_addActions.count(); ++i){
		menu->addAction(m_addActions[i]);
	}
}

void PreProcessorBCGroupDataItem::addCondition()
{
	QAction* a = dynamic_cast<QAction*>(sender());
	int index = 0;
	for (index = 0; index < m_addActions.count(); ++index){
		if (m_addActions[index] == a){break;}
	}

	PreProcessorBCDataItem* item = addCondition(index, true);
	if (item == 0){return;}

	dataModel()->objectBrowserView()->expand(m_standardItem->index());
	dataModel()->objectBrowserView()->select(item->standardItem()->index());
	dataModel()->handleObjectBrowserSelection(item->standardItem()->index());
	bool ok = item->showDialog();
	if (! ok){
		// Cancel button is clicked.
		delete item;
		dataModel()->handleObjectBrowserClick(m_standardItem->index());
		updateItemMap();
	}
}

PreProcessorBCDataItem* PreProcessorBCGroupDataItem::addCondition(int index, bool hideSetting)
{
	if (index > m_addActions.count()){return 0;}

	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	SolverDefinitionGridType* gtype = gtItem->gridType();
	SolverDefinitionBoundaryCondition* bc = gtype->boundaryConditions().at(index);
	PreProcessorBCDataItem* item = new PreProcessorBCDataItem(projectData()->solverDefinition(), bc, this, hideSetting);
	QString tmpName(QString("New %1").arg(bc->englishCaption()));
	item->setName(tmpName);
	int number = 1;
	QList <GraphicsWindowDataItem*>::iterator it = m_childItems.begin();
	while (it != m_childItems.end()){
		PreProcessorBCDataItem* tmpItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
		SolverDefinitionBoundaryCondition* tmpbc = tmpItem->condition();
		int tmpindex = gtype->boundaryConditions().indexOf(tmpbc);
		if (tmpindex == index){++ number;}
		if (tmpindex > index){break;}
		++ it;
	}
	item->setProjectNumber(number);
	item->setCgnsNumber(number);
	item->setColor(m_colorSource->getColor(m_childItems.count()));
	// add it simply.
	m_childItems.insert(it, item);
	PreProcessorGridDataItem* gItem = dynamic_cast<PreProcessorGridDataItem*>(parent());
	if (gItem->grid() != 0){
		gItem->grid()->setModified();
	}

	// setup object browser tree again.
	int rows = m_standardItem->rowCount();
	for (int row = rows - 1; row >= 0; --row){
		m_standardItem->takeRow(row);
	}
	for (int i = 0; i < m_childItems.count(); ++i){
		GraphicsWindowDataItem* item = m_childItems.at(i);
		m_standardItem->appendRow(item->standardItem());
	}
	updateItemMap();
	assignActionZValues(m_zDepthRange);
	emit itemsUpdated();

	return item;
/*
	dataModel()->objectBrowserView()->expand(m_standardItem->index());
	dataModel()->objectBrowserView()->select(item->standardItem()->index());
	dataModel()->handleObjectBrowserSelection(item->standardItem()->index());
	bool ok = item->showDialog();
	if (! ok){
		// Cancel button is clicked.
		delete item;
		dataModel()->handleObjectBrowserClick(m_standardItem->index());
		updateItemMap();
	}
*/
}


void PreProcessorBCGroupDataItem::updateZDepthRangeItemCount()
{
	m_zDepthRange.setItemCount(5);
}

void PreProcessorBCGroupDataItem::updateBCMenu(PreProcessorBCDataItem* item)
{
	m_bcMenu->clear();
	for (int i = 0; i < m_addActions.count(); ++i){
		m_bcMenu->addAction(m_addActions[i]);
	}
	m_bcMenu->addSeparator();
	if (item == 0){
		// add dummy menu items.
		m_bcMenu->addAction(m_dummyEditAction);
		m_bcMenu->addAction(m_dummyDeleteAction);
		m_bcMenu->addSeparator();
		m_bcMenu->addAction(m_dummyAssignAction);
		m_bcMenu->addAction(m_dummyReleaseAction);
	} else {
		m_bcMenu->addAction(item->editAction());
		m_bcMenu->addAction(item->deleteAction());
		m_bcMenu->addSeparator();
		m_bcMenu->addAction(item->assignAction());
		m_bcMenu->addAction(item->releaseAction());
	}
}

void PreProcessorBCGroupDataItem::clear()
{
	QList<GraphicsWindowDataItem*> itemCopys = m_childItems;
	for (int i = 0; i < itemCopys.count(); ++i){
		GraphicsWindowDataItem* item = itemCopys[i];
		delete item;
	}
	m_childItems.clear();
	updateItemMap();
}

void PreProcessorBCGroupDataItem::clearPoints()
{
	QList<GraphicsWindowDataItem*> itemCopys = m_childItems;
	for (int i = 0; i < itemCopys.count(); ++i){
		GraphicsWindowDataItem* item = itemCopys[i];
		PreProcessorBCDataItem* bcItem = dynamic_cast<PreProcessorBCDataItem*> (item);
		bcItem->clearPoints();
	}
}

void PreProcessorBCGroupDataItem::setGrid(Grid* grid)
{
	QList<GraphicsWindowDataItem*> itemCopys = m_childItems;
	for (int i = 0; i < itemCopys.count(); ++i){
		GraphicsWindowDataItem* item = itemCopys[i];
		PreProcessorBCDataItem* bcItem = dynamic_cast<PreProcessorBCDataItem*> (item);
		bcItem->setGrid(grid);
	}
}