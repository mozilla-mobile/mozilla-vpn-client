/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "theme.h"

#include <QDir>
#include <QJSEngine>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#  include <QGuiApplication>
#  include <QStyleHints>
#endif

#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "resourceloader.h"
#include "settingsholder.h"

#ifdef MZ_IOS
#  include "platforms/ios/ioscommons.h"
#endif

#include <QCoreApplication>
#include <QPainter>
#include <QQmlEngine>

namespace {
Logger logger("Theme");
}

Theme::Theme(QObject* parent) : QAbstractListModel(parent) {
  MZ_COUNT_CTOR(Theme);

  connect(ResourceLoader::instance(), &ResourceLoader::cacheFlushNeeded, this,
          [this]() {
            m_themes.clear();
            initialize(QmlEngineHolder::instance()->engine());
          });
}

Theme::~Theme() { MZ_COUNT_DTOR(Theme); }

// static
Theme* Theme::instance() {
  static Theme* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new Theme(qApp);
  }
  return s_instance;
}

void Theme::initialize(QJSEngine* engine) {
  m_themes.clear();

  QDir dir(ResourceLoader::instance()->loadDir(":/nebula/themes"));
  QStringList files = dir.entryList();

  for (const QString& file : files) {
    parseTheme(engine, file);
  }

  if (!loadTheme(SettingsHolder::instance()->theme())) {
    logger.error() << "Failed to load the theme"
                   << SettingsHolder::instance()->theme();
    loadTheme(DEFAULT_THEME);
  }
}

void Theme::parseTheme(QJSEngine* engine, const QString& themeName) {
  logger.debug() << "Parse theme" << themeName;

  QString path(":/nebula/themes/");
  path.append(themeName);

  QJSValue sizingValue;
  QJSValue colorsValue;

  {
    QString resource = path;
    resource.append("/sizing.js");
    QFile file(ResourceLoader::instance()->loadFile(resource));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      logger.error() << "Failed to open the sizing.js for the" << themeName
                     << "theme";
      return;
    }

    sizingValue = engine->evaluate(file.readAll());
    if (sizingValue.isError()) {
      logger.error() << "Exception processing the sizing.js:"
                     << sizingValue.toString();
      return;
    }

    if (!sizingValue.isObject()) {
      logger.error() << "sizing.js must expose an object";
      return;
    }
  }

  QByteArray completeColorFileBytes = QByteArray();
  // The files in the next line must be in the specific order so that they
  // create a working JS object when appended.
  QList<QString> colorFiles = {"/../colors.js", "/theme.js",
                               "/../theme-derived.js"};
  for (QString colorFile : colorFiles) {
    QString resource = path;
    resource.append(colorFile);
    QFile file(resource);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      logger.error() << "Failed to open " << colorFile << "for the" << themeName
                     << "theme";
      return;
    }
    QByteArray colorFileBytes = file.readAll();
    completeColorFileBytes.append(colorFileBytes);
  }

  colorsValue = engine->evaluate(completeColorFileBytes);
  if (colorsValue.isError()) {
    logger.error() << "Exception processing the colors:"
                   << colorsValue.toString();
    return;
  }

  if (!colorsValue.isObject()) {
    logger.error() << "The combined 3 color files must expose an object";
    return;
  }

  ThemeData* data = new ThemeData();
  data->theme = sizingValue;
  data->colors = colorsValue;
  m_themes.insert(themeName, data);
}

void Theme::setCurrentTheme(const QString& themeName) {
  loadTheme(themeName);
  SettingsHolder::instance()->setTheme(themeName);
}

bool Theme::loadTheme(const QString& themeName) {
  if (!m_themes.contains(themeName)) return false;
  m_currentTheme = themeName;
  emit changed();
  return true;
}

const QJSValue& Theme::readTheme() const {
  Q_ASSERT(m_themes.contains(m_currentTheme));
  ThemeData* data = m_themes.value(m_currentTheme);
  return data->theme;
}

const QJSValue& Theme::readColors() const {
  Q_ASSERT(m_themes.contains(m_currentTheme));
  ThemeData* data = m_themes.value(m_currentTheme);
  return data->colors;
}

QHash<int, QByteArray> Theme::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  return roles;
}

int Theme::rowCount(const QModelIndex&) const {
  return static_cast<int>(m_themes.count());
}

QVariant Theme::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  QStringList themes = m_themes.keys();
  themes.sort();

  switch (role) {
    case NameRole:
      return QVariant(themes.at(index.row()));

    default:
      return QVariant();
  }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
Qt::ColorScheme Theme::currentSystemTheme() {
  QStyleHints* styleHints = QGuiApplication::styleHints();
  Qt::ColorScheme currentColorScheme = styleHints->colorScheme();
  logger.debug() << "Current system theme: " << currentColorScheme;
  return currentColorScheme;
}
#endif
#ifdef MZ_WINDOWS
#  include <dwmapi.h>

#  include <QWindow>
#  pragma comment(lib, "dwmapi.lib")
#endif

void Theme::setStatusBarTextColor([[maybe_unused]] StatusBarTextColor color) {
#ifdef MZ_IOS
  IOSCommons::setStatusBarTextColor(color);
#endif
}

bool Theme::usesDarkModeAssets() const {
  if (readColors().hasProperty("useDarkAssets") &&
      readColors().property("useDarkAssets").isBool()) {
    return readColors().property("useDarkAssets").toBool();
  }
  // This value should always be available
  Q_ASSERT(false);
  return true;
}

#include <QPainter>
#include <QSvgRenderer>

QImage Theme::getTitleBarIcon() {
  bool isDarkmode = isThemeDark();
  QString svgPath = ":/ui/resources/logo.svg";

  QImage image(32, 32, QImage::Format_ARGB32);
  image.fill(Qt::transparent);  // Ensure transparency

  // Load the SVG
  QSvgRenderer svgRenderer(svgPath);
  if (!svgRenderer.isValid()) {
    qWarning() << "Failed to load SVG: " << svgPath;
    return QImage();
  }

  // Paint the SVG onto the QImage
  QPainter painter(&image);
  svgRenderer.render(&painter, QRectF(0, 0, 32, 32));
  painter.end();

  // Invert the pixels if in dark mode
  if (isDarkmode) {
    for (int y = 0; y < image.height(); ++y) {
      QRgb* scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
      for (int x = 0; x < image.width(); ++x) {
        QRgb pixel = scanLine[x];
        // Invert RGB but keep the alpha channel
        scanLine[x] = qRgba(255 - qRed(pixel), 255 - qGreen(pixel),
                            255 - qBlue(pixel), qAlpha(pixel));
      }
    }
  }
  return image;
}
