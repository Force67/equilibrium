// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "StatusWidget.h"

#include <qevent.h>
#include <qpainter.h>
#include <qpaintdevice.h>

namespace forms {
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
	_labelUserText->setText("1");

	SetLabelIcon(*_labelUserIcon, ":/user");

	OnDisconnect(0);
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

	// separator
	width += (kseperatorOffset * 1 /*SpacerCount*/);

	return QSize(width, _labelConnectText->sizeHint().height());
  }

  void StatusWidget::OnConnected()
  {
	SetLabelIcon(*_labelConnectIcon, ":/globe_green");
	_labelConnectText->setText("<font color='green'>Connected</font>");
	updateGeometry();
  }

  void StatusWidget::OnDisconnect(uint32_t r)
  {
	_labelUserText->setText("1");

	SetLabelIcon(*_labelConnectIcon, ":/globe_red");
	_labelConnectText->setText("<font color='red'>Disconnected</font>");
	updateGeometry();
  }

  void StatusWidget::OnAnnounce(int count)
  {
	_labelUserText->setText(QString::number(count));
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

	peter.end();

	auto papst = QPainter(this);
	papst.drawPixmap(paintEvent->rect(), buffer, buffer.rect());
	papst.end();
  }
} // namespace forms