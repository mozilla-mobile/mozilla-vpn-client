package tools;

import io.appium.java_client.AppiumDriver;
import core.*;
import io.qameta.allure.Attachment;
import org.openqa.selenium.OutputType;
import org.openqa.selenium.TakesScreenshot;
import org.testng.ITestContext;
import org.testng.ITestListener;
import org.testng.ITestResult;

public class Listener implements ITestListener {

		@Attachment(value = "Page screenshot", type = "image/png")
		public byte[] saveScreenshotPNG(AppiumDriver driver) {
				return ((TakesScreenshot)driver).getScreenshotAs(OutputType.BYTES);
		}

		@Attachment(value = "{0}", type = "text/plain")
		public static String saveTextLog(String message) {
				return message;
		}

		@Override
		public void onTestStart(ITestResult result) {

		}

		@Override
		public void onTestSuccess(ITestResult result) {

		}

		@Override
		public void onTestFailure(ITestResult result) {
				System.out.println("Test case failed:  " + result.getName());

				Object testClass = result.getInstance();
				AppiumDriver driver = ((BaseTest) testClass).getDriver();

				//saves allure screenshot

				saveScreenshotPNG(driver);

				//saves text log
				saveTextLog(result.getTestName() + " failed and screenshot taken");

		}

		@Override
		public void onTestSkipped(ITestResult result) {

		}

		@Override
		public void onTestFailedButWithinSuccessPercentage(ITestResult result) {

		}

		@Override
		public void onTestFailedWithTimeout(ITestResult result) {

		}

		@Override
		public void onStart(ITestContext context) {

		}

		@Override
		public void onFinish(ITestContext context) {

		}
}