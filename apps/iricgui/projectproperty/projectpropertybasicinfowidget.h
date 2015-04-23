#ifndef PROJECTPROPERTYBASICINFOWIDGET_H
#define PROJECTPROPERTYBASICINFOWIDGET_H

#include <QWidget>

namespace Ui {
	class ProjectPropertyBasicInfoWidget;
}

class ProjectData;

class ProjectPropertyBasicInfoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ProjectPropertyBasicInfoWidget(QWidget *parent = 0);
	~ProjectPropertyBasicInfoWidget();
	void setProjectData(ProjectData* data);

private slots:
	void showSelectCoordinateSystemDialog();
	void showSetOffsetDialog();

private:
	void updateCoordinateSystem();
	void updateCoordinateOffset();
	ProjectData* m_projectData;
	Ui::ProjectPropertyBasicInfoWidget *ui;
};

#endif // PROJECTPROPERTYBASICINFOWIDGET_H