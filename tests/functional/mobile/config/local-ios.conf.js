exports.config = {
    user: process.env.BROWSERSTACK_USERNAME,
    key: process.env.BROWSERSTACK_ACCESS_KEY,
  
    updateJob: false,
    specs: [
      './test/specs/**/*.js'
    ],
    exclude: [],
  
    capabilities: [{
      project: "Local Appium iOS",
      build: 'Local Builds',
      name: 'local_rapha_ios',
      device: 'iPhone 8 Plus',
      os_version: "14.3",
      bundleId: "org.reactjs.native.example"
    }],
  
    logLevel: 'info',
    coloredLogs: true,
    screenshotPath: './errorShots/',
    baseUrl: '',
    waitforTimeout: 10000,
    connectionRetryTimeout: 90000,
    connectionRetryCount: 3,
  
    framework: 'mocha',
    mochaOpts: {
      ui: 'bdd',
      timeout: 20000
    }
  };