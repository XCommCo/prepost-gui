#include "../post2dgridregionselectdialog.h"
#include "post2dwindowarrowunstructuredsettingdialog.h"
#include "ui_post2dwindowarrowunstructuredsettingdialog.h"

#include <guicore/postcontainer/postzonedatacontainer.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>

#include <QtGlobal>

#include <vtkPointData.h>
#include <vtkStructuredGrid.h>

Post2dWindowArrowUnstructuredSettingDialog::Post2dWindowArrowUnstructuredSettingDialog(QWidget *parent) :
		QDialog(parent),
		ui(new Ui::Post2dWindowArrowUnstructuredSettingDialog)
{
		ui->setupUi(this);
		ui->colorbarSettingButton->hide();

	connect(ui->specificRadioButton, SIGNAL(toggled(bool)), ui->colorEditWidget, SLOT(setEnabled(bool)));
	connect(ui->scalarRadioButton, SIGNAL(toggled(bool)), ui->scalarComboBox, SLOT(setEnabled(bool)));
	connect(ui->regionSettingButton, SIGNAL(clicked()), this, SLOT(showRegionDialog()));
}

Post2dWindowArrowUnstructuredSettingDialog::~Post2dWindowArrowUnstructuredSettingDialog()
{
		delete ui;
}

void Post2dWindowArrowUnstructuredSettingDialog::setZoneData(PostZoneDataContainer* zoneData)
{
	vtkPointSet* p = zoneData->data();
	ui->samplingNumberSpinBox->setMaximum(p->GetNumberOfPoints());
	setupSolutionComboBox(zoneData);
}

void Post2dWindowArrowUnstructuredSettingDialog::setSolution(const QString& sol)
{
	int index = m_solutions.indexOf(sol);
	if (index == -1){index = 0;}
	ui->solutionComboBox->setCurrentIndex(index);
}

const QString Post2dWindowArrowUnstructuredSettingDialog::solution()
{
	int index = ui->solutionComboBox->currentIndex();
	return m_solutions.at(index);
}

void Post2dWindowArrowUnstructuredSettingDialog::setScalarValue(const QString& scalar)
{
	int index = m_scalars.indexOf(scalar);
	if (index == -1) index = 0;
	ui->scalarComboBox->setCurrentIndex(index);
}

const QString Post2dWindowArrowUnstructuredSettingDialog::scalarValue()
{
	int index = ui->scalarComboBox->currentIndex();
	return m_scalars.at(index);
}

void Post2dWindowArrowUnstructuredSettingDialog::setColor(const QColor& color)
{
	ui->colorEditWidget->setColor(color);
}

const QColor Post2dWindowArrowUnstructuredSettingDialog::color() const
{
	return ui->colorEditWidget->color();
}

void Post2dWindowArrowUnstructuredSettingDialog::setSamplingMode(Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::SamplingMode sm)
{
	if (sm == Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::smAll){
		ui->samplingAllRadioButton->setChecked(true);
	}else if (sm == Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::smRate){
		ui->samplingRateRadioButton->setChecked(true);
	}else{
		ui->samplingNumberRadioButton->setChecked(true);
	}
}

Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::SamplingMode Post2dWindowArrowUnstructuredSettingDialog::samplingMode()
{
	if (ui->samplingAllRadioButton->isChecked()){
		return Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::smAll;
	}else if (ui->samplingRateRadioButton->isChecked()){
		return Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::smRate;
	}else{
		return Post2dWindowNodeVectorArrowGroupUnstructuredDataItem::smNumber;
	}
}

void Post2dWindowArrowUnstructuredSettingDialog::setSamplingRate(int rate)
{
	ui->samplingRateSpinBox->setValue(rate);
}

int Post2dWindowArrowUnstructuredSettingDialog::samplingRate()
{
	return ui->samplingRateSpinBox->value();
}

void Post2dWindowArrowUnstructuredSettingDialog::setSamplingNumber(int num)
{
	ui->samplingNumberSpinBox->setValue(num);
}

int Post2dWindowArrowUnstructuredSettingDialog::samplingNumber()
{
	return ui->samplingNumberSpinBox->value();
}

void Post2dWindowArrowUnstructuredSettingDialog::setMapping(Post2dWindowNodeVectorArrowGroupDataItem::Mapping mapping)
{
	switch (mapping){
	case Post2dWindowNodeVectorArrowGroupDataItem::Specific:
		ui->specificRadioButton->setChecked(true);
		break;
	case Post2dWindowNodeVectorArrowGroupDataItem::Scalar:
		ui->scalarRadioButton->setChecked(true);
		break;
	}
}

Post2dWindowNodeVectorArrowGroupDataItem::Mapping Post2dWindowArrowUnstructuredSettingDialog::mapping()
{
	if (ui->specificRadioButton->isChecked()) return Post2dWindowNodeVectorArrowGroupDataItem::Specific;
	if (ui->scalarRadioButton->isChecked()) return Post2dWindowNodeVectorArrowGroupDataItem::Scalar;
	// default
	return Post2dWindowNodeVectorArrowGroupDataItem::Specific;
}

void Post2dWindowArrowUnstructuredSettingDialog::setArrowSetting(const ArrowSettingContainer& arrowSetting)
{
	ui->arrowSizeSpinBox->setValue(arrowSetting.arrowSize());
	ui->lineWidthSpinBox->setValue(arrowSetting.lineWidth());
}

ArrowSettingContainer Post2dWindowArrowUnstructuredSettingDialog::arrowSetting()
{
	ArrowSettingContainer c;
	c.setArrowSize(ui->arrowSizeSpinBox->value());
	c.setLineWidth(ui->lineWidthSpinBox->value());
	return c;
}

void Post2dWindowArrowUnstructuredSettingDialog::setRegionMode(StructuredGridRegion::RegionMode regionMode)
{
	m_regionMode = regionMode;
}

StructuredGridRegion::RegionMode Post2dWindowArrowUnstructuredSettingDialog::regionMode() const
{
	return m_regionMode;
}


void Post2dWindowArrowUnstructuredSettingDialog::setupSolutionComboBox(PostZoneDataContainer* zoneData)
{
	vtkPointData* pd = zoneData->data()->GetPointData();
	SolverDefinitionGridType* gt = zoneData->gridType();
	int num = pd->GetNumberOfArrays();
	ui->solutionComboBox->blockSignals(true);
	for (int i = 0; i < num; ++i){
		vtkAbstractArray* tmparray = pd->GetArray(i);
		if (tmparray == 0){continue;}
		QString name = tmparray->GetName();
		if (pd->GetArray(i)->GetNumberOfComponents() <= 1){
			// scalar attributes.
			ui->scalarComboBox->addItem(gt->solutionCaption(name));
			m_scalars.append(name);
		} else {
			// vector attributes.
			ui->solutionComboBox->addItem(gt->solutionCaption(name));
			m_solutions.append(name);
		}
	}
	ui->solutionComboBox->blockSignals(false);
	if (m_solutions.count() <= 1){
		ui->physValLabel->hide();
		ui->solutionComboBox->hide();
	}
}

void Post2dWindowArrowUnstructuredSettingDialog::showRegionDialog()
{
	Post2dGridRegionSelectDialog dialog(this);
	if (! m_activeAvailable){
		dialog.disableActive();
	}
	dialog.hideCustom();
	dialog.setRegionMode(m_regionMode);
	int ret = dialog.exec();
	if (ret == QDialog::Rejected){return;}
	m_regionMode = dialog.regionMode();
}

void Post2dWindowArrowUnstructuredSettingDialog::disableActive()
{
	m_activeAvailable = false;
}

void Post2dWindowArrowUnstructuredSettingDialog::setLengthMode(Post2dWindowNodeVectorArrowGroupDataItem::LengthMode lm)
{
	if (lm == Post2dWindowNodeVectorArrowGroupDataItem::lenAuto){
		ui->lengthAutoCheckBox->setChecked(true);
	} else {
		ui->lengthAutoCheckBox->setChecked(false);
	}
}

Post2dWindowNodeVectorArrowGroupDataItem::LengthMode Post2dWindowArrowUnstructuredSettingDialog::lengthMode()
{
	if (ui->lengthAutoCheckBox->isChecked()){
		return Post2dWindowNodeVectorArrowGroupDataItem::lenAuto;
	} else {
		return Post2dWindowNodeVectorArrowGroupDataItem::lenCustom;
	}
}

void Post2dWindowArrowUnstructuredSettingDialog::setStandardValue(double stdVal)
{
	ui->stdValueSpinBox->setValue(stdVal);
}

double Post2dWindowArrowUnstructuredSettingDialog::standardValue()
{
	return ui->stdValueSpinBox->value();
}

void Post2dWindowArrowUnstructuredSettingDialog::setLegendLength(int len)
{
	ui->legendLengthSpinBox->setValue(len);
}

int Post2dWindowArrowUnstructuredSettingDialog::legendLength()
{
	return ui->legendLengthSpinBox->value();
}

void Post2dWindowArrowUnstructuredSettingDialog::setMinimumValue(double minVal)
{
	return ui->minValueSpinBox->setValue(minVal);
}

double Post2dWindowArrowUnstructuredSettingDialog::minimumValue()
{
	return ui->minValueSpinBox->value();
}