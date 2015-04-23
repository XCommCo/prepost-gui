#include "gridcomplexconditiondialog.h"
#include "gridcomplexconditionwidget.h"
#include "ui_gridcomplexconditiondialog.h"
#include "../../base/iricmainwindowinterface.h"
#include "../base/preprocessorrawdatacomplexgroupdataiteminterface.h"
#include "../../project/projectdata.h"
#include "../../solverdef/solverdefinitiongridrelatedcomplexcondition.h"
#include "../../project/colorsource.h"

#include <QWidget>
#include <QTextCodec>
#include <QMessageBox>

GridComplexConditionDialog::GridComplexConditionDialog(PreProcessorRawDataComplexGroupDataItemInterface* item, iRICMainWindowInterface* mainWindow, QWidget* parent)
	: QDialog(parent),
		ui(new Ui::GridComplexConditionDialog)
{
	ui->setupUi(this);
	m_dataItem = item;
	m_mainWindow = mainWindow;
	m_colorSource = new ColorSource(0);
	connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(selectItem(int)));
	connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addItem()));
	connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(removeItem()));
}

GridComplexConditionDialog::~GridComplexConditionDialog()
{
	delete ui;
	delete m_colorSource;
}

void GridComplexConditionDialog::setWidgets(QList<GridComplexConditionWidget*> widgets)
{
	m_widgets = widgets;
	updateList();

	if (m_widgets.count() > 0){
		ui->listWidget->setCurrentRow(0);
	}
}

void GridComplexConditionDialog::updateList()
{
	ui->listWidget->clear();
	for (int i = 0; i < m_widgets.count(); ++i){
		GridComplexConditionWidget* w = m_widgets.at(i);
		ui->listWidget->addItem(w->caption());
	}
}

void GridComplexConditionDialog::selectItem(int item)
{
	if (item == -1){return;}
	GridComplexConditionWidget* w = m_widgets.at(item);
	ui->widgetContainer->setWidget(w);
	disconnect(this, SLOT(updateCurrentName(QString)));
	connect(w, SIGNAL(captionChanged(QString)), this, SLOT(updateCurrentName(QString)));
	disconnect(this, SLOT(defaultChecked(bool)));
	connect(w, SIGNAL(defaultChecked(bool)), this, SLOT(defaultChecked(bool)));
}

void GridComplexConditionDialog::addItem()
{
	SolverDefinitionGridRelatedComplexCondition* compCond =
			dynamic_cast<SolverDefinitionGridRelatedComplexCondition*>(m_dataItem->condition());
	GridComplexConditionWidget* newWidget = new GridComplexConditionWidget(m_mainWindow, this);
	newWidget->setup(m_dataItem->projectData()->solverDefinition(), compCond->element(), m_mainWindow->locale());
	newWidget->setCaption(QString("Item%1").arg(m_widgets.count() + 1));
	if (m_widgets.count() == 0){
		// this is the first one. make it the default.
		newWidget->setIsDefault(true);
	}
	newWidget->setColor(m_colorSource->getColor(m_widgets.count()));
	m_widgets.append(newWidget);
	updateList();
	ui->listWidget->setCurrentRow(m_widgets.count() - 1);
}

void GridComplexConditionDialog::removeItem()
{
	if (m_widgets.count() == 1){
		// The user tried to remove the last item.
		QMessageBox::warning(this, tr("Warning"), tr("There must be one group at least."));
		return;
	}
	int current = ui->listWidget->currentRow();
	m_widgets.removeAt(current);
	updateList();
	if (m_widgets.count() == 0){
		// removed the last one.
		QWidget* w = new QWidget(this);
		ui->widgetContainer->setWidget(w);
	} else {
		if (current >= m_widgets.count()){
			current = m_widgets.count() - 1;
		}
		ui->listWidget->setCurrentRow(current);
	}
}

void GridComplexConditionDialog::updateCurrentName(const QString& name)
{
	ui->listWidget->item(ui->listWidget->currentRow())->setText(name);
}

void GridComplexConditionDialog::accept()
{
	QTextCodec* asciiCodec = QTextCodec::codecForName("latin1");
	bool allok = true;
	for (int i = 0; i < m_widgets.count(); ++i){
		GridComplexConditionWidget* w = m_widgets.at(i);
		allok = allok && asciiCodec->canEncode(w->caption());
	}
	if (! allok){
		QMessageBox::warning(this, tr("Warning"), tr("Name has to consist of only English characters."));
		return;
	}
	// Check whether one of the items are set to be default.
	int defIndex = -1;
	for (int i = 0; i < m_widgets.count(); ++i){
		if (m_widgets[i]->isDefault()){
			defIndex = i;
		}
	}

	disconnect(this, SLOT(defaultChecked(bool)));
	if (defIndex == -1){
		// if no default specified and there is more than one widget, make the first one default.
		m_widgets[0]->setIsDefault(true);
	}
	QDialog::accept();
}

void GridComplexConditionDialog::defaultChecked(bool checked)
{
	if (! checked){return;}
	int current = ui->listWidget->currentRow();
	for (int i = 0; i < m_widgets.count(); ++i){
		if (i != current){
			m_widgets[i]->setIsDefault(false);
		}
	}
}