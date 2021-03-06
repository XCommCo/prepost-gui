#include "ui_preferencepageproxy.h"

#include "preferencepageproxy.h"

PreferencePageProxy::PreferencePageProxy(QWidget* parent) :
	PreferencePage(parent),
	ui(new Ui::PreferencePageProxy)
{
	ui->setupUi(this);

	switch (m_setting.proxyMode()) {
	case NetworkSetting::ProxyMode::NoProxy:
		ui->noProxyRadioButton->setChecked(true);
		break;
	case NetworkSetting::ProxyMode::SystemSetting:
		ui->systemProxyRadioButton->setChecked(true);
		break;
	case NetworkSetting::ProxyMode::CustomSetting:
		ui->customProxyRadioButton->setChecked(true);
		break;
	}
	ui->proxySiteEdit->setText(m_setting.proxySite());
	ui->proxyPortSpinBox->setValue(m_setting.proxyPort());

	ui->proxyAuthCheckBox->setChecked(m_setting.needAuthentication());
	ui->userNameEdit->setText(m_setting.userName());
	ui->passwordEdit->setText(m_setting.password());
}

PreferencePageProxy::~PreferencePageProxy()
{
	delete ui;
}

void PreferencePageProxy::update()
{
	if (ui->noProxyRadioButton->isChecked()) {
		m_setting.setProxyMode(NetworkSetting::ProxyMode::NoProxy);
	} else if (ui->systemProxyRadioButton->isChecked()) {
		m_setting.setProxyMode(NetworkSetting::ProxyMode::SystemSetting);
	} else if (ui->customProxyRadioButton->isChecked()) {
		m_setting.setProxyMode(NetworkSetting::ProxyMode::CustomSetting);
	}
	m_setting.setProxySite(ui->proxySiteEdit->text());
	m_setting.setProxyPort(ui->proxyPortSpinBox->value());
	m_setting.setNeedAuthentication(ui->proxyAuthCheckBox->isChecked());
	m_setting.setUserName(ui->userNameEdit->text());
	m_setting.setPassword(ui->passwordEdit->text());

	m_setting.save();
}
