// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace QT {

  class QMenuBar;
  class QAction;
} // namespace QT

namespace noda {

  class Plugin;

  class QuickMenu {
  public:
	explicit QuickMenu(Plugin &, QT::QMenuBar *);

  private:
	void OpenAboutDialog();

	void OpenSettings();
	void ToggleConnect();

	QAction *_connectAct = nullptr;
	QAction *_projectAct = nullptr;

	Plugin &_plugin;
  };
} // namespace noda