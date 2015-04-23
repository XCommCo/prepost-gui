#include "rawdatapolygon.h"
#include "rawdatapolygonabstractpolygon.h"
#include "rawdatapolygoncoordinateseditdialog.h"
#include "ui_rawdatapolygoncoordinateseditdialog.h"

#include <guibase/realnumbereditwidget.h>
#include <misc/iricundostack.h>

#include <QPolygonF>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QUndoCommand>
#include <QVector>
#include <QVector2D>

#include <vtkPoints.h>
#include <vtkPolygon.h>

class RawDataPolygonCoordinatesEditDialogDelegate : public QStyledItemDelegate {
public:
	RawDataPolygonCoordinatesEditDialogDelegate(QObject* = 0){}
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/,
		const QModelIndex & /*index*/) const
	{
		RealNumberEditWidget* realEdit = new RealNumberEditWidget(parent);
		return realEdit;
	}
	void setEditorData(QWidget *editor, const QModelIndex &index) const {
		QVariant dat = index.model()->data(index, Qt::DisplayRole);
		RealNumberEditWidget* realEdit = dynamic_cast<RealNumberEditWidget*>(editor);
		realEdit->setValue(dat.toDouble());
	}
	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const
	{
		// Float. Edit with RealEdit
		RealNumberEditWidget* realEdit = dynamic_cast<RealNumberEditWidget*>(editor);
		model->setData(index, realEdit->value(), Qt::EditRole);
	}

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const
	{
		editor->setGeometry(option.rect);
	}
};

class RawDataPolygonCoordinatesEditCommand : public QUndoCommand
{
public:
	RawDataPolygonCoordinatesEditCommand(bool apply, QVector<QVector2D> coords, RawDataPolygon* pol)
		: QUndoCommand(RawDataPolygon::tr("Edit Polygon Coordinates"))
	{
		m_apply = apply;
		m_newCoords = coords;
		m_polygon = pol;
		m_selectedPolygon = pol->m_selectedPolygon;
		double p[3];
		vtkPoints* points = m_selectedPolygon->getVtkPolygon()->GetPoints();
		for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i){
			points->GetPoint(i, p);
			m_oldCoords.append(QVector2D(p[0], p[1]));
		}
	}
	void undo()
	{
		vtkPoints* points = m_selectedPolygon->getVtkPolygon()->GetPoints();
		for (vtkIdType i = 0; i < m_oldCoords.count(); ++i){
			QVector2D v = m_oldCoords.at(i);
			points->SetPoint(i, v.x(), v.y(), 0);
		}
		points->Modified();
		m_selectedPolygon->updateShapeData();
		if (! m_apply){
			m_polygon->renderGraphicsView();
		}
	}
	void redo()
	{
		vtkPoints* points = m_selectedPolygon->getVtkPolygon()->GetPoints();
		for (vtkIdType i = 0; i < m_newCoords.count(); ++i){
			QVector2D v = m_newCoords.at(i);
			points->SetPoint(i, v.x(), v.y(), 0);
		}
		points->Modified();
		m_selectedPolygon->updateShapeData();
		m_polygon->renderGraphicsView();
	}
private:
	bool m_apply;
	QVector<QVector2D> m_newCoords;
	QVector<QVector2D> m_oldCoords;
	RawDataPolygonAbstractPolygon* m_selectedPolygon;
	RawDataPolygon* m_polygon;
};

RawDataPolygonCoordinatesEditDialog::RawDataPolygonCoordinatesEditDialog(RawDataPolygon* polygon, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::RawDataPolygonCoordinatesEditDialog)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);
	m_polygon = polygon;
	m_applyed = false;
	setupData();
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(handleButtonClick(QAbstractButton*)));
}

RawDataPolygonCoordinatesEditDialog::~RawDataPolygonCoordinatesEditDialog()
{
	delete ui;
}

void RawDataPolygonCoordinatesEditDialog::accept()
{
	if (m_applyed){
		// undo the apply action.
		iRICUndoStack::instance().undo();
	}
	QVector<QVector2D> coords = getCoords();
	iRICUndoStack::instance().push(new RawDataPolygonCoordinatesEditCommand(false, coords, m_polygon));
	QDialog::accept();
}

void RawDataPolygonCoordinatesEditDialog::reject()
{
	if (m_applyed){
		// undo the apply action.
		iRICUndoStack::instance().undo();
		m_polygon->m_selectedPolygon->updateShapeData();
		m_polygon->renderGraphicsView();
	}
	QDialog::reject();
}

void RawDataPolygonCoordinatesEditDialog::apply()
{
	if (m_applyed){
		// undo the apply action.
		iRICUndoStack::instance().undo();
	}
	QVector<QVector2D> coords = getCoords();
	iRICUndoStack::instance().push(new RawDataPolygonCoordinatesEditCommand(true, coords, m_polygon));
	m_applyed = true;
}

void RawDataPolygonCoordinatesEditDialog::handleButtonClick(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole){
		apply();
	}
}

void RawDataPolygonCoordinatesEditDialog::setupData()
{
	// setup model first.
	m_model = new QStandardItemModel(0, 2, this);
	m_model->setHeaderData(0, Qt::Horizontal, tr("X"));
	m_model->setHeaderData(1, Qt::Horizontal, tr("Y"));

	vtkPoints* points = m_polygon->m_selectedPolygon->getVtkPolygon()->GetPoints();
	for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i){
		m_model->insertRow(i);
		double p[3];
		points->GetPoint(i, p);
		m_model->setData(m_model->index(i, 0, QModelIndex()), p[0]);
		m_model->setData(m_model->index(i, 1, QModelIndex()), p[1]);
	}
	// set the model into view.
	ui->tableView->setModel(m_model);
	for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i){
		ui->tableView->setRowHeight(i, defaultRowHeight);
	}
	QAbstractItemDelegate* delegate = new RawDataPolygonCoordinatesEditDialogDelegate(this);
	ui->tableView->setItemDelegate(delegate);
}

QVector<QVector2D> RawDataPolygonCoordinatesEditDialog::getCoords()
{
	QVector<QVector2D> ret;
	int rows = m_model->rowCount();
	for (int i = 0; i < rows; ++i){
		double x = m_model->data(m_model->index(i, 0, QModelIndex())).toDouble();
		double y = m_model->data(m_model->index(i, 1, QModelIndex())).toDouble();
		ret.append(QVector2D(x, y));
	}
	return ret;
}