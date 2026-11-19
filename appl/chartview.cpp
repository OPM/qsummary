/*
   Copyright 2022 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <appl/chartview.hpp>

#include <QtGui/QMouseEvent>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QLegendMarker>
#include <QLineSeries>
#include <QPointer>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>

#include <algorithm>
#include <cmath>

#include <iostream>

namespace {

class LegendLabel : public QGraphicsTextItem
{
public:
    LegendLabel(QLegendMarker *marker, ChartView *view, QGraphicsItem *parent)
        : QGraphicsTextItem(parent), m_marker(marker), m_view(view)
    {
        setPlainText(marker->label());
        setFont(marker->font());
        setDefaultTextColor(marker->labelBrush().color());
        setToolTip(marker->label());
        setAcceptedMouseButtons(Qt::LeftButton);
        setCursor(Qt::PointingHandCursor);
        document()->setDocumentMargin(0);
        auto option = document()->defaultTextOption();
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        document()->setDefaultTextOption(option);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (m_marker) {
            m_marker->clicked();
            // Rebuilding the legend must wait until this item's event has returned.
            QTimer::singleShot(0, m_view, [view = m_view]() { view->update_legend(); });
        }
        event->accept();
    }

private:
    QPointer<QLegendMarker> m_marker;
    ChartView *m_view;
};

}

ChartView::ChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent),
    m_chart(chart),
    m_isTouching(false),
    m_parent(parent)
{
    setRubberBand(QChartView::RectangleRubberBand);

    m_xaxis_obj = new XaxisTicks(m_chart);
    m_chart->legend()->hide();
    m_legend = new QGraphicsItemGroup(m_chart);
    m_legend->setHandlesChildEvents(false);
    m_legend->setZValue(10);

    connect(m_chart, &QChart::plotAreaChanged, this, [this](const QRectF& plot) {
        // Only here are chart size, margins and plot area guaranteed to be consistent.
        if (plot.height() > 0)
            m_non_plot_height = m_chart->size().height() - plot.height() - m_chart->margins().bottom();
        update_legend();
    });
}

void ChartView::set_legend_visible(bool visible)
{
    m_legend_visible = visible;
    update_legend();
}

void ChartView::update_legend()
{
    if (m_updating_legend)
        return;

    m_updating_legend = true;

    const qreal padding = 12;
    const qreal chart_height = m_chart->size().height();
    const auto margins = m_chart->margins();
    const qreal other = m_non_plot_height;
    const qreal min_plot_height = 120;

    int height = 0;
    for (qreal scale : {1.0, 0.9, 0.8, 0.7}) {
        height = layout_legend(scale);
        if (chart_height - other - (20 + height) >= min_plot_height)
            break;
    }

    m_legend->setVisible(height > 0);
    m_legend->setPos(padding, chart_height - height);

    // The legend never constrains the widget size, so the window stays freely
    // resizable. If the legend still does not fit, the plot shrinks instead.
    const int max_bottom = std::max(20, static_cast<int>(chart_height - other - 20));
    auto new_margins = margins;
    new_margins.setBottom(std::min(20 + height, max_bottom));
    if (margins != new_margins)
        m_chart->setMargins(new_margins);
    m_updating_legend = false;
}

int ChartView::layout_legend(qreal font_scale)
{
    for (auto item : m_legend->childItems())
        delete item;

    if (!m_legend_visible)
        return 0;

    const qreal padding = 12;
    const qreal sample_width = 30;
    const qreal entry_gap = 14;
    const qreal row_gap = 2;
    const qreal width = std::max(qreal(80), m_chart->size().width() - 2 * padding);
    qreal x = 0;
    qreal y = 0;
    qreal row_height = 0;

    for (auto marker : m_chart->legend()->markers()) {
        if (!marker->series()->isVisible() || marker->label().isEmpty())
            continue;

        auto label = new LegendLabel(marker, this, m_legend);
        QFont font = marker->font();
        if (font.pointSizeF() > 0)
            font.setPointSizeF(font.pointSizeF() * font_scale);
        else if (font.pixelSize() > 0)
            font.setPixelSize(std::max(1, qRound(font.pixelSize() * font_scale)));
        label->setFont(font);

        const qreal text_width = std::min(label->boundingRect().width(), width - sample_width);
        const qreal entry_width = sample_width + text_width;
        if (x > 0 && x + entry_width > width) {
            x = 0;
            y += row_height + row_gap;
            row_height = 0;
        }

        label->setTextWidth(text_width);
        label->setPos(x + sample_width, y);
        const qreal height = label->boundingRect().height();
        auto sample = new QGraphicsLineItem(0, height / 2, sample_width - 6, height / 2, label);
        sample->setPos(-sample_width, 0);
        if (auto line = qobject_cast<QLineSeries *>(marker->series()))
            sample->setPen(line->pen());
        sample->setAcceptedMouseButtons(Qt::NoButton);

        row_height = std::max(row_height, height);
        x += entry_width + entry_gap;
    }

    return row_height > 0 ? static_cast<int>(std::ceil(y + row_height + padding)) : 0;
}


void ChartView::keyPressEvent(QKeyEvent *event)
{

    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;

    default:
        QApplication::sendEvent(m_parent, event);
        break;
    }
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    if (scene()) {
         scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
         m_chart->resize(event->size());
         update_legend();
    }

    QGraphicsView::resizeEvent(event);
}


void ChartView::set_xaxis_ticks(const std::vector<std::tuple<std::string, double>>& xaxis_ticks)
{
    m_xaxis_ticks = xaxis_ticks;
    m_xaxis_obj->set_xaxis_ticks(xaxis_ticks);
}

void ChartView::update_geometry()
{
    m_xaxis_obj->prepare_update();
}


void ChartView::update_graphics()
{
    // repaint is not working
    //this->repaint();

    // this works but this is not pretty
    // will keep this for now

    QRect cv_rect = this->rect();

    this->resize(cv_rect.width()+1, cv_rect.height());
    this->resize(cv_rect.width()-1, cv_rect.height());

}

void ChartView::hide_xaxis_obj()
{
    m_xaxis_obj->set_visible(false);
}

void ChartView::show_xaxis_obj()
{
    m_xaxis_obj->set_visible(true);
}