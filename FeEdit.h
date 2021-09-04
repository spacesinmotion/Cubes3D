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

  void additionalHighlights();

  void specialEditDialog();

  void duplicateSelection();
  void copySelection(bool remove = false);
  void cutSelection();
  void insertSelection();

  void cursorToOuterList(bool select = false);
  void cursorToInnerList(bool select = false);
  void cursorToNextInList(bool select = false);
  void cursorToPrevInList(bool select = false);
  void cursorToListStart(bool select = false);
  void cursorToListEnd(bool select = false);

  QCompleter *m_complete{nullptr};
};