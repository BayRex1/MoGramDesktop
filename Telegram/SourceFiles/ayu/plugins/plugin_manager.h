// Mogram plugin discovery layer.
// This code is part of the AyuGram Desktop-derived Mogram project.
#pragma once

#include "ayu/ui/boxes/plugin_info_box.h"

#include <QVector>

namespace Mogram::Plugins {

struct PluginEntry {
	QString filePath;
	Ui::PluginMetadata metadata;
};

// Discovers Python-style plugin entry files and reads their declared metadata.
// This function does not execute plugin code.
[[nodiscard]] QVector<PluginEntry> Scan(const QString &rootPath);

} // namespace Mogram::Plugins
