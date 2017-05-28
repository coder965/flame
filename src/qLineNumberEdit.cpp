#include"qLineNumberEdit.h"

QLineNumberEdit::QLineNumberEdit(QWidget *p)
	: QPlainTextEdit(p)
{
	QFontMetrics metrics(font());
	setTabStopWidth(4 * metrics.width(' '));

	lineNumberArea = new QLineNumberArea(this);

	connect(this, &QPlainTextEdit::blockCountChanged, this, &QLineNumberEdit::updateLineNumberAreaWidth);
	connect(this, &QPlainTextEdit::updateRequest, this, &QLineNumberEdit::updateLineNumberArea);

	updateLineNumberAreaWidth(0);
}

int QLineNumberEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;


    return space;
}

void QLineNumberEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void QLineNumberEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void QLineNumberEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height());
}

void QLineNumberEdit::drawLineNumberArea(QPaintEvent *e)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(e->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= e->rect().bottom())
    {
        if (block.isVisible() && bottom >= e->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
            Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom += (int) blockBoundingRect(block).height();
        blockNumber++;
    }
}

QSize QLineNumberEdit::sizeHint() const
{
	return QSize(1440, 900);
}

void QLineNumberArea::paintEvent(QPaintEvent *e)
{
	pEdit->drawLineNumberArea(e);
}

QLineNumberArea::QLineNumberArea(QLineNumberEdit *_pEdit)
	: QWidget(_pEdit)
{
	pEdit = _pEdit;
}

QSize QLineNumberArea::sizeHint() const
{
	return QSize(pEdit->lineNumberAreaWidth(), 0);
}