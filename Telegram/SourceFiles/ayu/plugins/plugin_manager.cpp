// Mogram plugin discovery and installation layer.
// This code is part of the AyuGram Desktop-derived Mogram project.
#include "ayu/plugins/plugin_manager.h"

#include <algorithm>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QStandardPaths>

namespace Mogram::Plugins {
namespace {

constexpr qint64 kMaximumMetadataFileSize = 1024 * 1024;
constexpr qint64 kMaximumPluginFileSize = 20 * 1024 * 1024;

} // namespace

QString InstalledDirectory() {
	const auto appData = QStandardPaths::writableLocation(
		QStandardPaths::AppDataLocation);
	return appData.isEmpty()
		? QString()
		: QDir(appData).filePath(QStringLiteral("plugins"));
}

InstallResult Install(
		const QString &sourcePath,
		const Ui::PluginMetadata &metadata) {
	const auto sourceInfo = QFileInfo(sourcePath);
	if (!sourceInfo.exists()
		|| !sourceInfo.isFile()
		|| sourceInfo.isSymLink()
		|| sourceInfo.size() <= 0
		|| sourceInfo.size() > kMaximumPluginFileSize
		|| !sourceInfo.fileName().endsWith(
			QStringLiteral(".py"),
			Qt::CaseInsensitive)
		|| metadata.id.isEmpty()) {
		return InstallResult::InvalidPlugin;
	}

	auto source = QFile(sourceInfo.absoluteFilePath());
	if (!source.open(QIODevice::ReadOnly)) {
		return InstallResult::IoError;
	}
	const auto contents = source.readAll();
	if (source.error() != QFileDevice::NoError) {
		return InstallResult::IoError;
	}
	const auto parsed = Ui::ParsePluginMetadata(contents);
	if (parsed.id.isEmpty()
		|| parsed.id != metadata.id
		|| parsed.name.isEmpty()) {
		return InstallResult::InvalidPlugin;
	}

	const auto rootPath = InstalledDirectory();
	if (rootPath.isEmpty()) {
		return InstallResult::IoError;
	}
	auto root = QDir(rootPath);
	if (!root.exists() && !QDir().mkpath(rootPath)) {
		return InstallResult::IoError;
	}

	const auto destination = root.filePath(metadata.id + QStringLiteral(".py"));
	if (QFileInfo::exists(destination)) {
		return InstallResult::AlreadyInstalled;
	}
	if (!QFile::copy(sourceInfo.absoluteFilePath(), destination)) {
		return InstallResult::IoError;
	}
	return InstallResult::Installed;
}

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
		if (info.size() <= 0 || info.size() > kMaximumPluginFileSize) {
			continue;
		}

		auto file = QFile(path);
		if (!file.open(QIODevice::ReadOnly)) {
			continue;
		}
		const auto metadata = Ui::ParsePluginMetadata(
			file.read(kMaximumMetadataFileSize));
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
