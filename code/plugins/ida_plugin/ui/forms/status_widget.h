// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qlabel.h>

class Plugin;

namespace forms {
class StatusWidget final : public QWidget {
  Q_OBJECT;

 public:
  StatusWidget(QWidget* parent, Plugin&);
  ~StatusWidget() = default;

 private:
  void SetConnected();
  void SetPending();
  void SetDisconnected();

  void paintEvent(QPaintEvent*) override;
  QSize sizeHint() const override;

  void SetLabelIcon(QLabel&, const QString&);

 private:
  QScopedPointer<QLabel> _labelConnectText;
  QScopedPointer<QLabel> _labelConnectIcon;

  QScopedPointer<QLabel> _labelUserText;
  QScopedPointer<QLabel> _labelUserIcon;
};
}  // namespace forms