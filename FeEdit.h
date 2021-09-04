#pragma once

#include <QTextEdit>

class QCompleter;

class FeEdit : public QTextEdit
{
public:
  explicit FeEdit(QWidget *parent = nullptr);

protected:
  void keyPressEvent(QKeyEvent *e) override;

private:
  void insertCompletion(const QString &completion);
  QTextCursor textUnderCursor() const;

  QCompleter *m_complete{nullptr};
};