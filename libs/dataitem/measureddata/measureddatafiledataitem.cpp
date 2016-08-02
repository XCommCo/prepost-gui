#include "measureddatafiledataitem.h"
#include "measureddatapointgroupdataitem.h"
#include "measureddatavectorgroupdataitem.h"

#include <guicore/base/iricmainwindowinterface.h>
#include <guicore/datamodel/graphicswindowdatamodel.h>
#include <guicore/project/measured/measureddata.h>
#include <guicore/project/measured/measureddatacsvexporter.h>
#include <misc/errormessage.h>
#include <misc/lastiodirectory.h>
#include <misc/xmlsupport.h>

#include <QAction>
#include <QDir>
#include <QDomElement>
#include <QDomNode>
#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>
#include <QStatusBar>
#include <QXmlStreamWriter>

MeasuredDataFileDataItem::MeasuredDataFileDataItem(MeasuredData* md, GraphicsWindowDataItem* parent) :
	GraphicsWindowDataItem {tr("File"), QIcon(":/libs/guibase/images/iconFolder.png"), parent},
	m_measuredData {md},
	m_pointGroupDataItem {new MeasuredDataPointGroupDataItem {this}},
	m_vectorGroupDataItem {new MeasuredDataVectorGroupDataItem {this}}
{
	setSubPath("file");

	m_isDeletable = true;
	m_standardItem->setText(md->name());
	m_standardItem->setCheckable(true);
	m_standardItem->setCheckState(Qt::Checked);

	m_standardItemCopy = m_standardItem->clone();

	m_childItems.append(m_pointGroupDataItem);
	m_childItems.append(m_vectorGroupDataItem);

	m_exportAction = new QAction(QIcon(":/libs/guibase/images/iconExport.png"), MeasuredDataFileDataItem::tr("&Export..."), this);
	connect(m_exportAction, SIGNAL(triggered()), this, SLOT(exportToFile()));
}

MeasuredDataFileDataItem::~MeasuredDataFileDataItem()
{}

void MeasuredDataFileDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomNode pdNode = iRIC::getChildNode(node, "PointData");
	if (! pdNode.isNull()) {m_pointGroupDataItem->loadFromProjectMainFile(pdNode);}
	QDomNode vdNode = iRIC::getChildNode(node, "VectorData");
	if (! vdNode.isNull()) {m_vectorGroupDataItem->loadFromProjectMainFile(vdNode);}
}

void MeasuredDataFileDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	writer.writeAttribute("index", QString::number(m_measuredData->index()));

	writer.writeStartElement("PointData");
	m_pointGroupDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();

	writer.writeStartElement("VectorData");
	m_vectorGroupDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();
}

void MeasuredDataFileDataItem::exportToFile()
{
	QString dir = LastIODirectory::get();
	QString filter(tr("CSV file (*.csv)"));
	QString fname = QFileDialog::getSaveFileName(iricMainWindow(), tr("Export Measured Data"), dir, filter);
	if (fname == "") { return; }

	try {
		MeasuredDataCsvExporter exporter;
		exporter.exportData(fname, *m_measuredData);

		iricMainWindow()->statusBar()->showMessage(tr("Measured Data successfully exported to %1.").arg(QDir::toNativeSeparators(fname)), iRICMainWindowInterface::STATUSBAR_DISPLAYTIME);
	} catch (ErrorMessage& message) {
		QMessageBox::critical(iricMainWindow(), tr("Error"), message);
	}
}


MeasuredData* MeasuredDataFileDataItem::measuredData() const
{
	return m_measuredData;
}

MeasuredDataPointGroupDataItem* MeasuredDataFileDataItem::pointGroupDataItem() const
{
	return m_pointGroupDataItem;
}

MeasuredDataVectorGroupDataItem* MeasuredDataFileDataItem::vectorGroupDataItem() const
{
	return m_vectorGroupDataItem;
}

void MeasuredDataFileDataItem::addCustomMenuItems(QMenu* menu)
{
	menu->addAction(m_exportAction);
}

void MeasuredDataFileDataItem::doApplyOffset(double x, double y)
{
	m_measuredData->applyOffset(x, y);
}