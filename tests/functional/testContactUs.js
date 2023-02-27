/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const queries = require('./queries.js');
const guardianEndpoints = require("./servers/guardian_endpoints.js");

async function openContactUsInSettings(isAuthenticated) {
  if (!isAuthenticated) {
    await vpn.waitForQueryAndClick(
      queries.screenInitialize.GET_HELP_LINK.visible()
    );
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
  } else {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());
  }

  await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
  await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT.visible());
  await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
}

describe("Contact us view -  unauthenticated user", function () {
  this.ctx.guardianOverrideEndpoints = {
    GETs: {},
    POSTs: {
      "/api/v1/vpn/createGuestSupportTicket": {
        status: 201,
        bodyValidator: guardianEndpoints.validators.supportTicketData,
        body: null,
      },
    },
    DELETEs: {},
  };

  it("VPNUserInfo is not visible to unathenticated users", async () => {
    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.USER_INFO.hidden()
    );
    assert.equal(
      await vpn.getQueryProperty(
        queries.screenGetHelp.contactSupportView.USER_INFO,
        "visible"
      ),
      "false"
    );
  });

  it("Email inputs are visible to unauthenticated user", async () => {
    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.UNAUTH_USER_INPUT.visible()
    );
  });

  it("Unauthenticated User is able to fill out form", async () => {
    console.log(
      "*****************Unauthenticated User is able to fill out form********************"
    );
    await openContactUsInSettings(false);

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.EMAIL_INPUT.visible(),
      "test@test.com"
    );
    await vpn.wait(1000);
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.CONFIRM_EMAIL_INPUTS.visible(),
      "test@test.com"
    );

    // open combo box
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    await vpn.wait();
    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    await Promise.all([
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-0-text", "text"),
        "Payment and billing"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-1-text", "text"),
        "Account issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-2-text", "text"),
        "Technical issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-3-text", "text"),
        "Request features"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-4-text", "text"),
        "Other"
      ),
    ]);

    await vpn.waitForQueryAndClick("//category-Dropdown-Option-4-text");
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      "automated subject"
    );

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      "automated describe area"
    );

    await vpn.waitForQueryAndClick(queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible());
  });
});

describe("Unaunthenticated failures", function () {
  this.ctx.guardianOverrideEndpoints = {
    GETs: {},
    POSTs: {
      "/api/v1/vpn/createGuestSupportTicket": {
        status: 400,
        body: {},
      },
    },
    DELETEs: {},
  };

  it("failures, Unauthenticated User is able to fill out form", async () => {
    console.log(
      "*****************Failed Unauthenticated User is able to fill out form********************"
    );
    await openContactUsInSettings(false);

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.EMAIL_INPUT.visible(),
      "test@test.com"
    );
    await vpn.wait(1000);
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.CONFIRM_EMAIL_INPUTS.visible(),
      "test@test.com"
    );

    // open combo box
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    await vpn.wait();
    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );

    await Promise.all([
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-0-text", "text"),
        "Payment and billing"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-1-text", "text"),
        "Account issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-2-text", "text"),
        "Technical issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-3-text", "text"),
        "Request features"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-4-text", "text"),
        "Other"
      ),
    ]);

    await vpn.wait(1000);
    await vpn.waitForQueryAndClick("//category-Dropdown-Option-4-text");

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      "automated subject"
    );

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      "automated describe area"
    );

    await vpn.waitForQueryAndClick(queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible());
  });
});

describe("Contact us view - authenticated user", function () {
  this.ctx.authenticationNeeded = true;
  this.ctx.guardianOverrideEndpoints = {
    GETs: {},
    POSTs: {
      "/api/v1/vpn/createSupportTicket": {
        status: 201,
        requiredHeaders: ["Authorization"],
        bodyValidator: guardianEndpoints.validators.supportTicketData,
        body: null,
      },
    },
    DELETEs: {},
  };

  it("VPNUserInfo visible to authenticated users", async () => {
    await openContactUsInSettings();

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.USER_INFO.visible()
    );
  });

  it("VPNUserInfo is disabled", async () => {
    await openContactUsInSettings();

    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.USER_INFO.visible().disabled()
    );
  });

  it("authenticated User is able to fill out form", async () => {
    console.log(
      "*****************authenticated User is able to fill out form********************"
    );
    await openContactUsInSettings(true);

    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );
    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );
    await vpn.wait();
    assert.equal(
      await vpn.getQueryProperty("//category-Dropdown-Option-0-text", "text"),
      "Payment and billing"
    ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-1-text", "text"),
        "Account issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-2-text", "text"),
        "Technical issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-3-text", "text"),
        "Request features"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-4-text", "text"),
        "Other"
      ),
      await vpn.waitForQueryAndClick("//category-Dropdown-Option-4-text");
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      "automated subject"
    );
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      "automated describe area"
    );
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible()
    );
  });

  it("Email inputs are not visible to authenticated user", async () => {
    it("Email inputs are visible to unauthenticated user", async () => {
      await openContactUsInSettings();

      await vpn.waitForQuery(
        queries.screenGetHelp.contactSupportView.UNAUTH_USER_INPUT.hidden()
      );
    });
  });
});

describe("Authenticated failures", function () {
  this.ctx.authenticationNeeded = true;
  this.ctx.guardianOverrideEndpoints = {
    GETs: {},
    POSTs: {
      "/api/v1/vpn/createSupportTicket": {
        status: 400,
        body: {},
      },
    },
    DELETEs: {},
  };

  it("failures, authenticated User is able to fill out form", async () => {
    console.log(
      "*****************failures, authenticated User is able to fill out form********************"
    );
    await openContactUsInSettings(true);

    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );
    await vpn.waitForQuery(
      queries.screenGetHelp.contactSupportView.CATEGORY_DROPDOWN.visible()
    );
    await vpn.wait();
    assert.equal(
      await vpn.getQueryProperty("//category-Dropdown-Option-0-text", "text"),
      "Payment and billing"
    ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-1-text", "text"),
        "Account issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-2-text", "text"),
        "Technical issues"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-3-text", "text"),
        "Request features"
      ),
      assert.equal(
        await vpn.getQueryProperty("//category-Dropdown-Option-4-text", "text"),
        "Other"
      ),
      await vpn.waitForQueryAndClick("//category-Dropdown-Option-4-text");
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.SUBJECT_INPUT.visible(),
      "automated subject"
    );
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenGetHelp.contactSupportView.DESCRIBE_TEXTAREA.visible(),
      "automated describe area"
    );
    await vpn.waitForQueryAndClick(
      queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.visible()
    );
  });
});