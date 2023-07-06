package pages;

import io.appium.java_client.AppiumDriver;
import io.appium.java_client.pagefactory.AppiumFieldDecorator;
import org.openqa.selenium.Dimension;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.PointerInput;
import org.openqa.selenium.interactions.Sequence;
import org.openqa.selenium.support.PageFactory;
import org.openqa.selenium.support.ui.ExpectedConditions;
import org.openqa.selenium.support.ui.WebDriverWait;

import java.time.Duration;
import java.util.List;

public class BasePage {

		protected AppiumDriver driver;
		protected WebDriverWait wait;

		public BasePage(AppiumDriver driver) {
				this.driver = driver;
				PageFactory.initElements(new AppiumFieldDecorator(driver), this);
		}

		public WebElement waitForElementPresent(WebElement element, String error_message, long timeoutInSeconds) {
				wait = new WebDriverWait(driver, Duration.ofSeconds(timeoutInSeconds));
				wait.withMessage(error_message + "\n");
				return wait.until(
						ExpectedConditions.visibilityOf(element));
		}

//		public WebElement waitForElementPresent(WebElement element, String error_message, long timeoutInSeconds) {
//				return waitForElementPresent(element, error_message, timeoutInSeconds);
//		}

		public WebElement waitForElementAndClick(WebElement element, String error_message, long timeoutInSeconds) {
				waitForElementPresent(element, error_message, timeoutInSeconds);
				element.click();
				return element;
		}

		public WebElement waitForElementAndSendKeys(WebElement element, String value, String error_message, long timeoutInSeconds) {
				waitForElementPresent(element, error_message, timeoutInSeconds);
				element.sendKeys(value);
				return element;
		}

		public boolean waitForElementNotPresent(WebElement element, String error_message, long timeoutInSeconds) {
				wait = new WebDriverWait(driver, Duration.ofSeconds(timeoutInSeconds));
				wait.withMessage(error_message + "\n");
				return wait.until(
						ExpectedConditions.invisibilityOf(element));
		}

		public WebElement waitForElementAndClear(WebElement element, String error_message, long timeoutInSeconds) {
				waitForElementPresent(element, error_message, timeoutInSeconds);
				element.clear();
				return element;
		}

		public void swipeUp(int timeOfSwipeInMillis) {
				Dimension size = driver.manage().window().getSize();
				int x = size.width / 2;
				int start_y = size.height / 2;
				int end_y = (int) (size.height * 0.2);

				PointerInput finger = new PointerInput(PointerInput.Kind.TOUCH, "finger");
				Sequence scroll = new Sequence(finger, 0);

				scroll.addAction(finger.createPointerMove(Duration.ZERO, PointerInput.Origin.viewport(), x, start_y));
				scroll.addAction(finger.createPointerDown(PointerInput.MouseButton.LEFT.asArg()));
				scroll.addAction(finger.createPointerMove(Duration.ofMillis(timeOfSwipeInMillis), PointerInput.Origin.viewport(), x, end_y));
				scroll.addAction(finger.createPointerUp(PointerInput.MouseButton.LEFT.asArg()));

				driver.perform(List.of(scroll));
		}

		public void swipeUpQuick() {
				swipeUp(200);
		}

		public void swipeUpToFindElement(WebElement element, String error_message, int max_swipes) {
				int already_swiped = 0;
				while (!element.isDisplayed()) {
						if (already_swiped > max_swipes) {
								waitForElementPresent(element, "Can't find element by swiping up. \n" + error_message, 0);
								return;
						}
						swipeUpQuick();
						already_swiped++;
				}
		}

		public int getAmountOfElements(List<WebElement> elements) {
				return elements.size();
		}

		public void assertElementNotPresent(List<WebElement> elements, String error_message) {
				int amount_of_elements = getAmountOfElements(elements);
				if (amount_of_elements > 0) {
						String default_message = "An element '" + elements.toString() + "' supposed to be not present";
						throw new AssertionError(default_message + " " + error_message);
				}
		}

		public String waitForElementAndGetAttribute(WebElement element, String attribute, String error_message, long timeoutInSeconds) {
				waitForElementPresent(element, error_message, timeoutInSeconds);
				return element.getAttribute(attribute);
		}
}