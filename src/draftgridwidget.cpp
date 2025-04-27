#include "draftgridwidget.h"
#include <QDebug>
#include <QPainter>

DraftGridWidget::DraftGridWidget(QWidget* parent)
	: QWidget(parent), m_logicalRows(4), m_logicalCols(4), m_cellSize(20),
	  m_emptyColor(Qt::white), m_filledColor(Qt::black),
	  m_gridColor(Qt::lightGray), m_rotated(false)
{
	setGridSize(m_logicalRows, m_logicalCols);
	setMinimumSize(minimumSizeHint());
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setMouseTracking(true);
}

int DraftGridWidget::visualRows() const
{
	return m_rotated ? m_logicalCols : m_logicalRows;
}

int DraftGridWidget::visualCols() const
{
	return m_rotated ? m_logicalRows : m_logicalCols;
}


void DraftGridWidget::setRotated(bool rotated)
{
	if (m_rotated != rotated)
	{
		m_rotated = rotated;
		updateGeometry();
		update();
	}
}

void DraftGridWidget::setGridSize(int rows, int cols)
{
	if (rows <= 0 || cols <= 0)
	{
		m_logicalRows = 0;
		m_logicalCols = 0;
		m_data.clear();
		updateGeometry();
		update();
		return;
	}

	bool changed = (m_logicalRows != rows || m_logicalCols != cols);

	m_logicalRows = rows;
	m_logicalCols = cols;

	m_data.resize(m_logicalRows);
	for (int r = 0; r < m_logicalRows; ++r)
	{
		m_data[r].resize(m_logicalCols);
	}

	if (changed)
	{
		updateGeometry();
		update();
	}
}

QVector<QVector<bool>> DraftGridWidget::getData() const
{
	return m_data;
}

void DraftGridWidget::setData(const QVector<QVector<bool>>& data)
{
	if (data.isEmpty() || data[0].isEmpty())
	{
		setGridSize(0, 0);
		return;
	}

	int newRows = data.size();
	int newCols = data[0].size();

	m_logicalRows = newRows;
	m_logicalCols = newCols;
	m_data = data;

	updateGeometry();
	update();
}

QSize DraftGridWidget::minimumSizeHint() const
{
	return QSize(visualCols() * m_cellSize + 1, visualRows() * m_cellSize + 1);
}

QSize DraftGridWidget::sizeHint() const
{
	return minimumSizeHint();
}

void DraftGridWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);

	int vRows = visualRows();
	int vCols = visualCols();
	int totalWidth = vCols * m_cellSize;
	int totalHeight = vRows * m_cellSize;

	// painter.fillRect(rect(), Qt::darkGray);

	if (m_logicalRows <= 0 || m_logicalCols <= 0)
		return;

	for (int vr = 0; vr < vRows; ++vr)
	{
		for (int vc = 0; vc < vCols; ++vc)
		{
			QRect cellRect(vc * m_cellSize, vr * m_cellSize, m_cellSize,
						   m_cellSize);

			int lr = m_rotated ? vc : vr;
			int lc = m_rotated ? vr : vc;

			if (lr >= 0 && lr < m_logicalRows && lc >= 0 && lc < m_logicalCols)
			{
				painter.fillRect(cellRect,
								 m_data[lr][lc] ? m_filledColor : m_emptyColor);
			}
			else
			{
				painter.fillRect(cellRect, Qt::gray);
			}
		}
	}

	painter.setPen(m_gridColor);
	for (int vr = 0; vr <= vRows; ++vr)
	{
		painter.drawLine(0, vr * m_cellSize, totalWidth, vr * m_cellSize);
	}
	for (int vc = 0; vc <= vCols; ++vc)
	{
		painter.drawLine(vc * m_cellSize, 0, vc * m_cellSize, totalHeight);
	}
}


QPoint DraftGridWidget::getLogicalCellFromPos(const QPoint& pos) const
{
	int visualCol = pos.x() / m_cellSize;
	int visualRow = pos.y() / m_cellSize;

	int logicalRow = m_rotated ? visualCol : visualRow;
	int logicalCol = m_rotated ? visualRow : visualCol;

	return QPoint(logicalCol, logicalRow);
}


void DraftGridWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isDragging = true;
		QPoint logicalCell = getLogicalCellFromPos(event->pos());
		int lc = logicalCell.x();
		int lr = logicalCell.y();

		if (lr >= 0 && lr < m_logicalRows && lc >= 0 && lc < m_logicalCols)
		{
			m_dragState = m_data[lr][lc];
			updateCell(event->pos());
		}
		else
		{
			m_isDragging = false;
		}
		event->accept();
	}
	else
	{
		QWidget::mousePressEvent(event);
	}
}

void DraftGridWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isDragging && (event->buttons() & Qt::LeftButton))
	{
		updateCell(event->pos(), false);
		event->accept();
	}
	else
	{
		QWidget::mouseMoveEvent(event);
	}
}

void DraftGridWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isDragging = false;
		event->accept();
	}
	else
	{
		QWidget::mouseReleaseEvent(event);
	}
}

void DraftGridWidget::updateCell(const QPoint& pos, bool toggleOnClick)
{
	QPoint logicalCell = getLogicalCellFromPos(pos);
	int lc = logicalCell.x();
	int lr = logicalCell.y();

	if (lr >= 0 && lr < m_logicalRows && lc >= 0 && lc < m_logicalCols)
	{
		bool changed = false;
		if (toggleOnClick && m_isDragging)
		{
			m_data[lr][lc] = !m_data[lr][lc];
			m_dragState = m_data[lr][lc];
			changed = true;
		}
		else if (m_isDragging && !toggleOnClick)
		{
			if (m_data[lr][lc] != m_dragState)
			{
				m_data[lr][lc] = m_dragState;
				changed = true;
			}
		}

		if (changed)
		{
			update();
			emit gridChanged();
		}
	}
}