// Mogram plugin discovery layer.
// This code is part of the AyuGram Desktop-derived Mogram project.
#include "ayu/plugins/plugin_manager.h"

#include <algorithm>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>

namespace Mogram::Plugins {
namespace {

constexpr qint64 kMaximumMetadataFileSize = 1024 * 1024;

} // namespace

QVector<PluginEntry> Scan(const QString &rootPath) {
	auto result = QVector<PluginEntry>();
	const auto root = QDir(rootPath);
	if (!root.exists()) {
		return result;
	}

	auto seenIds = QSet<QString>();
	auto iterator = QDirIterator(
		root.absolutePath(),
		{ QStringLiteral("*.py") },
		QDir::Files | QDir::Readable | QDir::NoSymLinks,
		QDirIterator::Subdirectories);

	while (iterator.hasNext()) {
		const auto path = iterator.next();
		const auto info = QFileInfo(path);
		if (info.size() <= 0 || info.size() > kMaximumMetadataFileSize) {
			continue;
		}

		auto file = QFile(path);
		if (!file.open(QIODevice::ReadOnly)) {
			continue;
		}
		const auto metadata = Ui::ParsePluginMetadata(file.readAll());
		if (metadata.id.isEmpty() || seenIds.contains(metadata.id)) {
			continue;
		}

		seenIds.insert(metadata.id);
		result.push_back({
			.filePath = info.absoluteFilePath(),
			.metadata = metadata,
		});
	}

	std::sort(
		result.begin(),
		result.end(),
		[](const PluginEntry &a, const PluginEntry &b) {
			return a.metadata.name.localeAwareCompare(b.metadata.name) < 0;
		});
	return result;
}

} // namespace Mogram::Plugins
