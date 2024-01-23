- Status:
- Date: 2023-10-16
- Author: [@brizental](https://github.com/brizental)
- RFC PR: [#8297](https://github.com/mozilla-mobile/mozilla-vpn-client/pull/8297)
- Implementation GitHub issue: TODO

Mozilla VPN and Nimbus Integration
==================================

TIMELINE
========

2023-09-18: Initial draft

2023-09-21: Feedback Requested on GDocs (accepting feedback until October 13th)

2023-10-16: Feedback Request on Github

2023-12-01: Approved

> **Note**: This RFC was first shared on Google Docs. See: https://docs.google.com/document/d/1TqAlbwuNCx4fXgZvvQhmg6nBKBOPfFC2iYU1AfGYjTI/edit

SUMMARY
-------

This document details the integration of Nimbus on the Mozilla VPN.

As described on[  "Nimbus for Mozilla VPN"](https://docs.google.com/document/d/12oRkct-l-n9Sq8fzRH1YiiA8CtbEqDxASSpVqC2AKI4/edit#heading=h.lo57q9b2r8iy) Nimbus integration on the Mozilla VPN will be done through a server side integration with[  Cirrus](https://github.com/mozilla/experimenter/tree/main/cirrus).

Nimbus
------

Nimbus is the experimentation platform built by Mozilla and used by Mozilla products.

Through a web application called "Experimenter", experiments are defined. Through the Nimbus SDKs or Cirrus, applications query Nimbus for the active experiment branches, called "features" on the Nimbus ecosystem. Nimbus takes care of all the experiment targeting logic to serve the application with the correct features active for each given application instance.

See more on[  https://experimenter.info](https://experimenter.info).

Cirrus
------

Cirrus is a new solution for applications to interact with Nimbus. Different from the Nimbus SDKs, Cirrus is a server side solution.

Cirrus must be deployed alongside the consuming application web server. It is a sidecar service, meaning it extends the functionality of the main project it is attached to. It exposes a single POST endpoint /v1/features for querying Nimbus for the list of active features.

The consuming application can choose how to use the feature information provided by Cirrus.

See more about the Cirrus architecture at[  Nimbus on the Web Architecture Decision](https://docs.google.com/document/d/1ub46GXVz0rD6vsdS85UF_LvUfJItMwE9jicFCXc1jvw/edit).

Implementation plan
-------------------

There are two parts to the implementation of the Nimbus integration on the Mozilla VPN.

Experiments will be run on the client side. Therefore, the features need to be accessible from there. But the server is the one that has access to Cirrus, so it is the one responsible for providing the APIs that allow the client to get the list of active features.

### Server side implementation

Server side will be responsible for hosting the Cirrus service and exposing its functionality to the VPN client.

#### 1\. Register the Cirrus application id with Glean

Cirrus uses Glean internally. Therefore it requires a Glean application id. The process for requesting an application id can be found on[  Enabling data to be ingested by the data platform](https://mozilla.github.io/glean/book/user/adding-glean-to-your-project/enable-data-ingestion.html). We would want something like "mozillavpn_cirrus".

Documentation of metrics and pings sent by the Cirrus service can be found at https://github.com/mozilla/experimenter/blob/main/cirrus/server/telemetry/docs/metrics.md.

> **Note** Cirrus also  requires a Remote Settings URL.  That will be provided by the Nimbus team and there is no action on our side required.

#### 2\. Deploy Cirrus as a sidecar to Guardian

The Cirrus sidecar service will be deployed alongside the Mozilla VPN Guardian server. Deploying this service is as straightforward as adding a few extra configurations to the Guardian server configuration on the mozilla-services/cloudops-infra repository.

The work done for Firefox Monitor on this can be used as a template.

#### 3\. Share the Nimbus feature manifest with the Cirrus volume

The only Cirrus related configuration file that will remain on the Guardian repository is the "feature manifest". An example of such a file can be found at:[  https://github.com/mozilla/experimenter/blob/main/cirrus/server/feature_manifest/sample.yml](https://github.com/mozilla/experimenter/blob/main/cirrus/server/feature_manifest/sample.yml).

This file then needs to be shared with the Docker volume that contains the Cirrus service. An example of how to share this file with the Cirrus volume can be seen at:[  https://github.com/mozilla/blurts-server/pull/3298](https://github.com/mozilla/blurts-server/pull/3298)

#### 4\. Expose Cirrus' features API to the VPN client

Guardian's /featurelist endpoint will be modified to expose [Cirrus' features API](https://github.com/mozilla/experimenter/tree/main/cirrus#endpoint) to the client, due to the overlapping functionality between it and the Cirrus API.

Currently that is a GET endpoint which serves a static JSON file with a list of features to enable or disable and is periodically queried by the client to keep the feature flags state up to date. The current response format is like so:

```
{
  "featuresOverwrite":  {
    "Feature1":  true
    "Feature2":  false
    "FeatureN":  true
  }
}
```

This endpoint will be changed to only accept POST requests. Requests will be expected to contain the following JSON body, which is compliant with the body of the request that will be sent to Cirrus:

```
{
  "client_id":  "4a1d71ab-29a2-4c5f-9e1d-9d9df2e6e449",
  "context":  {
    "key1":  "value1",
    "key2":  {
      "key2.1":  "value2",
      "key2.2":  "value3"
    }
  }
}
```

> **Note** Throughout this document, client_id will be referred to as "experimenter id" so as not to confuse it with the Glean client_id.

The Cirrus reply will be added to the body of the response object, like so:
```
{
 "featuresOverwrite":  {
    "Feature1":  true
    "Feature2":  false
    "FeatureN":  true
  },
  "experimentalFeatures":  {
    "Feature1":  {
      "Variable1.1":  "valueA",
      "Variable1.2":  "valueB"
    },
    "Feature2":  {
      "Variable2.1":  "valueC",
      "Variable2.2":  "valueD"
    },
    "FeatureN":  {
      "VariableN.1":  "valueX",
      "VariableN.2":  "valueY"
    }
  }
}
```

> **Note** This request does not need to be authenticated. It only needs to be made using HTTPS.

If "featuresOverride" conflicts with "experimentalFeatures", "experimentalFeatures" will always have higher priority.

### Client side implementation

Client side will be responsible for querying the server for active features and applying treatments based on the obtained data.

#### 1\. Generate an experimenter id

In order for Nimbus to run its targeting logic, it requires an experimenter id to be attached to each request.

The experimenter id must be generated per user. It's important that this is user based to ensure users are assigned to the same features in all of their logged in devices. The Mozilla VPN client already has access to an identifier that fits the requirements of the experimenter id: the FxA id. Therefore that id will be used for this purpose.

That poses an issue for non-logged in users. In these cases, a random UUID will be generated for the experimenter id and the id will be switched to the FxA once the user successfully logs in. (See also question 2. on the "Open questions" section of this document).

Nimbus relies on Glean data for experiment analysis. Therefore the generated experimenter id needs to be included in all Glean pings. The Glean team is working on a new API that will facilitate setting this id, see [Bug 1848201](https://bugzilla.mozilla.org/show_bug.cgi?id=1848201). The Mozilla VPN application will rely on this API to provide the experiment id to Glean.

Timing: The experimenter id must be set as soon as possible on the application life cycle, since it is required for all Cirrus requests and should be included in all Glean pings.

#### 2\. Query Nimbus for active features

The /featurelist endpoint is already queried by the application  on initialization and once every hour. Other than these triggers, a feature list request will be made whenever the experiment id is set.

> **Note** The list of active features will be added to each VPN log summary, to aid in debugging branch specific bugs.

#### 3\. Apply features

Feature flags will be used to apply features dynamically. This binds experiments to the Mozilla VPN release cycle, new feature flags cannot be dynamically set on the client  -- even if the experiment treatment is implemented through addons. Experiments may be designed and implemented in Nimbus/Experimenter before a given feature has been released, however users will only actively see treatments being applied once they are on a version of the application that contains the treatment.

Once the feature list is received from Cirrus, it will be parsed and known features will be turned on while unknown features will be discarded.

Since the Cirrus features will be served alongside the list of features on the featurelist endpoint, TaskGetFeatureList will also be reused to apply experiment features. 

Experimental features may have variables associated with them as can be seen by the [Cirrus feature API response](https://github.com/mozilla/experimenter/tree/main/cirrus#output). Each experimental feature will have a settings group associated with it, with the exact same name as the feature. Each variable will have a corresponding setting under the group associated with it, with the exact same name. For example, an experimental feature like so:

```
{

 "featuresOverwrite":  {  },
 "experimentalFeatures":  {
    "onboardingCtaStyleExperiment":  {
      "color":  "#FF0000",
      "fontFamily":  "Proxima Nova"
    },
  }
}
```

Will have a related feature like so:

```
EXPERIMENTAL_FEATURE(
      onboardingCtaStyleExperiment, // Feature ID
      "Experimentations on the onboarding CTA", // Feature name
      QStringList("color", "fontFamily"),  // Feature variables
)
```

The EXPERIMENTAL_FEATURE macro, intentionally doesn't contain the field for declaring if the feature can be flipped on or off. Experimental features must always be "toggleable".

> **Note** Mozilla VPN features may or may not be "toggleable", meaning they may not be allowed to be turned on or off. Experimental features must always be toggleable, otherwise they cannot be applied dynamically. This has consequences for the implementation of the treatment itself. This needs to be considered on a case by case basis.

It also doesn't contain the feature callback field. Experimental features always default to false unless they have been turned on through Cirrus. 

Finally, the experimental feature settings will be accessible from C++ like so:

```
settingsHolder.onboardingCtaStyleExperiment.color()
settingsHolder.onboardingCtaStyleExperiment.fontFamily()
```

Or from QML like so:

```
MZSettings.onboardingCtaStyleExperiment.color
MZSettings.onboardingCtaStyleExperiment.fontFamily
```

Experimental feature settings will only have public getters. They may only be set or removed by the TaskGetFeatureList.

> **Note** In order for this setup to work, experimental features will have to be declared in a separate feature list file e.g. experimentalfeaturelist.h. This will make it possible to import the experimental features only on the settings holder module and create the settings API necessary.

It will be the responsibility of the TaskGetFeatureList at the time of parsing the featurelist response, to set each of these settings to the received value. Turning the feature on will happen only after all settings are set.

Experiment treatments may also be in the form of addons. Addons can be conditioned to feature flags already, therefore nothing is required by the addons infrastructure to attend to this proposal.

#### 4\. EXTRA: Experimentation opt-out

Users will be able to opt-out of telemetry and experimentation independently. These options will be available to users in the Settings section of the VPN application. Design for the user experience of these options is still TBD (See: VPN-5862).

Validation
----------

In order to validate that the Nimbus integration is working as expected and can be used to run real experiments, an A/A experiment with no actual treatment will be executed.

An A/A experiment is a type of experiment, where users are assigned to different branches, but the treatment applied to all branches is exactly the same. In this case, a no-op feature flag will be used.

It is out of scope for this document to detail the design of the A/A experiment itself.

Following is a list of metrics that might be used for validation:

-   Guardian already has mechanisms in place to analyze error replies, so that can be used to validate that the Cirrus proxy endpoint is working as expected.

-   The experimenter id will be included in every Glean ping.

Open questions
--------------

1.  As described in[  EXP-3092](https://mozilla-hub.atlassian.net/browse/EXP-3092) the Nimbus team is working alongside the first adopter of Nimbus + Cirrus -- Firefox Monitor -- and once that integration is complete the team will work on documenting the process, so that other applications can also be integrated. When would Mozilla VPN be cleared to start integrating? Would we need to wait for Monitor integration to be complete and validated?

    1.  [Jared Lockhart](https://github.com/jaredlockhart)  The monitor A/A has now launched! 🎉 We expect to wrap it at the end of October, and barring any significant unforeseen complications, we would expect to then move on to focusing on the VPN integration.  Until then the VPN team can get a head start by beginning Glean integration.  We'll only need a single test event from each app to do a similar A/A validation for VPN so it's not necessary to rigorously instrument all of the VPN applications with Glean to test.

    2.  [Beatriz Rizental](https://github.com/brizental) We are lucky that the VPN is already instrumented with Glean :) And we have more than one event.

2.  How would Nimbus handle the case when users are assigned a random UUID before logging in? Is that the correct approach for such situations? What if the user is in an experiment branch that is conflicting with whatever experiment branch their user id is assigned to? Is this a concern or is it fine to just switch the user to the other branch after login?

    1.  [Jared Lockhart](https://github.com/jaredlockhart) This is still an open question.  A naive strategy is that yes, when a user passes through an auth boundary and their client id changes, their feature configurations will be recomputed with their authenticated id.  I expect we will start there and then evaluate whether a more robust solution is necessary.

3.  If a user opts out of telemetry/Glean, does this automatically opt them out of Experiments as well? How should we handle cases where the user is enrolled in an experiment at the time of opt out?  

    1.  [Beatriz Rizental](https://github.com/brizental)  My hunch here is that users that have telemetry turned off will not participate in experiments. Wouldn't that break the targeting logic for example when it's based on percentage of users? However if a user is already enrolled in some experiment, we will keep them in that branch until the experiment is over. Otherwise turning data collection off would have unexpected behavior in some cases.

    2.  [Jared Lockhart](https://github.com/jaredlockhart)  Our existing process in other applications is that at the time the user opts out of telemetry and/or experimentation (opting out of telemetry also opts out of experiments, opting out of experiments does not opt out of telemetry), they are unenrolled from the experiment. This can not be managed by the Nimbus infrastructure, it must be implemented by the integration application.  Ie there must be some UI that allows a user to opt out of telemetry and/or experimentation, the application must store that preference, and any application that is opted out must no longer send requests to Cirrus.
