/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtaskgetfeaturelist.h"

#include "feature/taskgetfeaturelist.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

namespace {
std::function<bool(NetworkRequest*)> s_noopDeleteResourceCallback =
    [](NetworkRequest*) { return true; };
std::function<bool(NetworkRequest*)> s_noopGetResourceCallback =
    [](NetworkRequest*) { return true; };
std::function<bool(NetworkRequest*, QIODevice*)> s_noopPostIODeviceCallback =
    [](NetworkRequest*, QIODevice*) { return true; };

void stubRequestPostHandler(
    std::function<bool(NetworkRequest*, const QByteArray&)> mockPostHandler) {
  // Clone the callbacks for methods we don't care about.
  auto mockDeleteHandler = s_noopDeleteResourceCallback;
  auto mockGetHandler = s_noopGetResourceCallback;
  auto mockPostIOHandler = s_noopPostIODeviceCallback;

  NetworkRequest::setRequestHandler(
      std::move(mockDeleteHandler), std::move(mockGetHandler),
      std::move(mockPostHandler), std::move(mockPostIOHandler));
}
}  // namespace

void TestTaskGetFeatureList::testSendsAuthedRequestWhenAuthed() {
  // Initialize singletons that are expected to exist.
  SettingsHolder settingsHolder;
  SimpleNetworkManager simpleNetworkManager;

  QNetworkRequest featureListRequest;
  QString featureListRequestBody;

  std::function<bool(NetworkRequest*, const QByteArray&)> mockPostHandler =
      [&](NetworkRequest* request, const QByteArray& requestBody) {
        if (request->m_request.url().toString() ==
            Constants::apiUrl(Constants::FeatureList)) {
          featureListRequest = request->m_request;
          featureListRequestBody = requestBody;
        }

        return true;
      };
  stubRequestPostHandler(mockPostHandler);

  // If a token is set, the task will assume we are authed.
  settingsHolder.setToken("aToken");

  TaskGetFeatureList task;
  task.run();

  QVERIFY(featureListRequest.hasRawHeader("Authorization"));
  QCOMPARE(featureListRequestBody, "{}");

  // Reset request handlers.
  NetworkRequest::resetRequestHandler();
}
void TestTaskGetFeatureList::testAddsExperimeterIdToBodyWhenNotAuthed() {
  // Initialize singletons that are expected to exist.
  SettingsHolder settingsHolder;
  SimpleNetworkManager simpleNetworkManager;

  QNetworkRequest featureListRequest;
  QString featureListRequestBody;

  std::function<bool(NetworkRequest*, const QByteArray&)> mockPostHandler =
      [&](NetworkRequest* request, const QByteArray& requestBody) {
        if (request->m_request.url().toString() ==
            Constants::apiUrl(Constants::FeatureList)) {
          featureListRequest = request->m_request;
          featureListRequestBody = requestBody;
        }

        return true;
      };
  stubRequestPostHandler(mockPostHandler);

  TaskGetFeatureList task;
  task.run();

  QVERIFY(!featureListRequest.hasRawHeader("Authorization"));
  QVERIFY(featureListRequestBody.contains("experimenterId"));
  QVERIFY(
      featureListRequestBody.contains(settingsHolder.unauthedExperimenterId()));

  // Reset request handlers.
  NetworkRequest::resetRequestHandler();
}

void TestTaskGetFeatureList::testUnauthedExperimenterIdIsOnlySetOnce() {
  // Initialize singletons that are expected to exist.
  SettingsHolder settingsHolder;
  SimpleNetworkManager simpleNetworkManager;

  QNetworkRequest featureListRequest;
  QString featureListRequestBody;

  std::function<bool(NetworkRequest*, const QByteArray&)> mockPostHandler =
      [&](NetworkRequest* request, const QByteArray& requestBody) {
        if (request->m_request.url().toString() ==
            Constants::apiUrl(Constants::FeatureList)) {
          featureListRequest = request->m_request;
          featureListRequestBody = requestBody;
        }

        return true;
      };
  stubRequestPostHandler(mockPostHandler);

  // Manually set an experimenter id. The task should not overwrite it.
  QString expectedExperimenterId = "aId";
  settingsHolder.setUnauthedExperimenterId(expectedExperimenterId);

  TaskGetFeatureList task;
  task.run();

  QVERIFY(!featureListRequest.hasRawHeader("Authorization"));
  QVERIFY(featureListRequestBody.contains("experimenterId"));
  QVERIFY(featureListRequestBody.contains(expectedExperimenterId));

  task.run();

  QVERIFY(!featureListRequest.hasRawHeader("Authorization"));
  QVERIFY(featureListRequestBody.contains("experimenterId"));
  QVERIFY(featureListRequestBody.contains(expectedExperimenterId));

  // Reset it again just in case.
  expectedExperimenterId = "anotherId";
  settingsHolder.setUnauthedExperimenterId(expectedExperimenterId);

  task.run();

  QVERIFY(!featureListRequest.hasRawHeader("Authorization"));
  QVERIFY(featureListRequestBody.contains("experimenterId"));
  QVERIFY(featureListRequestBody.contains(expectedExperimenterId));

  // Reset request handlers.
  NetworkRequest::resetRequestHandler();
}

static TestTaskGetFeatureList s_testTaskGetFeatureList;
