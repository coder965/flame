#ifndef __QLINENUMBEREDIT__
#define __QLINENUMBEREDIT__

#include <QTextBlock>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPainter>

class QLineNumberArea;
class QLineNumberEdit : public QPlainTextEdit
{
	QLineNumberArea *lineNumberArea;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
protected:
    void resizeEvent(QResizeEvent *e) override;
public:
	QLineNumberEdit(QWidget *p = nullptr);
    void drawLineNumberArea(QPaintEvent *e);
    int lineNumberAreaWidth();
};

class QLineNumberArea : public QWidget
{
	QLineNumberEdit *pEdit;
protected:
	void paintEvent(QPaintEvent *e) override;
public:
	QLineNumberArea(QLineNumberEdit *_pEdit);
	QSize sizeHint() const override;

};

#endif