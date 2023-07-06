# Steps to setup local environment:

## Prerequisites

Before proceeding with the setup, make sure you have the following software installed on your system:

1. Java Development Kit (JDK) 11
2. Appium 2.0
3. Android SDK (for Android testing)
4. Xcode (for iOS testing)

## Installation Steps

Follow the steps below to set up your local environment:

### Java Development Kit (JDK) 11

* Install JDK 11 from the official Oracle website or using your package manager. (I'm using temurin-11)
* Set up the JAVA_HOME environment variable to point to your JDK 11 installation directory.

### Appium

* Install Appium globally by running the following command:
```
npm install -g appium@2.0
```

### Android SDK (for Android testing)

* Install Android SDK and set up necessary environment variables. ($ANDROID_HOME)
* Download and install Android Studio from the official website.
* Launch Android Studio and go to Preferences (on macOS) or Settings (on Windows/Linux).
* Navigate to Appearance & Behavior > System Settings > Android SDK.
* Install the desired Android SDK versions and necessary platform tools.

### Xcode (for iOS testing)

* Install Xcode from the Mac App Store.
* Install the necessary iOS simulators and developer tools through Xcode's Preferences > Components tab.

## Project Setup

1. Pull latest from the remote repository
2. Navigate to tests/ui directory and run following command:
```
gradle build
```
3. Configure the test suite and test cases according to your needs using TestNG. Modify the testng.xml file to include your desired test cases.

## Configure Appium

1. Make sure Appium installed correctly by running `appium` in your console 
2. If previous step worked correctly kill the process and install drivers:
```
appium driver install uiautomator2
appium driver install xcuitest
```
3. Run `appium-doctor` and verify that all necessary dependencies are checked/green

## Other pre-requisites to consider before starting a test run

* Add an apk/.app to the root folder `/ui`
* Start emulator/simulator prior to starting a test run
* Edit `device` & `test` properties is `resources/configs` to give path to apk/app and specify your emulator details like `deviceName, platformName, platformVersion, PLATFORM, udid, etc.`

## Running Tests

1. Run `appium` in terminal to start local appium instance
2. Run `gradle test` in terminal or simply start a test from the test class itself or run `testng.xml` file
3. To generate local allure report execute `allure serve build/allure-results` 


