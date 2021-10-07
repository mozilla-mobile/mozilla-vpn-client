const assert = require('assert');
const firefox = require('selenium-webdriver/firefox');
const webdriver = require('selenium-webdriver');

module.exports = class FirefoxHelper {
  static async createDriver() {
    require('dotenv').config();

    const options = new firefox.Options();
    if (process.env.HEADLESS) {
      options.headless();
    }

    const driver = await new webdriver.Builder()
                       .forBrowser('firefox')
                       .setFirefoxOptions(options)
                       .build();

    return driver;
  }

  static async waitForURL(driver, url) {
    await driver.setContext('content');

    let retries = 50;

    // I'm sure there is something better than this, but this is the only
    // solution to monitor the tab loading so far.
    return await new Promise(resolve => {
      const check = async () => {
        const handles = await driver.getAllWindowHandles();
        for (let handle of handles) {
          await driver.switchTo().window(handle);
          const t = await driver.getCurrentUrl();
          console.log('DEBUG: webdriver URL', t.split('?')[0])
          if (t.includes(url)) {
            resolve(handle);
            return;
          }
        }

        retries--;
        if (('ARTIFACT_DIR' in process.env) && (retries == 0)) {
          const path =
              process.env.ARTIFACT_DIR + '/screencapture/webdriver.png';
          console.log('DEBUG: saving webdriver screenshot to', path);
          driver.takeScreenshot().then(function(image) {
            require('fs').writeFileSync(path, image, 'base64');
          })
        }

        setTimeout(check, 500);
      };

      check();
    });
  }
};
