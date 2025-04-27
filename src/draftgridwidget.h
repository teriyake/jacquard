#pragma once

#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QWidget>

class DraftGridWidget : public QWidget
{
	Q_OBJECT
public:
	explicit DraftGridWidget(QWidget* parent = nullptr);

	void setGridSize(int rows, int cols);
	QVector<QVector<bool>> getData() const;
	void setData(const QVector<QVector<bool>>& data);

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	void setRotated(bool rotated);

signals:
	void gridChanged();

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	int m_logicalRows;
	int m_logicalCols;
	QVector<QVector<bool>> m_data;
	int m_cellSize;
	QColor m_emptyColor;
	QColor m_filledColor;
	QColor m_gridColor;

	bool m_rotated = false;
	bool m_isDragging = false;
	bool m_dragState = false;

	void updateCell(const QPoint& pos, bool toggleOnClick = true);
	QPoint getLogicalCellFromPos(const QPoint& pos) const;
	int visualRows() const;
	int visualCols() const;
};