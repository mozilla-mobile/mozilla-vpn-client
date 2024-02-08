/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"
#include "float.h"
#include "helper.h"
#include "pinghelper.h"
#include "settingsholder.h"

class TestConnectionHealth final : public TestHelper {
  Q_OBJECT

 private slots:
  void init();
  void cleanup();
  void dnsPingReceived();
  void healthCheckup();
  void updateDnsPingLatency();
  void testTelemetry();

  /**
   * @brief Calculates the Exponentially Weighted Moving Average of the
   * provided observations.
   *
   * @param obs Observations ordered by time from older to newer.
   * @param weight The weight to increase relevance of recent observations.
   * @return int The exponentially weighted moving average.
   */
  int ewma(std::vector<int> obs, float weight) {
    if (obs.empty()) {
      return 0;
    }
    if (obs.size() == 1) {
      return obs.front();
    }
    return weight * obs.back() +
           (1 - weight) * ewma(std::vector(obs.begin(), obs.end() - 1), weight);
  }

 private:
  SettingsHolder* m_settingsHolder = nullptr;
  int getTimingDistCountFromValues(QHash<int, int> values);
  void metricsTest(int expectedStablePeriods, int expectedUnstablePeriods,
                   int expectedNoSignalPeriods,
                   ConnectionHealth::ConnectionStability currentPeriod);
};
