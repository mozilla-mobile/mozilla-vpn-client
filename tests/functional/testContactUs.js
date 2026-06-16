/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const queries = require('./queries.js');
const {validators} = require('./servers/guardian_endpoints.js');


describe('Contact us view', function() {
  describe('Contact us view - unauthenticated user', function() {
    this.ctx.authenticationNeeded = false;

    const ticket = {
      email: null,
      logs: null,
      versionString: null,
      platformVersion: null,
      subject: null,
      issueText: null,
      category: null
    };

    this.ctx.guardianOverrideEndpoints = {
      POSTs: {
        '/api/v1/vpn/createGuestSupportTicket': {
          status: 201,
          callback: (req) => {
            ticket.email = req.body.email;
            ticket.logs = req.body.logs;
            ticket.subject = req.body.subject;
            ticket.issueText = req.body.issueText;
            ticket.category = req.body.category;
          },
          body: {},
        },
      }
    };

    it('Unauthenticated user can create support ticket', async () => {
      await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
      await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);
      await vpn.waitForQuery(queries.screenGetHelp.contactSupportView
                                 .UNAUTH_USER_INPUTS.visible());
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.EMAIL_INPUT, 'text',
          'test@test.com');
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.DROPDOWN.visible());
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.DROPDOWN, 'currentIndex', 1);
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.TEXTAREA, 'text',
          'user issue description');
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.SUBJECT_INPUT, 'text',
          'user issue subject');
      await vpn.wait(200);
      await vpn.scrollToQuery(
          queries.screenGetHelp.contactSupportView.VIEW,
          queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON);
      await vpn.waitForQueryAndClick(
          queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.enabled());
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.THANK_YOU_PANEL.visible());
      await vpn.waitForQueryAndClick(
          queries.screenGetHelp.contactSupportView.DONE_BUTTON);
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());

      assert.equal(ticket.category, 'account');
      assert.equal(ticket.issueText, 'user issue description');
      assert.equal(ticket.subject, 'user issue subject');
      assert.equal(ticket.email, 'test@test.com');
      assert.notEqual(ticket.logs, 'Logs not shared.');
    });

    it('VPNUserInfo is not visible to unathenticated users', async () => {
      await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
      await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

      await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);

      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.USER_INFO.hidden());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenGetHelp.contactSupportView.USER_INFO, 'visible'),
          'false');
    });
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    const ticket = {
      email: null,
      logs: null,
      versionString: null,
      platformVersion: null,
      subject: null,
      issueText: null,
      category: null
    };

    this.ctx.guardianOverrideEndpoints = {
      POSTs: {
        '/api/v1/vpn/createSupportTicket': {
          status: 201,
          requiredHeaders: ['Authorization'],
          callback: (req) => {
            ticket.logs = req.body.logs;
            ticket.subject = req.body.subject;
            ticket.issueText = req.body.issueText;
            ticket.category = req.body.category;
          },
          body: {},
        },
      }
    };

    async function openContactUsInSettings() {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS);

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);
    }

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUsInSettings();

      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.USER_INFO.visible());
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUsInSettings();

      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.USER_INFO.visible()
              .disabled());
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUsInSettings();

        await vpn.waitForQuery(queries.screenGetHelp.contactSupportView
                                   .UNAUTH_USER_INPUTS.hidden());
      });
    });

    it('Check share logs checkbox differently by category', async () => {
      await openContactUsInSettings();
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.DROPDOWN.visible());
      const categoryShareLogsMap = Object.entries({
        'payment': false,
        'account': true,
        'technical': true,
        'feature': false,
        'other': false
      });

      for (const [index, [category, shouldShareLogs]] of Object.entries(
               categoryShareLogsMap)) {
        await vpn.setQueryProperty(
            queries.screenGetHelp.contactSupportView.DROPDOWN, 'currentIndex',
            index);
        assert.equal(
            await vpn.query(queries.screenGetHelp.contactSupportView
                                .SHARE_LOGS_CHECKBOX.prop('isChecked', true)),
            shouldShareLogs);
        assert.equal(
            await vpn.getQueryProperty(
                queries.screenGetHelp.contactSupportView.DROPDOWN,
                'currentValue'),
            category);
      }
    });

    it('Create support ticket with logs', async () => {
      await openContactUsInSettings();
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.DROPDOWN.visible());
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.DROPDOWN, 'currentIndex', 2);
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.TEXTAREA, 'text',
          'user issue description');
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.SUBJECT_INPUT, 'text',
          'user issue subject');
      // Checkbox should be checked by default
      assert.equal(
          await vpn.query(
              queries.screenGetHelp.contactSupportView.SHARE_LOGS_CHECKBOX.prop(
                  'isChecked', true)),
          true);
      await vpn.wait(200);
      // We need to scroll to be able to click the submit button
      await vpn.scrollToQuery(
          queries.screenGetHelp.contactSupportView.VIEW,
          queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON);
      await vpn.waitForQueryAndClick(
          queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.enabled());
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.THANK_YOU_PANEL.visible());
      await vpn.waitForQueryAndClick(
          queries.screenGetHelp.contactSupportView.DONE_BUTTON);
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());

      assert.equal(ticket.category, 'technical');
      assert.equal(ticket.issueText, 'user issue description');
      assert.equal(ticket.subject, 'user issue subject');
      assert.notEqual(ticket.logs, 'Logs not shared.');
    });

    it('Create support ticket without logs', async () => {
      await openContactUsInSettings();
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.DROPDOWN.visible());
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.DROPDOWN, 'currentIndex', 1);
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.TEXTAREA, 'text',
          'user issue description');
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.SUBJECT_INPUT, 'text',
          'user issue subject');
      // Ensure the checkbox is checked and can be unchecked
      assert.equal(
          await vpn.query(
              queries.screenGetHelp.contactSupportView.SHARE_LOGS_CHECKBOX.prop(
                  'isChecked', true)),
          true);
      await vpn.setQueryProperty(
          queries.screenGetHelp.contactSupportView.SHARE_LOGS_CHECKBOX,
          'isChecked', false);
      await vpn.wait(200);
      await vpn.scrollToQuery(
          queries.screenGetHelp.contactSupportView.VIEW,
          queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON);
      await vpn.waitForQueryAndClick(
          queries.screenGetHelp.contactSupportView.SUBMIT_BUTTON.enabled());
      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.THANK_YOU_PANEL.visible());
      await vpn.waitForQueryAndClick(
          queries.screenGetHelp.contactSupportView.DONE_BUTTON);
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());

      assert.equal(ticket.category, 'account');
      assert.equal(ticket.issueText, 'user issue description');
      assert.equal(ticket.subject, 'user issue subject');
      assert.equal(ticket.logs, 'Logs not shared.');
    });
  });
});
