# Messages

Messages, located in the Messages Inbox, are used to notify users about important information.

Technically, they are implemented as add-ons (see the add-on documentation).

The JSON files are validated via [JSON Schema](https://json-schema.org/) files:
https://github.com/mozilla-mobile/mozilla-vpn-client/tree/main/scripts/ci/jsonSchemas

On this page, we want to describe how to write a message.

## JSON format

The Message JSON files are add-on JSON objects, with type "message" and a "message"
property object with the following properties:

| Property | Description  | Type | Required |
| -- | -- | --| -- |
| id | A unique identifier for the message | String | Yes |
| usesSharedStrings | Whether it uses the translation file shared between all messages (default: false) | Boolean | No |
| shortVersion | A human-readable version number to insert into user-facing strings | Strings | Yes if usesSharedMessageStrings is true, No otherwise |
| title | The subject of the message  | String | Yes |
| subtitle | A brief description of the message | String | No |
| title_comment | An optional comment to describe the meaning of the title | String | No |
| subtitle_comment | An optional comment to describe the meaning of the subtitle | String | No |
| date | The date the message was received (using seconds since epoch time) | Number | No |
| badge | A label used to tag a message (options: `warning`, `critical`, `new_update`, `whats_new`, `survey`, `subscription` ) | String | No |
| notify | Should we notify the user about this message via system notifications (Default: true) | Boolean | No |
| blocks | An array of graphical blocks that compose the user interface of the message's contents (see more info [here](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki/guides#block-object)) | Array of Block objects | Yes |
