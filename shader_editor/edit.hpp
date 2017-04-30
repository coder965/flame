class LineNumberArea;

class MyEdit : public QPlainTextEdit
{
    LineNumberArea *lineNumberArea;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
protected:
    void resizeEvent(QResizeEvent *e) override;
public:
    MyEdit();
    void drawLineNumberArea(QPaintEvent *e);
    int lineNumberAreaWidth();
};

class LineNumberArea : public QWidget
{
    MyEdit *pEdit;
protected:
    void paintEvent(QPaintEvent *e) override
    {
        pEdit->drawLineNumberArea(e);
    }
public:
    LineNumberArea(MyEdit *_pEdit)
        : QWidget(_pEdit)
    {
        pEdit = _pEdit;
    }
    QSize sizeHint() const override
    {
        return QSize(pEdit->lineNumberAreaWidth(), 0);
    }

};

MyEdit::MyEdit()
{
    QFontMetrics metrics(font());
    setTabStopWidth(4 * metrics.width(' '));

    lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &MyEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &MyEdit::updateLineNumberArea);

    updateLineNumberAreaWidth(0);
}

int MyEdit::lineNumberAreaWidth()
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

void MyEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void MyEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void MyEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height());
}

void MyEdit::drawLineNumberArea(QPaintEvent *e)
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
