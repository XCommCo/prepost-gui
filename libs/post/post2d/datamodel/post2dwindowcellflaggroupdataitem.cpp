#include "post2dwindowcellflagdataitem.h"
#include "post2dwindowcellflaggroupdataitem.h"
#include "post2dwindowcellflagsettingdialog.h"
#include "post2dwindowgridtypedataitem.h"
#include "post2dwindowzonedataitem.h"

#include <guicore/project/colorsource.h>
#include <guicore/solverdef/solverdefinitiongridrelatedcondition.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <guicore/solverdef/solverdefinitionintegeroptioncellgridrelatedcondition.h>
#include <misc/iricundostack.h>

#include <QMouseEvent>
#include <QUndoCommand>

Post2dWindowCellFlagGroupDataItem::Post2dWindowCellFlagGroupDataItem(Post2dWindowDataItem* p)
	: Post2dWindowDataItem(tr("Cell attributes"), QIcon(":/libs/guibase/images/iconFolder.png"), p)
{
	m_isDeletable = false;
	m_standardItem->setCheckable(true);
	m_standardItem->setCheckState(Qt::Checked);

	m_standardItemCopy = m_standardItem->clone();

	initSetting();
}

QDialog* Post2dWindowCellFlagGroupDataItem::propertyDialog(QWidget* p)
{
	Post2dWindowCellFlagSettingDialog* d = new Post2dWindowCellFlagSettingDialog(p);
	SolverDefinitionGridType* gt = dynamic_cast<Post2dWindowGridTypeDataItem*>(parent()->parent())->gridType();
	d->setGridType(gt);
	d->setSettings(settings());
	d->setOpacityPercent(m_opacityPercent);
	return d;
}

class Post2dWindowCellFlagGroupSettingCommand : public QUndoCommand
{
public:
	Post2dWindowCellFlagGroupSettingCommand(const QList<Post2dWindowCellFlagSetting>& newsettings, int newo, Post2dWindowCellFlagGroupDataItem* item)
		:QUndoCommand(Post2dWindowCellFlagGroupDataItem::tr("Cell Flag Setting"))
	{
		m_newSettings = newsettings;
		m_newOpacityPercent = newo;

		m_oldSettings = item->settings();
		m_oldOpacityPercent = item->m_opacityPercent;
		m_item = item;
	}
	void redo()
	{
		m_item->setSettings(m_newSettings, m_newOpacityPercent);
		m_item->renderGraphicsView();
	}
	void undo()
	{
		m_item->setSettings(m_oldSettings, m_oldOpacityPercent);
		m_item->renderGraphicsView();
	}
private:
	QList<Post2dWindowCellFlagSetting> m_newSettings;
	int m_newOpacityPercent;

	QList<Post2dWindowCellFlagSetting> m_oldSettings;
	int m_oldOpacityPercent;
	Post2dWindowCellFlagGroupDataItem* m_item;
};

void Post2dWindowCellFlagGroupDataItem::handlePropertyDialogAccepted(QDialog* d)
{
	Post2dWindowCellFlagSettingDialog* dialog = dynamic_cast<Post2dWindowCellFlagSettingDialog*>(d);
	iRICUndoStack::instance().push(new Post2dWindowCellFlagGroupSettingCommand(dialog->settings(), dialog->opacityPercent(), this));
}

QList<Post2dWindowCellFlagSetting> Post2dWindowCellFlagGroupDataItem::settings()
{
	QList<Post2dWindowCellFlagSetting> ret;
	QList <GraphicsWindowDataItem*>::iterator it;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		Post2dWindowCellFlagDataItem* item = dynamic_cast<Post2dWindowCellFlagDataItem*>(*it);
		Post2dWindowCellFlagSetting setting;
		setting.attributeName = item->attributeName();
		setting.value = item->value();
		setting.color = item->color();
		setting.enabled = (item->standardItem()->checkState() == Qt::Checked);
		ret.append(setting);
	}
	return ret;
}

void Post2dWindowCellFlagGroupDataItem::setSettings(const QList<Post2dWindowCellFlagSetting>& settings, int opacity)
{
	QList <GraphicsWindowDataItem*>::iterator it;
	QMap<Post2dWindowCellFlagSetting, Post2dWindowCellFlagDataItem*> map;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		Post2dWindowCellFlagDataItem* item = dynamic_cast<Post2dWindowCellFlagDataItem*>(*it);
		Post2dWindowCellFlagSetting setting;
		setting.attributeName = item->attributeName();
		setting.value = item->value();
		setting.color = item->color();
		setting.enabled  = (item->standardItem()->checkState() == Qt::Checked);
		map.insert(setting, item);
	}
	for (int i = 0; i < m_childItems.count(); ++i){
		m_standardItem->takeRow(0);
	}
	m_childItems.clear();
	QList<Post2dWindowCellFlagSetting>::const_iterator cit;
	for (cit = settings.begin(); cit != settings.end(); ++cit){
		Post2dWindowCellFlagSetting s = *cit;
		Post2dWindowCellFlagDataItem* item = map.value(s);
		item->setColor(s.color);
		item->setOpacity(opacity);
		if (s.enabled){
			item->standardItem()->setCheckState(Qt::Checked);
		}else{
			item->standardItem()->setCheckState(Qt::Unchecked);
		}
		item->updateVisibilityWithoutRendering();
		m_standardItem->appendRow(item->standardItem());
		m_childItems.append(item);
	}
	m_opacityPercent = opacity;
	updateZDepthRange();
}

void Post2dWindowCellFlagGroupDataItem::initSetting()
{
	ColorSource* cs = new ColorSource(0);
	cs->load(":/libs/guicore/data/colorsource_cell.xml");

	m_opacityPercent = 50;

	SolverDefinitionGridType* gt = dynamic_cast<Post2dWindowGridTypeDataItem*>(parent()->parent())->gridType();
	const QList<SolverDefinitionGridRelatedCondition*>& conds = gt->gridRelatedConditions();
	QList<SolverDefinitionGridRelatedCondition*>::const_iterator it;
	int index = 0;
	for (it = conds.begin(); it != conds.end(); ++it){
		const SolverDefinitionGridRelatedCondition* cond = *it;
		if (cond->position() != SolverDefinitionGridRelatedCondition::CellCenter){continue;}
		if (! cond->isOption()){continue;}
		const SolverDefinitionGridRelatedIntegerCondition* icond = dynamic_cast<const SolverDefinitionGridRelatedIntegerCondition*>(cond);
		if (icond == 0){continue;}

		const IntegerEnumLoader* el = dynamic_cast<const IntegerEnumLoader*>(cond);
		QMap<int, QString> enums = el->enumerations();
		QMap<int, QString>::iterator it;
		for (it = enums.begin(); it != enums.end(); ++it){
			QColor color = cs->getColor(index ++);
			QString cap = QString("%1 (%2)").arg(icond->caption(), it.value());
			Post2dWindowCellFlagDataItem* item = new Post2dWindowCellFlagDataItem(icond->name(), it.key(), color, cap, this);
			item->setOpacity(m_opacityPercent);
			m_childItems.append(item);
		}
	}

	updateItemMap();
	updateZDepthRangeItemCount();
}

void Post2dWindowCellFlagGroupDataItem::update()
{
	QList <GraphicsWindowDataItem*>::iterator it;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		Post2dWindowCellFlagDataItem* item = dynamic_cast<Post2dWindowCellFlagDataItem*>(*it);
		item->update();
	}
}

void Post2dWindowCellFlagGroupDataItem::doLoadFromProjectMainFile(const QDomNode & node)
{
	m_opacityPercent = loadOpacityPercent(node);
	QList <GraphicsWindowDataItem*>::iterator it;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		Post2dWindowCellFlagDataItem* item = dynamic_cast<Post2dWindowCellFlagDataItem*>(*it);
		item->setOpacity(m_opacityPercent);
	}
	QDomNodeList childNodes = node.childNodes();
	for (int i = 0; i < childNodes.count(); ++i){
		QDomNode cnode = childNodes.at(i);
		QDomElement celem = cnode.toElement();
		QString attName = celem.attribute("attributeName");
		int value = celem.attribute("value").toInt();

		QList <GraphicsWindowDataItem*>::iterator it;
		for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
			Post2dWindowCellFlagDataItem* item = dynamic_cast<Post2dWindowCellFlagDataItem*>(*it);
			if (item->attributeName() == attName && item->value() == value){
				item->loadFromProjectMainFile(cnode);
			}
		}
	}
}

void Post2dWindowCellFlagGroupDataItem::doSaveToProjectMainFile(QXmlStreamWriter & writer)
{
	writeOpacityPercent(m_opacityPercent, writer);
	QList <GraphicsWindowDataItem*>::iterator it;
	for (it = m_childItems.begin(); it != m_childItems.end(); ++it){
		Post2dWindowCellFlagDataItem* item = dynamic_cast<Post2dWindowCellFlagDataItem*>(*it);
		writer.writeStartElement("CellFlag");
		item->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
}

bool Post2dWindowCellFlagGroupDataItem::hasTransparentPart()
{
	if (standardItem()->checkState() == Qt::Unchecked){return false;}
	if (m_opacityPercent == 100){return false;}
	for (int i = 0; i < m_childItems.count(); ++i){
		GraphicsWindowDataItem* item = m_childItems[i];
		if (item->standardItem()->checkState() == Qt::Checked){return true;}
	}
	return false;
}

void Post2dWindowCellFlagGroupDataItem::informSelection(VTKGraphicsView * /*v*/)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent())->initCellAttributeBrowser();
}

void Post2dWindowCellFlagGroupDataItem::informDeselection(VTKGraphicsView* /*v*/)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent())->clearCellAttributeBrowser();
}

void Post2dWindowCellFlagGroupDataItem::mouseMoveEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent())->updateCellAttributeBrowser(QPoint(event->x(), event->y()), v);
}

void Post2dWindowCellFlagGroupDataItem::mouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent())->fixCellAttributeBrowser(QPoint(event->x(), event->y()), v);
}