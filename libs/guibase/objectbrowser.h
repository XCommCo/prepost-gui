#ifndef OBJECTBROWSER_H
#define OBJECTBROWSER_H

#include "guibase_global.h"

#include <QDockWidget>

class GUIBASEDLL_EXPORT ObjectBrowser : public QDockWidget
{
	Q_OBJECT

public:
	ObjectBrowser(QWidget *parent = 0);
};

#endif // OBJECTBROWSER_H