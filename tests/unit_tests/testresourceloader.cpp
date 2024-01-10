/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testresourceloader.h"

#include <QQmlAbstractUrlInterceptor>
#include <QQmlApplicationEngine>
#include <QUrl>

#include "addons/addonreplacer.h"
#include "authenticationinapp/authenticationinapp.h"
#include "context/qmlengineholder.h"
#include "feature/feature.h"
#include "helper.h"
#include "languagei18n.h"
#include "localizer.h"
#include "models/licensemodel.h"
#include "resourceloader.h"
#include "settings/settingsholder.h"

class Interceptor final : public QQmlAbstractUrlInterceptor {
 public:
  Interceptor(const QUrl& a, const QUrl& b) : m_a(a), m_b(b) {}

  QUrl intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
    if (url == m_a) {
      return m_b;
    }
    return url;
  }

 private:
  const QUrl m_a;
  const QUrl m_b;
};

void TestResourceLoader::loadFile() {
  SettingsHolder settingsHolder;
  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  ResourceLoader* rl = ResourceLoader::instance();
  QVERIFY(!!rl);

  QCOMPARE(rl->loadFile(":aa"), ":aa");

  Interceptor i(QUrl("qrc:aa"), QUrl("qrc:bb"));

  rl->addUrlInterceptor(&i);
  QCOMPARE(rl->loadFile(":aa"), ":bb");

  rl->removeUrlInterceptor(&i);
  QCOMPARE(rl->loadFile(":aa"), ":aa");
}

void TestResourceLoader::loadDir() {
  SettingsHolder settingsHolder;
  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  ResourceLoader* rl = ResourceLoader::instance();
  QVERIFY(!!rl);

  QCOMPARE(rl->loadDir(":aa"), ":aa");

  Interceptor i(QUrl("qrc:aa/"), QUrl("qrc:bb/"));

  rl->addUrlInterceptor(&i);
  QCOMPARE(rl->loadDir(":aa"), ":bb/");

  rl->removeUrlInterceptor(&i);
  QCOMPARE(rl->loadDir(":aa"), ":aa");
}

void TestResourceLoader::commonPasswords() {
  SettingsHolder settingsHolder;
  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  AuthenticationInApp* aia = AuthenticationInApp::instance();
  QVERIFY(!!aia);

  QCOMPARE(aia->validatePasswordCommons("12345678"), false);

  Interceptor i(QUrl("qrc:/resources/encodedPassword.txt"),
                QUrl("qrc:/replace/encodedPassword.txt"));

  ResourceLoader* rl = ResourceLoader::instance();

  rl->addUrlInterceptor(&i);
  QCOMPARE(aia->validatePasswordCommons("12345678"), true);
  QCOMPARE(aia->validatePasswordCommons("ciaociao"), false);

  rl->removeUrlInterceptor(&i);
  QCOMPARE(aia->validatePasswordCommons("12345678"), false);
}

void TestResourceLoader::languageI18N() {
  SettingsHolder settingsHolder;
  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QCOMPARE(LanguageI18N::instance()->translateLanguage("fr", "tlh"), "klingon");
  QVERIFY(LanguageI18N::instance()->translateLanguage("gr", "tlh").isEmpty());

  Interceptor i(QUrl("qrc:/i18n/languages.json"),
                QUrl("qrc:/replace/languages.json"));

  ResourceLoader* rl = ResourceLoader::instance();

  rl->addUrlInterceptor(&i);
  QVERIFY(LanguageI18N::instance()->translateLanguage("fr", "tlh").isEmpty());
  QCOMPARE(LanguageI18N::instance()->translateLanguage("gr", "tlh"),
           "Κλίνγκον γλώσσα");

  rl->removeUrlInterceptor(&i);
  QCOMPARE(LanguageI18N::instance()->translateLanguage("fr", "tlh"), "klingon");
  QVERIFY(LanguageI18N::instance()->translateLanguage("gr", "tlh").isEmpty());
}

void TestResourceLoader::licenseModel() {
  SettingsHolder settingsHolder;
  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  LicenseModel* lm = LicenseModel::instance();

  lm->initialize();
  QVERIFY(lm->contentLicense().length() > 0);
  QVERIFY(lm->rowCount(QModelIndex()) > 0);

  QString title = lm->data(lm->index(0, 0), LicenseModel::TitleRole).toString();

  Interceptor i(QUrl("qrc:/license/LICENSE.md"),
                QUrl("qrc:/replace/LICENSE.md"));

  ResourceLoader* rl = ResourceLoader::instance();

  rl->addUrlInterceptor(&i);
  QVERIFY(lm->data(lm->index(0, 0), LicenseModel::TitleRole).toString() !=
          title);

  rl->removeUrlInterceptor(&i);
  QCOMPARE(lm->data(lm->index(0, 0), LicenseModel::TitleRole).toString(),
           title);
}

void TestResourceLoader::addon() {
  SettingsHolder settingsHolder;
  Localizer l;

  settingsHolder.setFeaturesFlippedOn(QStringList{"replacerAddon"});
  const_cast<Feature*>(Feature::get(Feature::Feature_replacerAddon))
      ->maybeFlipOnOrOff();

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QJsonArray urls;

  {
    QJsonObject urlObj;
    urlObj["request"] = "qrc:/foo/file.txt";
    urlObj["response"] = "replace/LICENSE.md";
    urls.append(urlObj);
  }

  {
    QJsonObject urlObj;
    urlObj["request"] = "qrc:/dir";
    urlObj["response"] = "replace";
    urlObj["type"] = "directory";
    urls.append(urlObj);
  }

  {
    QJsonObject urlObj;
    urlObj["request"] = "qrc:/dir2/";
    urlObj["response"] = "replace";
    urlObj["type"] = "directory";
    urls.append(urlObj);
  }

  QJsonObject content;
  content["urls"] = urls;

  QJsonObject obj;
  obj["replacer"] = content;

  QObject parent;
  Addon* replacer =
      AddonReplacer::create(&parent, ":/manifest.json", "bar", "name", obj);
  QVERIFY(!!replacer);

  replacer->enable();
  QVERIFY(replacer->enabled());

  ResourceLoader* rl = ResourceLoader::instance();
  QCOMPARE(rl->loadFile(":aa"), ":aa");
  QCOMPARE(rl->loadFile(":/foo/file.txt"), ":/replace/LICENSE.md");
  QCOMPARE(rl->loadFile(":/dir/file.txt"), ":/replace/file.txt");
  QCOMPARE(rl->loadDir(":/dir"), ":/replace/");
  QCOMPARE(rl->loadDir(":/dir/"), ":/replace/");

  settingsHolder.setFeaturesFlippedOff(QStringList{"replacerAddon"});
  const_cast<Feature*>(Feature::get(Feature::Feature_replacerAddon))
      ->maybeFlipOnOrOff();
}

static TestResourceLoader s_testResourceLoader;
