#include "fesyntaxhighlighter.h"

FeSyntaxHighlighter::FeSyntaxHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
  HighlightingRule rule;
  const QString groupPatterns[] = {
      QStringLiteral("\\brotate\\b"),  QStringLiteral("\\brotateZ\\b"), QStringLiteral("\\brotateY\\b"),
      QStringLiteral("\\brotateX\\b"), QStringLiteral("\\bgroup\\b"),   QStringLiteral("\\btranslate\\b"),
      QStringLiteral("\\bscale\\b"),   QStringLiteral("\\bhelper\\b"),
  };
  for (const QString &pattern : groupPatterns)
  {
    rule.pattern = QRegularExpression(pattern);
    rule.format = QTextCharFormat();
    rule.format.setForeground(QColor(30, 135, 218));
    rule.format.setFontWeight(QFont::Bold);
    highlightingRules.append(rule);
  }

  const QString valuePatterns[] = {
      QStringLiteral("\\bvec3\\b"),  QStringLiteral("\\blfo\\b"),   QStringLiteral("\\btrue\\b"),
      QStringLiteral("\\bfalse\\b"), QStringLiteral("\\bcolor\\b"),
  };
  for (const QString &pattern : valuePatterns)
  {
    rule.pattern = QRegularExpression(pattern);
    rule.format = QTextCharFormat();
    rule.format.setForeground(Qt::darkRed);
    rule.format.setFontWeight(QFont::Bold);
    highlightingRules.append(rule);
  }

  const QString primitivePatterns[] = {
      QStringLiteral("\\bcube\\b"),
  };
  for (const QString &pattern : primitivePatterns)
  {
    rule.pattern = QRegularExpression(pattern);
    rule.format = QTextCharFormat();
    rule.format.setForeground(QColor(83, 30, 218));
    rule.format.setFontWeight(QFont::Bold);
    highlightingRules.append(rule);
  }

  static const char *primnames[] = {
      "let",   "=",   "if",     "fn",     "mac",       "while",   "quote", "and",  "or",    "do",    "cons",
      "car",   "cdr", "setcar", "setcdr", "list",      "not",     "is",    "atom", "print", "show",  "add",
      "clear", "sin", "cos",    "tan",    "asin",      "acos",    "atan",  "deg",  "rad",   "floor", "ceil",
      "sqrt",  "abs", "max",    "min",    "animation", "require", "fsin",  "fcos",
  };
  for (const auto *pattern : primnames)
  {
    rule.pattern = QRegularExpression(QStringLiteral("\\b") + pattern + QStringLiteral("\\b"));
    rule.format = QTextCharFormat();
    rule.format.setForeground(Qt::darkYellow);
    rule.format.setFontWeight(QFont::Bold);
    highlightingRules.append(rule);
  }

  rule.pattern = QRegularExpression(QStringLiteral("\\b[0-9]+(\\.[0-9]*)?\\b"));
  rule.format = QTextCharFormat();
  rule.format.setForeground(Qt::darkBlue);
  highlightingRules.append(rule);

  rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
  rule.format.setForeground(Qt::darkGreen);
  highlightingRules.append(rule);

  rule.pattern = QRegularExpression(QStringLiteral(";[^\n]*"));
  rule.format.setForeground(Qt::gray);
  highlightingRules.append(rule);
}

void FeSyntaxHighlighter::highlightBlock(const QString &text)
{
  for (const HighlightingRule &rule : qAsConst(highlightingRules))
  {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
    while (matchIterator.hasNext())
    {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
