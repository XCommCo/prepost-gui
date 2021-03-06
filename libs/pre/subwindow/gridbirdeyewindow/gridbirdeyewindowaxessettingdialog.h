#ifndef GRIDBIRDEYEWINDOWAXESSETTINGDIALOG_H
#define GRIDBIRDEYEWINDOWAXESSETTINGDIALOG_H

#include <QDialog>

namespace Ui
{
	class GridBirdEyeWindowAxesSettingDialog;
}

class GridBirdEyeWindowAxesSettingDialog : public QDialog
{
	Q_OBJECT

public:
	explicit GridBirdEyeWindowAxesSettingDialog(QWidget* parent = nullptr);
	~GridBirdEyeWindowAxesSettingDialog();
	void setAxesVisible(bool visible);
	bool axesVisible() const;
	void setColor(const QColor& col);
	QColor color() const;

private:
	Ui::GridBirdEyeWindowAxesSettingDialog* ui;
};

#endif // GRIDBIRDEYEWINDOWAXESSETTINGDIALOG_H
