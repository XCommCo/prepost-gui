#include "rawdatapointmaprealimporterfilterdialog.h"
#include "ui_rawdatapointmaprealimporterfilterdialog.h"

RawDataPointmapRealImporterFilterDialog::RawDataPointmapRealImporterFilterDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::RawDataPointmapRealImporterFilterDialog)
{
	ui->setupUi(this);
}

RawDataPointmapRealImporterFilterDialog::~RawDataPointmapRealImporterFilterDialog()
{
	delete ui;
}

int RawDataPointmapRealImporterFilterDialog::filterValue()
{
	return ui->filterSpinBox->value();
}