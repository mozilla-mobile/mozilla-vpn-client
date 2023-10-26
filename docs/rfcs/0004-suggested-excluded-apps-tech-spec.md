#Suggesting Apps for Split Tunneling
===================================

Epic: [VPN-4412: Suggest Apps to Exclude from VPN Protection](https://mozilla-hub.atlassian.net/browse/VPN-4412)

Context / Problem Statement
---------------------------

We're looking to create tools that prompt the user in ways that help them stay more secure. Split tunneling (called 'App Exclusion' to users) is a feature that can reduce the need to turn off the VPN entirely by excluding traffic in specific apps from the VPN tunnel, but many users don't know about it. The current feature is in settings, and provides an alphabetical checklist of installed apps:

<img src="./images/0004-01.png" height=500>

This epic is for a new feature to proactively let users know when they have apps that could benefit from the split tunneling feature.

Assumptions & Out-of-scope
--------------------------

-   This is only for platforms that currently support split tunnel - Windows, Linux, and Android. We will not build split tunneling into macOS or iOS as part of this work.
-   Notifications refer to system-level notifications. There are no additions to the messages tab considered in this tech spec.

Open Questions
--------------

-   What do we use to canonically track apps we may want to exclude? Is there a bundle ID? The name of app (fragile)? Something else? Do we use different IDs across platforms?
-   How hard would these checks be to run in Android daemon or PeriodicWorkRequest? (If substantially harder, Santiago open to doing it in Android app - which reduces how often it runs, as the app must be open in order for it to run.) On desktop platforms, the app is always active when the VPN is active. This is not true on mobile, which is why this is considered separately.
-   Any easy ways to categorize games? (See [VPN-4433](https://mozilla-hub.atlassian.net/browse/VPN-4433).)
-   How will we decide which apps are initially on the suggested list?
-   Does the suggested list just include specific apps, or can it include entire directories (like well-known Windows game directories) and/or allow wildcard matching?

Implementation (estimated 39-64 points total)
---------------------------------------------------

This work is broken into four sections, with the bulk of code coming in the middle two sections. Those two sections should each be built behind their own separate feature flag, though we'd expose both parts together via Nimbus.

### Prerequisites (21-36 points total)

-   (3-5 points) Answer the open questions above
-   (5-8 points) Create a well-structured file (for our own research, as well as one users can add to via GH pull requests on the client repo) w/ apps to exclude (and update the repo's readme to explain this new type of contribution). Add this doc to addons. Add tests to ensure the doc's format integrity is maintained. This format should be designed to allow new platforms (like websites) to be added without breaking anything.
    -   Current proposal for format: Individual text files for each platform (`android.txt`, `linux.txt`, etc.) in a specific directory. Each line of the file will be a single path, or bundle ID, or whatever we end up using as an identifier. (See "Other options considered" section for why this is the current proposal.)
    -   Additions to the list should be given the same amount of thought that adding new dependencies to our repo are given. That is, full research by our engineers to ensure nothing malicious is being snuck in. And erring on the side of not including any.
        -   This statement (or something similar) should be added to the top of the file in a comment.
    -   From Santiago: <https://github.com/citizenlab/test-lists/tree/master/lists>, <https://ooni.org/>
    -   Santiago list for Android: <https://docs.google.com/spreadsheets/d/1j2REPQkcdPg5bNW5B3T-L8dnlNPPwAms1WlBiYvVwoc/edit#gid=0>
-   (8-13 points) Deep linking into app for notification to work ([VPN-5034](https://mozilla-hub.atlassian.net/browse/VPN-5034)): Currently, our navigation code allows us to switch tabs, but does not permit moving to a deeper screen - so we can move a user to the settings screen, but can't move them into a menu or submenu within the screen (like App Exclusions). We need deep linking to be able to open the notification where we want.
-   (2-5 points) Create initial list of recommended app exclusions
-   (3-5 points) Create a task for checking for getting suggested app exclusions, ensure this is well tested
    -   The list of suggested app exclusions will live in the VPN client repo, and be served via addons.


### Create daily task in app (8-12 points total)
<img src="./images/0004-02.png" width=800 alt="New daily task">

-   (3-5 points) Daily task to check for suggested apps that are installed on the device (after one week from first install), halt task if don't have notification permissions or if the platform doesn't have app exclusions feature. (Create dev option to immediately pull down the new list.)
    -   This will be run by the app for desktop, and a [PeriodicWorkRequest](https://developer.android.com/reference/androidx/work/PeriodicWorkRequest) on Android. (iOS does not have split tunneling.)
        -   After running the job, we will create a 24 hour countdown timer to run the job next. Additionally, we'll record the time for this job as a setting, and we'll check this timestamp on app/daemon launch - if more than 24 hours have passed we'll
    -   Create a dev option to pull a new suggested apps list immediately.
-   (3-5 points) Is there a new app?
    -   Check against setting - if we have ever sent a notification for this app before, don't send one (this should be done off the app's domain name or something, not marketing name - that can change)
    -   Pop an OS-level notification (on Android, use a suggestion channel)
    -   Add to "have sent notification" list
    -   (Create a dev option to clear the "have sent notification" list.)
-   (2 points) Ensure tapping notification leads to correct screen

### Improve existing split tunnel screen (10-16 points total)
[Figma mocks](https://www.figma.com/file/UZYzma7hlcfE5ke3z8jGbN/App-exclusions-suggestions?type=design&node-id=196-6366&mode=design&t=RL1hdfBQLMS1rKVa-0 "https://www.figma.com/file/UZYzma7hlcfE5ke3z8jGbN/App-exclusions-suggestions?type=design&node-id=196-6366&mode=design&t=RL1hdfBQLMS1rKVa-0")

<img src="./images/0004-03.png" width=500 alt="Updated flow for existing App Exclusion screen">

-   (2 points) Run the task on the launch of screen
    -   Update the "have seen these apps" settings list with all suggested apps that are on the system. (This means that if a new app is seen on the Settings screen before the daily job runs and we send a notification, we never send a notification. This was confirmed by Santiago.)
-   (1-2 points) Add recommended section (only if 1 more more apps on the system are suggested) - pull any apps in the "have seen them" list
-   (1 point) Sort recommended section
-   (1 point) Sort unrecommended section
-   (1-2 points) Update sort when something is added or removed
-   (1-2 points) Ensure search works, shows one section, is alphabetically sorted
-   (2-5 points) Ensure accessibility is up to date
-   (1 point) Add metrics for how many suggested (and non-suggested) apps are excluded\

### Day 2

These are important components that we are committed to delivering, but are outside the scope of this epic. They are items that will be done as soon as the epic is closed, but do not block the feature on shipping.

-   Engineering metrics
    -   Metrics will be created for a successful deep linking, as well as an erroneous deep linking (the app cannot figure out how to resolve the deep link).
    -   Add performance metrics around load time of the Excluded Apps screen.
    -   Create a metric for when the daily job was successful, and another one for when it fails.
    -   A metric should be recorded when the notification is sent

Other options considered
------------------------

Design and research went through several iterations, so the [designs](https://www.figma.com/file/UZYzma7hlcfE5ke3z8jGbN/App-exclusions-suggestions?type=design&node-id=196-6366&mode=design&t=RL1hdfBQLMS1rKVa-0 "https://www.figma.com/file/UZYzma7hlcfE5ke3z8jGbN/App-exclusions-suggestions?type=design&node-id=196-6366&mode=design&t=RL1hdfBQLMS1rKVa-0") were set before this tech spec was written. (See [completed tickets in the epic](https://mozilla-hub.atlassian.net/browse/VPN-4412 "https://mozilla-hub.atlassian.net/browse/VPN-4412") for more background.)

Other options considered included a fixed list of apps to suggest, which could only be updated when the app updates.

We considered hosting the list in Guardian (which is a private repo, so the list would need to be hosted elsewhere to allow public suggestions) and in the client repo outside of addons (requiring a new app release to update the list).

### Other options considered for suggested app files

The goals for the file format:
-   Easy to read by humans
-   Easy for humans to add to, no matter their level of technical proficiency
-   Easy for us to parse in the app

Initially, there were two additional goals (no longer considerations):
-   To have one file for all platforms, so that we'd remember to add a specific app to all platforms at once. With a conversation around potentially wanting different suggested app categories on mobile, this seems unimportant.
-   Allowing comments in the file. While we still could structure a text file to allow comments, this became less important when the plan became "one file per platform, one app per line".

Other options considered included CSV, YAML, and JSON. While YAML could be useful (as it's easy for less-technical people to read), we don't have a good YAML parser in the app yet. We could add a library, of course. However, we'd need to write our own document format test, because the YAML file (or JSON or CSV) would need to be structured in a specific way.

Ultimately, the multiple text files seemed simpler (and thus easier to build) - there aren't the same parsing concerns and it's easy for contributors.

Metrics
-------

We will create several metrics as part of this work; they are described in the appropriate spots in the "Implementation" section.

We'd expect the `apps_excluded` metric to increase after this is implemented on platforms which have app exclusions. Both the number of people with more than 0 apps excluded and the average count of apps excluded should increase.

If Nimbus is available by the time this feature is complete, we will use it to roll out the test.

Security / Privacy
------------------

Since all the work is being done on device, the only security concerns are ensuring we don't log any private info.

The excluded apps list will likely live in the Guardian repo. There are ways to weaponize the suggested apps list to weaken privacy of key apps, but for this attack vector to be used a Mozilla staffer would need to approve the PR with the bad-faith addition to the suggested app list.

Rollout considerations
----------------------

While the reworked app exclusion page will affect all platforms that support split tunneling, notifications will only be shown to users who have already allowed our app to send them. On mobile, we show the "approve notifications" screen immediately upon launching the app for the first time, which likely depresses approval rates. We may want to re-consider how we show this as part of onboarding.

Test Plan
---------

-   Fresh install of the app on a device that has a suggested app. Ensure that app is shown appropriately on the screen in VPN Settings. Ensure a notification suggesting its exclusion is popped at an appropriate time.
-   Install an app on the suggested list on the device. Ensure the app is now shown appropriately on the screen in VPN settings. Ensure a notification suggesting its exclusion is popped at an appropriate time.
-   Add an app on the device to the list on the repo, wait, and ensure it is offered as an app to exclude within 24 hours.
-   Delete an app on the list from the device, ensure it is no longer shown on the screen in VPN settings.
-   Test on all devices with split tunnelling, to ensure the app list works on all platforms
