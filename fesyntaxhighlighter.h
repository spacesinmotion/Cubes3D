#ifndef FESYNTAXHIGHLIGHTER_H
#define FESYNTAXHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

class FeSyntaxHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  FeSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule
  {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;
};

#endif // FESYNTAXHIGHLIGHTER_H
