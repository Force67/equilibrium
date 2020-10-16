// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "StatusWidget.h"

#include <qevent.h>
#include <qpainter.h>
#include <qpaintdevice.h>

#include "net/NetBase.h"
#include "net/protocol/Announcement_generated.h"

namespace noda {
  constexpr int ksizeOffset = 3;
  constexpr int kseperatorOffset = 6;

  StatusWidget::StatusWidget(QWidget *parent) :
      QWidget(parent)
  {
	setAttribute(Qt::WA_PaintOnScreen);

	auto createLabel = [] {
	  auto *label = new QLabel();
	  label->setAutoFillBackground(false);
	  label->setAttribute(Qt::WA_PaintOnScreen);
	  label->setAttribute(Qt::WA_TranslucentBackground);
	  return label;
	};

	_labelConnectText.reset(createLabel());
	_labelConnectIcon.reset(createLabel());
	_labelUserText.reset(createLabel());
	_labelUserIcon.reset(createLabel());
	_labelUpText.reset(createLabel());
	_labelDownText.reset(createLabel());
	_labelUpIcon.reset(createLabel());
	_labelDownIcon.reset(createLabel());

	_labelUserText->setText("1");
	_labelUpText->setText("0");
	_labelDownText->setText("0");

	SetLabelIcon(*_labelUserIcon, ":/user");
	SetLabelIcon(*_labelUpIcon, ":/arrow_up");
	SetLabelIcon(*_labelDownIcon, ":/arrow_down");

	OnDisconnect(0);
	//QWidget::updateGeometry();
  }

  void StatusWidget::SetLabelIcon(QLabel &labelIcon, const QString &iconName)
  {
	int offset = _labelConnectText->sizeHint().height();
	labelIcon.setPixmap(QPixmap(iconName).scaled(
	    offset, offset,
	    Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }

  QSize StatusWidget::sizeHint() const
  {
	// connect
	auto width = ksizeOffset + _labelConnectText->sizeHint().width();
	width += ksizeOffset + _labelConnectIcon->sizeHint().width();

	// user
	width += ksizeOffset + _labelUserText->sizeHint().width();
	width += ksizeOffset + _labelUserIcon->sizeHint().width();

	// stats
	width += ksizeOffset + _labelUpText->sizeHint().width();
	width += ksizeOffset + _labelDownText->sizeHint().width();
	width += ksizeOffset + _labelDownIcon->sizeHint().width();
	width += ksizeOffset + _labelUpIcon->sizeHint().width();

	// separator
	width += (kseperatorOffset * 2);

	return QSize(width, _labelConnectText->sizeHint().height());
  }

  void StatusWidget::OnConnected()
  {
	SetLabelIcon(*_labelConnectIcon, ":/check");
	_labelConnectText->setText("<font color='green'>Connected</font>");
	updateGeometry();
  }

  void StatusWidget::OnDisconnect(uint32_t r)
  {
	_userCount = 1;
	_labelUserText->setText("1");

	SetLabelIcon(*_labelConnectIcon, ":/times");
	_labelConnectText->setText("<font color='red'>Disconnected</font>");
	updateGeometry();
  }

  void StatusWidget::OnBroadcast(int code)
  {
	switch(code) {
	case protocol::AnnounceType_FirstJoin:
	case protocol::AnnounceType_Joined:
	  _userCount++;
	  break;
	case protocol::AnnounceType_Disconnect:
	  _userCount--;
	  break;
	default:
	  break;
	}

	_labelUserText->setText(QString::number(_userCount));
  }

  void StatusWidget::OnStatsUpdate(const NetStats &stats)
  {
	_labelDownText->setText(QString::number(stats.bwDown));
	_labelUpText->setText(QString::number(stats.bwDown));
  }

  void StatusWidget::paintEvent(QPaintEvent *paintEvent)
  {
	auto dpr = QWidget::devicePixelRatioF();
	auto buffer = QPixmap(
	    QWidget::width() * dpr,
	    QWidget::height() * dpr);

	buffer.setDevicePixelRatio(dpr);
	buffer.fill(Qt::transparent);

	auto peter = QPainter(&buffer);

	//>> Connect:
	auto region = QRegion(QRect(QPoint(0, 0), _labelConnectText->sizeHint()));
	_labelConnectText->render(&peter, QPoint(0, 0), region);

	region = QRegion(QRect(QPoint(0, 0), _labelConnectIcon->sizeHint()));
	auto x = _labelConnectText->sizeHint().width() + ksizeOffset;
	_labelConnectIcon->render(&peter, QPoint(x, 0), region);

	//>> User:
	region = QRegion(QRect(QPoint(0, 0), _labelUserIcon->sizeHint()));
	x += _labelConnectIcon->sizeHint().width() + ksizeOffset + kseperatorOffset;
	_labelUserIcon->render(&peter, QPoint(x, 0), region);

	region = QRegion(QRect(QPoint(0, 0), _labelUserText->sizeHint()));
	x += _labelUserIcon->sizeHint().width() + ksizeOffset;
	_labelUserText->render(&peter, QPoint(x, 0), region);

	//> Net Stats:
	region = QRegion(QRect(QPoint(0, 0), _labelUpIcon->sizeHint()));
	x += _labelUserText->sizeHint().width() + ksizeOffset + kseperatorOffset;
	_labelUpIcon->render(&peter, QPoint(x, 0), region);

	region = QRegion(QRect(QPoint(0, 0), _labelUpText->sizeHint()));
	x += _labelUpIcon->sizeHint().width() + ksizeOffset;
	_labelUpText->render(&peter, QPoint(x, 0), region);

	region = QRegion(QRect(QPoint(0, 0), _labelDownIcon->sizeHint()));
	x += _labelUpText->sizeHint().width() + ksizeOffset;
	_labelDownIcon->render(&peter, QPoint(x, 0), region);

	region = QRegion(QRect(QPoint(0, 0), _labelDownText->sizeHint()));
	x += _labelDownIcon->sizeHint().width() + ksizeOffset;
	_labelDownText->render(&peter, QPoint(x, 0), region);

	peter.end();

	auto papst = QPainter(this);
	papst.drawPixmap(paintEvent->rect(), buffer, buffer.rect());
	papst.end();
  }
} // namespace noda