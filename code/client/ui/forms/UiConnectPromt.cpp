
#include "UiConnectPrompt.h"
#include "ui/UiController.h"

UiConnectPromt::UiConnectPromt(UiController &controller) :
	QDialog(QApplication::activeWindow()), _controller(controller) {
  setupUi(this);

  connect(buttonConnect, &QPushButton::clicked, this, &UiConnectPromt::OnConnect);
  connect(buttonCancel, &QPushButton::clicked, this, &UiConnectPromt::close);
  connect(buttonNO, &QPushButton::clicked, this, &UiConnectPromt::SetDontAskagain);
}

void UiConnectPromt::SetDontAskagain() {
  // TODO: notify settings

  QDialog::close();
}

void UiConnectPromt::OnConnect() {
  _controller.DoConnect();
  QDialog::close();
}