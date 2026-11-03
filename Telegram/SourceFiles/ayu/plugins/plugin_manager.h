// Mogram plugin discovery and installation layer.
// This code is part of the AyuGram Desktop-derived Mogram project.
#pragma once

#include "ayu/ui/boxes/plugin_info_box.h"

#include <QVector>

namespace Mogram::Plugins {

struct PluginEntry {
	QString filePath;
	Ui::PluginMetadata metadata;
};

enum class InstallResult {
	Installed,
	AlreadyInstalled,
	InvalidPlugin,
	IoError,
};

// Returns the per-user directory containing installed Mogram plugins.
[[nodiscard]] QString InstalledDirectory();

// Installs a validated Python plugin without executing it.
[[nodiscard]] InstallResult Install(
	const QString &sourcePath,
	const Ui::PluginMetadata &metadata);

// Discovers installed Python-style plugins. This function does not execute code.
[[nodiscard]] QVector<PluginEntry> Scan(const QString &rootPath);

} // namespace Mogram::Plugins
