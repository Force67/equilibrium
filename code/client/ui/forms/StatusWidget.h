// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qlabel.h>

namespace noda {
  namespace net {
	struct NetStats;
  }

  namespace ui {
	class StatusWidget final : public QWidget {
	  Q_OBJECT;

	public:
	  StatusWidget(QWidget *parent);
	  ~StatusWidget() = default;

	public slots:
	  void OnConnected();
	  void OnDisconnect(uint32_t);
	  void OnBroadcast(int);
	  void OnStatsUpdate(const net::NetStats &);

	private:
	  void paintEvent(QPaintEvent *) override;
	  QSize sizeHint() const override;

	  void SetLabelIcon(QLabel &, const QString &);

	private:
	  QScopedPointer<QLabel> _labelConnectText;
	  QScopedPointer<QLabel> _labelConnectIcon;

	  QScopedPointer<QLabel> _labelUserText;
	  QScopedPointer<QLabel> _labelUserIcon;

	  QScopedPointer<QLabel> _labelUpText;
	  QScopedPointer<QLabel> _labelDownText;
	  QScopedPointer<QLabel> _labelUpIcon;
	  QScopedPointer<QLabel> _labelDownIcon;

	  int _userCount = 1;
	};
  } // namespace ui
} // namespace noda