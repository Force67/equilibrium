// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qlabel.h>

namespace noda {
struct NetStats;

class StatusWidget final : public QWidget {
  Q_OBJECT;

 public:
  StatusWidget(QWidget* parent);
  ~StatusWidget() = default;

 public slots:
  void OnConnected();
  void OnDisconnect(uint32_t);
  void OnAnnounce(int);

 private:
  void paintEvent(QPaintEvent*) override;
  QSize sizeHint() const override;

  void SetLabelIcon(QLabel&, const QString&);

 private:
  QScopedPointer<QLabel> _labelConnectText;
  QScopedPointer<QLabel> _labelConnectIcon;

  QScopedPointer<QLabel> _labelUserText;
  QScopedPointer<QLabel> _labelUserIcon;
};
}  // namespace noda