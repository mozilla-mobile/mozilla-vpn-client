const assert = require('chai').assert;

describe('Android Sample tests', () => {
    it('find element by xpath', async () => {
        // find element        
        const getHelpBtn = await $('~Get help ')
        const contactSupportBtn = await $('~Contact support ')
        const contactSupportTitle = await $('//android.view.View.VirtualChild[@content-desc="Contact support "]')

        // click on get help
        await getHelpBtn.waitForDisplayed({ timeout: 30_000 })
        await getHelpBtn.click()

        // click on contact support
        await contactSupportBtn.waitForDisplayed({ timeout: 30_000 })
        await contactSupportBtn.click()
        await new Promise(resolve => setTimeout(resolve, 2_000));

        // verify 
        assert.notEqual(contactSupportTitle, null)
    });
});