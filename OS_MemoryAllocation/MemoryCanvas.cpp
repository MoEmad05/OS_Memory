#include "MemoryCanvas.h"
#include <QRandomGenerator>

void MemoryCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Setup Margins
    int leftMargin = 80;
    int topMargin = 40;
    int bottomMargin = 40;
    int rightMargin = 40;

    // Background for the ruler area (Optional/Debug)
    painter.fillRect(0, 0, leftMargin, height(), QColor(245, 245, 245));

    // Handle initialization state
    if (totalMemorySize <= 0) {
        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, "Initialize Memory to Start");
        return;
    }

    int drawHeight = height() - topMargin - bottomMargin;
    int drawWidth = width() - leftMargin - rightMargin;
    double scaleY = (double)drawHeight / totalMemorySize;

    // --- 2. DRAW TIMELINE (ADDRESS RULER) ---
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(leftMargin - 10, topMargin, leftMargin - 10, topMargin + drawHeight);

    painter.setFont(QFont("Arial", 9, QFont::Bold));
    for (int i = 0; i <= 10; ++i) {
        int addr = (totalMemorySize * i) / 10;
        int y = topMargin + (addr * scaleY);

        painter.setPen(QPen(Qt::black, 1));
        painter.drawLine(leftMargin - 20, y, leftMargin - 10, y);

        // Use a rect for drawText to align it properly next to the markers
        painter.drawText(5, y - 10, leftMargin - 30, 20, Qt::AlignRight | Qt::AlignVCenter, QString::number(addr));
    }

    // --- 3. DRAW MEMORY BLOCKS ---
    painter.translate(leftMargin, topMargin);

    // Draw a default background (representing Reserved/Internal Memory)
    painter.fillRect(0, 0, drawWidth, drawHeight, QColor(200, 200, 200));

    // Draw Allocated Segments
    for (const Process& p : currentProcesses) {
        uint hash = qHash(p.name);
        QColor procColor = QColor::fromHsv(hash % 360, 150, 240);

        for (const Segment& seg : p.segments) {
            QRect rect(0, seg.base * scaleY, drawWidth, seg.size * scaleY);
            painter.setBrush(procColor);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(rect);

            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", 9, QFont::Bold));
            painter.drawText(rect, Qt::AlignCenter, p.name + "\n" + seg.name + "\n" + QString::number(seg.size) + "K");
        }
    }

    // Draw Free Holes
    for (const Hole& hole : currentHoles) {
        QRect rect(0, hole.base * scaleY, drawWidth, hole.size * scaleY);
        painter.setBrush(Qt::white);
        painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter.drawRect(rect);

        painter.setPen(Qt::darkGreen);
        painter.drawText(rect, Qt::AlignCenter, "FREE HOLE\n" + QString::number(hole.size) + "K");
    }
}


// Ensure this matches the header exactly!
MemoryCanvas::MemoryCanvas(QWidget *parent) : QWidget(parent) {
    totalMemorySize = 0;
}

void MemoryCanvas::updateMemory(int totalSize, const QVector<Hole>& holes, const QVector<Process>& processes) {
    this->totalMemorySize = totalSize;
    this->currentHoles = holes;
    this->currentProcesses = processes;
    update(); // This triggers the paintEvent
}