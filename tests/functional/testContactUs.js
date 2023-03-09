/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const queries = require('./queries.js');

async function openContactUsInSettings(isAuthenticated) {
  if (!isAuthenticated) {
    await vpn.waitForQueryAndClick(
      queries.screenInitialize.GET_HELP_LINK.visible()
    );
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
  } else {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    // wait for element to be interactable
    await vpn.wait();
    await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());
  }

  await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
  // wait for element to be interactable
  await vpn.wait();
  await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT.visible());
  await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
}

async function getDropDownTextString(index){
  return await vpn.getQueryProperty(`//category-Dropdown-Option-${index}-text`, 'text')
}

describe('Contact us view -  unauthenticated user', function () {
  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT.visible());

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.USER_INFO.hidden()
    );
    assert.equal(
      await vpn.getQueryProperty(
        queries.screenGetHelp.contactSupportView.USER_INFO,
        'visible'
      ),
      'false'
    );
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT.visible());

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.UNAUTH_USER_INPUT.visible()
    );
  });

  it.only('Unauthenticated User is able to fill out form', async () => {
    await openContactUsInSettings(false);

    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.EMAIL_INPUT.visible(),
      'test@test.com'
    );

    // wait for element to be interactable
    await vpn.wait();
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.CONFIRM_EMAIL_INPUTS.visible(),
      'test@test.com'
    );

    // open combo box
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    // wait for element to be interactable
    await vpn.wait();
    await Promise.all([
      assert.equal(
        await getDropDownTextString(0),
        'Payment and billing'
      ),
      assert.equal(
        await getDropDownTextString(1),
        'Account issues'
      ),
      assert.equal(
        await getDropDownTextString(2),
        'Technical issues'
      ),
      assert.equal(
        await getDropDownTextString(3),
        'Request features'
      ),
      assert.equal(
        await getDropDownTextString(4),
        'Other'
      ),
    ]);

    await vpn.waitForQueryAndClick('//category-Dropdown-Option-4-text');
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      'automated subject'
    );

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      'automated describe area'
    );

    await vpn.waitForQueryAndClick(queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible());
  });
});

describe('Unaunthenticated failures', function () {
  it('failures, Unauthenticated User is able to fill out form', async () => {
    await openContactUsInSettings(false);

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.EMAIL_INPUT.visible(),
      'test@test.com'
    );
    await vpn.wait();
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.CONFIRM_EMAIL_INPUTS.visible(),
      'test@test.com'
    );

    // open combo box
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    // wait for element to be interactable
    await vpn.wait();
    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    await Promise.all([
      assert.equal(
        await getDropDownTextString(0),
        'Payment and billing'
      ),
      assert.equal(
        await getDropDownTextString(1),
        'Account issues'
      ),
      assert.equal(
        await getDropDownTextString(2),
        'Technical issues'
      ),
      assert.equal(
        await getDropDownTextString(3),
        'Request features'
      ),
      assert.equal(
        await getDropDownTextString(4),
        'Other'
      ),
    ]);

    // wait for element to be interactable
    await vpn.wait();
    await vpn.waitForQueryAndClick('//category-Dropdown-Option-4-text');

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      'automated subject'
    );

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      'automated describe area'
    );

    await vpn.waitForQueryAndClick(queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible());
  });
});

describe('Contact us view - authenticated user', function () {
  this.ctx.authenticationNeeded = true;

  it('VPNUserInfo visible to authenticated users', async () => {
    await openContactUsInSettings(true);

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.USER_INFO.visible()
    );
  });

  it('VPNUserInfo is disabled', async () => {
    await openContactUsInSettings(true);

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.USER_INFO.visible().disabled()
    );
  });

  it('authenticated User is able to fill out form', async () => {
    await openContactUsInSettings(true);

    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    // wait for element to be interactable
    await vpn.wait();

    await Promise.all([
      assert.equal(
        await getDropDownTextString(0),
        'Payment and billing'
      ),
      assert.equal(
        await getDropDownTextString(1),
        'Account issues'
      ),
      assert.equal(
        await getDropDownTextString(2),
        'Technical issues'
      ),
      assert.equal(
        await getDropDownTextString(3),
        'Request features'
      ),
      assert.equal(
        await getDropDownTextString(4),
        'Other'
      ),
    ]);


    await vpn.waitForQueryAndClick('//category-Dropdown-Option-4-text');
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      'automated subject'
    );
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      'automated describe area'
    );
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible()
    );
  });

  it('Email inputs are not visible to authenticated user', async () => {
    it('Email inputs are visible to unauthenticated user', async () => {
      await openContactUsInSettings(true);

      await vpn.waitForQuery(
        queries.screenGetHelp.contactSupportView.UNAUTH_USER_INPUT.hidden()
      );
    });
  });
});

describe('Authenticated failures', function () {
  this.ctx.authenticationNeeded = true;

  it('failures, authenticated User is able to fill out form', async () => {
    await openContactUsInSettings(true);

    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );
    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    // wait for element to be interactable
    await vpn.wait();
    await Promise.all([
      assert.equal(
        await getDropDownTextString(0),
        'Payment and billing'
      ),
      assert.equal(
        await getDropDownTextString(1),
        'Account issues'
      ),
      assert.equal(
        await getDropDownTextString(2),
        'Technical issues'
      ),
      assert.equal(
        await getDropDownTextString(3),
        'Request features'
      ),
      assert.equal(
        await getDropDownTextString(4),
        'Other'
      ),
    ]);

    await vpn.waitForQueryAndClick('//category-Dropdown-Option-4-text');
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      'automated subject'
    );
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      'automated describe area'
    );
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible()
    );
  });
});