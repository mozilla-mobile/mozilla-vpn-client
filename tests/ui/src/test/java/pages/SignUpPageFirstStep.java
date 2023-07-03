package pages;

import io.appium.java_client.AppiumDriver;
import io.appium.java_client.pagefactory.AndroidFindBy;
import io.appium.java_client.pagefactory.iOSXCUITFindBy;
import io.qameta.allure.Allure;
import io.qameta.allure.Step;
import org.openqa.selenium.WebElement;
import org.testng.log4testng.Logger;

public class SignUpPageFirstStep extends BasePage {
		public SignUpPageFirstStep(AppiumDriver driver) {
				super(driver);
		}

		@AndroidFindBy(xpath = "//android.view.View.VirtualChild[@content-desc=\"Enter your email address to continue using Mozilla VPN \"]")
		public WebElement pageFlag;

		@AndroidFindBy(xpath = "/hierarchy/android.widget.FrameLayout/android.view.ViewGroup/android.widget.FrameLayout/android.view.ViewGroup/android.view.View[3]")
		WebElement emailField;

		@AndroidFindBy(xpath = "(//android.view.View.VirtualChild[@content-desc=\"Continue \"])[2]")
		@iOSXCUITFindBy(xpath = "test")
		WebElement continueButton;

		@Step("Verify sign up page is open")
		public boolean verifyPageIsOpen() {
				Allure.step("Verify if sign up page is open");
				return waitForElementPresent(pageFlag, "Sign up page didn't open", 10) != null;
		}

		@Step
		public SignUpPageFirstStep fillEmailField() {
				Allure.step("Fill email field");
				emailField.clear();
				emailField.sendKeys("testEmail1@mozilla.com");
				return this;
		}

		@Step("Tap on Continue button")
		public SignUpPageFirstStep tapOnContinueButton() {
				continueButton.click();
				return this;
		}
}