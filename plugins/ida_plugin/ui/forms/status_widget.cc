// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "status_widget.h"
#include "plugin.h"

#include <qevent.h>
#include <qpaintdevice.h>
#include <qpainter.h>

namespace forms {

namespace {
// vertical size of objects
constexpr int ksizeOffset = 3;

// space between items
constexpr int kseperatorOffset = 6;
}  // namespace

StatusWidget::StatusWidget(QWidget* parent, Plugin& plugin) : QWidget(parent) {
  setAttribute(Qt::WA_PaintOnScreen);

  auto createLabel = [] {
    auto* label = new QLabel();
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
  SetDisconnected();

  connect(
      &plugin.Sync(), &IdaSync::StateChange, this,
      [&](IdaSync::State newState) {
        switch (newState) {
          case IdaSync::State::kActive:
            return SetConnected();
          case IdaSync::State::kDisabled:
            return SetDisconnected();
          case IdaSync::State::kPending:
            return SetPending();
          default:
            break;
        }
      },
      Qt::QueuedConnection);

  connect(
      &plugin.Sync(), &IdaSync::Notify, this,
      [&](IdaSync::Notification code) {
        if (code == IdaSync::Notification::kUserJoined ||
            code == IdaSync::Notification::kUsersJoined ||
            code == IdaSync::Notification::kUserQuit) {
          _labelUserText->setText(
              QString::number(plugin.Sync().userCount));
        }
      },
      Qt::QueuedConnection);
}

void StatusWidget::SetLabelIcon(QLabel& labelIcon, const QString& iconName) {
  int offset = _labelConnectText->sizeHint().height();
  labelIcon.setPixmap(QPixmap(iconName).scaled(
      offset, offset, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QSize StatusWidget::sizeHint() const {
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

void StatusWidget::SetConnected() {
  SetLabelIcon(*_labelConnectIcon, ":/globe_green");
  _labelConnectText->setText("<font color='green'>Connected</font>");
  updateGeometry();
}

void StatusWidget::SetPending() {
  SetLabelIcon(*_labelConnectIcon, ":/spinner");
  _labelConnectText->setText("<font color='orange'>Connecting</font>");
  updateGeometry();
}

void StatusWidget::SetDisconnected() {
  _labelUserText->setText("1");

  SetLabelIcon(*_labelConnectIcon, ":/globe_red");
  _labelConnectText->setText("<font color='red'>Disconnected</font>");
  updateGeometry();
}

void StatusWidget::paintEvent(QPaintEvent* paintEvent) {
  auto dpr = QWidget::devicePixelRatioF();
  auto buffer = QPixmap(QWidget::width() * dpr, QWidget::height() * dpr);

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
}  // namespace forms