#ifndef PREFERENCEPAGEGRAPHICSDEFAULT_H
#define PREFERENCEPAGEGRAPHICSDEFAULT_H

#include "preferencepage.h"
#include <QSettings>

namespace Ui {
    class PreferencePageGraphicsDefault;
}

class PreferencePageGraphicsDefault : public PreferencePage
{
    Q_OBJECT

public:
    explicit PreferencePageGraphicsDefault(QWidget *parent = 0);
    ~PreferencePageGraphicsDefault();
	void update();
private:
	QSettings m_settings;
	Ui::PreferencePageGraphicsDefault *ui;

};

#endif // PREFERENCEPAGEGRAPHICSDEFAULT_H