package tests;

import org.testng.annotations.Test;
import core.BaseTest;
import pages.*;
import static org.hamcrest.MatcherAssert.*;
import static org.testng.Assert.*;

public class POCTests extends BaseTest {

		@Test(description = "Example of failed test in allure")
		public void failedTest() {
				HomePage homePage = new HomePage(driver);
				SignUpPageFirstStep signUpPageFirstStep = new SignUpPageFirstStep(driver);
				if(homePage.verifySignUpButtonPresent()) {
						homePage.signUpButton.click();
				}
				assertThat("Sign up page is showing when it should not", !signUpPageFirstStep.verifyPageIsOpen() == true);
		}

		@Test(description = "Example of passed test in allure")
		public void passedTest() {
				HomePage homePage = new HomePage(driver);
				SignUpPageFirstStep signUpPageFirstStep = new SignUpPageFirstStep(driver);
				if(homePage.verifySignUpButtonPresent()) {
						homePage.signUpButton.click();
				}
				assertThat("Sign up page is not showing when it should", signUpPageFirstStep.verifyPageIsOpen() == true);
		}
}