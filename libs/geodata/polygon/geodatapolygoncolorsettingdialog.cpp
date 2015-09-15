#include "ui_geodatapolygoncolorsettingdialog.h"

#include "geodatapolygoncolorsettingdialog.h"

GeoDataPolygonColorSettingDialog::Setting::Setting() :
	CompositeContainer ({&color, &opacity, &mapping}),
	color {"color"},
	opacity {},
	mapping {"mapping", Value}
{
	opacity = 50;
}

GeoDataPolygonColorSettingDialog::GeoDataPolygonColorSettingDialog(QWidget* parent) :
	QDialog {parent},
	ui {new Ui::GeoDataPolygonColorSettingDialog}
{
	ui->setupUi(this);
}

GeoDataPolygonColorSettingDialog::~GeoDataPolygonColorSettingDialog()
{
	delete ui;
}

void GeoDataPolygonColorSettingDialog::setSetting(const Setting& setting)
{
	// mapping
	if (setting.mapping == Value) {
		ui->byValueRadioButton->setChecked(true);
	} else if (setting.mapping == Arbitrary) {
		ui->arbitraryRadioButton->setChecked(true);
	}

	// opacity
	ui->transparencyWidget->setOpacity(setting.opacity);

	// color
	ui->colorEditWidget->setColor(setting.color);
}

GeoDataPolygonColorSettingDialog::Setting GeoDataPolygonColorSettingDialog::setting() const
{
	Setting ret;
	// mapping
	if (ui->byValueRadioButton->isChecked()) {ret.mapping = Value;}
	if (ui->arbitraryRadioButton->isChecked()) {ret.mapping = Arbitrary;}

	// opacity
	ret.opacity = ui->transparencyWidget->opacity();

	// color
	ret.color = ui->colorEditWidget->color();

	return ret;
}
