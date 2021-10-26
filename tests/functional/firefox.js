const assert = require('assert');
const fs = require('fs');
const firefox = require('selenium-webdriver/firefox');
const webdriver = require('selenium-webdriver');

module.exports = class FirefoxHelper {
  static async createDriver() {
    const options = new firefox.Options();
    if (process.env.HEADLESS) {
      options.headless();
    }
    if ('ARTIFACT_DIR' in process.env) {
      // Configure firefox to export HAR files as test artifacts
      options.setPreference(
          'devtools.netmonitor.har.defaultFileName', 'firefox');
      options.setPreference(
          'devtools.netmonitor.har.defaultLogDir', process.env.ARTIFACT_DIR);
      options.setPreference(
          'devtools.netmonitor.har.enableAutoExportToFile', true);
      options.addArguments('-devtools');
    }
    const driver = await new webdriver.Builder()
                       .forBrowser('firefox')
                       .setFirefoxOptions(options)
                       .build();
    return driver;
  }

  static async waitForURL(driver, url) {
    await driver.setContext('content');

    // If the webdriver takes a suspiciously long time, grab a screenshot
    // after about 25 seconds, based on a 500ms polling interval.
    let screenshotRetries = 50;

    // I'm sure there is something better than this, but this is the only
    // solution to monitor the tab loading so far.
    return await new Promise(resolve => {
      const check = async () => {
        const handles = await driver.getAllWindowHandles();
        for (let handle of handles) {
          await driver.switchTo().window(handle);
          const t = await driver.getCurrentUrl();
          if (t.includes(url)) {
            resolve(handle);
            return;
          }
        }

        screenshotRetries--;
        if (('ARTIFACT_DIR' in process.env) && (screenshotRetries == 0)) {
          const dir = process.env.ARTIFACT_DIR + '/screencapture';
          if (!fs.existsSync(dir)) {
            fs.mkdirSync(dir);
          }
          const path = dir + '/webdriver.png';
          driver.takeScreenshot().then(function(image) {
            fs.writeFileSync(path, image, 'base64');
          })
        }

        setTimeout(check, 500);
      };

      check();
    });
  }
};
