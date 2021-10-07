/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTOREVENTRECORDER_H
#define INSPECTOREVENTRECORDER_H

#include <QObject>
#include <QElapsedTimer>

class QFile;
class QMouseEvent;
class QHoverEvent;
class QKeyEvent;
class QWheelEvent;
class QQuickItem;

class InspectorEventRecorder final : public QObject {
  Q_OBJECT

 public:
  enum Mode {
    Picker,
    Recorder,
  };

  InspectorEventRecorder(QObject* parent, Mode mode,
                         const QString& fileName = QString(),
                         bool allEvents = false);

 private:
  bool eventFilter(QObject* obj, QEvent* event) override;

  bool eventFilterPicker(QObject* obj, QEvent* event);
  bool eventFilterRecorder(QObject* obj, QEvent* event);

  void trackMouseEvent(QMouseEvent* event);
  void trackHoverEvent(QHoverEvent* event);
  void trackKeyEvent(QKeyEvent* event);
  void trackWheelEvent(QWheelEvent* event);

  void trackEvent(const QJsonObject& obj);

  QStringList pickElement(QMouseEvent* event, QQuickItem* item);

 private:
  Mode m_mode;
  QString m_fileName;
  bool m_allEvents;
  QElapsedTimer m_timer;
  QFile* m_eventFile = nullptr;
  QEvent* m_lastEvent = nullptr;
};

#endif  // INSPECTOREVENTRECORDER_H
