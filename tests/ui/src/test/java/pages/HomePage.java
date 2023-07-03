package pages;

import io.appium.java_client.AppiumBy;
import io.appium.java_client.AppiumDriver;
import io.appium.java_client.pagefactory.AndroidBy;
import io.appium.java_client.pagefactory.AndroidFindBy;
import io.qameta.allure.Allure;
import io.qameta.allure.Step;
import org.openqa.selenium.By;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;

public class HomePage extends BasePage {
		public HomePage(AppiumDriver driver) {
				super(driver);
		}

		@AndroidFindBy(xpath = "(//android.view.View.VirtualChild[@content-desc=\"Sign up \"])[2]")
		@FindBy()
		public WebElement signUpButton;

		@Step("Verify home page is open")
		public Boolean verifySignUpButtonPresent() {
				Allure.step("Verify if home page is open");
				return waitForElementPresent(signUpButton, "Element not found", 10) != null;
		}

}