package core;

import io.appium.java_client.AppiumDriver;
import config.*;
import io.appium.java_client.android.AndroidDriver;
import io.appium.java_client.android.options.UiAutomator2Options;
import io.appium.java_client.ios.IOSDriver;
import io.appium.java_client.ios.options.XCUITestOptions;
import io.qameta.allure.Allure;
import io.qameta.allure.Step;
import org.openqa.selenium.remote.DesiredCapabilities;
import org.testng.annotations.AfterClass;
import org.testng.annotations.AfterMethod;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.BeforeMethod;

import java.net.URL;

public class BaseTest {

		protected AppiumDriver driver;
		private static String appiumUrl = ConfigReader.testConfig.remoteURL();

		@BeforeMethod
		@Step("Setup driver")
		protected void setUp() throws Exception {
				Allure.step("Setup driver");

				XCUITestOptions xcuiTestOptions = new XCUITestOptions()
						.setAutomationName("XCUITest")
						.setDeviceName(ConfigReader.deviceConfig.deviceName())
						.setPlatformVersion(ConfigReader.deviceConfig.platformVersion())
						.setPlatformName(ConfigReader.deviceConfig.platformName())
						.setApp(ConfigReader.deviceConfig.app())
						.setUdid(ConfigReader.deviceConfig.udid());

				UiAutomator2Options uiAutomator2Options = new UiAutomator2Options()
						.setAutomationName("UiAutomator2")
						.setDeviceName(ConfigReader.deviceConfig.deviceName())
						.setPlatformVersion(ConfigReader.deviceConfig.platformVersion())
						.setPlatformName(ConfigReader.deviceConfig.platformName())
						.setPrintPageSourceOnFindFailure(true)
						.setApp(ConfigReader.deviceConfig.app());

				if (ConfigReader.testConfig.PLATFORM().equals("ANDROID")) {
						driver = new AndroidDriver(new URL(appiumUrl), uiAutomator2Options);
				} else if (ConfigReader.testConfig.PLATFORM().equals("IOS")) {
						driver = new IOSDriver(new URL(appiumUrl), xcuiTestOptions);
				}
		}

		@AfterMethod
		protected void tearDown() throws Exception {
				Allure.step("Quit driver");
				driver.quit();
		}

		public AppiumDriver getDriver() {
				return driver;
		}

		public void setDriver(AppiumDriver driver) {
				this.driver = driver;
		}
}