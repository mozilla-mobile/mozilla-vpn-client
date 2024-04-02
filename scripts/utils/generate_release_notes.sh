#!/bin/bash

# Check if the required tool 'xmlstarlet' is installed
if ! command -v xmlstarlet &> /dev/null; then
  echo "Error: xmlstarlet is not installed. Please install it first."
  exit 1
fi

# Check for the correct number of command-line arguments
if [ "$#" -lt 2 ]; then
  echo ""
  echo "Usage: $0 major_version.minor_version blockid1 [blockid2 blockid3 ... blockidN]"
  echo "The blocks are the ids of the addon blocks we want to extract strings from"
  echo ""
  exit 1
fi

# Input root directory and output file paths
VERSION="$1"

shift # Shift command-line arguments to skip the VERSION argument
BLOCK_SELECTOR="["
for BLOCK_SUFFIX in $@
do
    BLOCK_SELECTOR+="contains(@id, '$BLOCK_SUFFIX') or"
done
# Remove the last " or"
BLOCK_SELECTOR_LENGTH=${#BLOCK_SELECTOR}
BLOCK_SELECTOR="${BLOCK_SELECTOR:0:$BLOCK_SELECTOR_LENGTH-3}"
BLOCK_SELECTOR+="]"

WORKSPACE_ROOT="$( cd "$(dirname "$0")/../.." ; pwd -P )"
TRANSLATIONS_DIRECTORY="$WORKSPACE_ROOT/3rdparty/i18n"
WHATS_NEW_MESSAGE_FOLDER="addons/message_whats_new_v$VERSION"
XLIFF_FILE_NAME="strings.xliff"

RELEASE_NOTES_FILE="$WORKSPACE_ROOT/release_notes.txt"
rm $RELEASE_NOTES_FILE
touch $RELEASE_NOTES_FILE

ES_FOUND=false

for DIR in $TRANSLATIONS_DIRECTORY/*/
do
    ORIGINAL_LANGUAGE_CODE=$(basename "$DIR")

    XLIFF_FILE="$DIR/$WHATS_NEW_MESSAGE_FOLDER/$XLIFF_FILE_NAME"

    LANGUAGE_CODE=""
    if [ "$ORIGINAL_LANGUAGE_CODE" == "en" ]; then
        LANGUAGE_CODE="en-US"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "de" ]; then
        LANGUAGE_CODE="de-DE"
    # There are multiple spanish variants.
    # Let's grab the first of them and run with it.
    #
    # Note: Yes, there are also variants for English,
    # but we are guaranteed to always have `en` strings,
    # so there is no need to overcomplicate it.
    elif [[ "$ORIGINAL_LANGUAGE_CODE" == "es"* ]]; then
        if [ $ES_FOUND == true ]; then
            continue;
        fi

        LANGUAGE_CODE="es-ES"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "fr" ]; then
        LANGUAGE_CODE="fr-FR"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "it" ]; then
        LANGUAGE_CODE="it-IT"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "nl" ]; then
        LANGUAGE_CODE="nl-NL"
    else
        # Play stores don't ask for the other languages.
        continue;
    fi

    if [ "$LANGUAGE_CODE" == "en-US" ]; then
        TRANSLATED_TEXT=$(xmlstarlet sel \
                -N x="urn:oasis:names:tc:xliff:document:1.2" \
                -t -m "/x:xliff/x:file/x:body/x:trans-unit$BLOCK_SELECTOR" \
                -v "concat(x:source, '\n')" -n \
                "$XLIFF_FILE")
    else
         TRANSLATED_TEXT=$(xmlstarlet sel \
            -N x="urn:oasis:names:tc:xliff:document:1.2" \
            -t -m "/x:xliff/x:file/x:body/x:trans-unit$BLOCK_SELECTOR" \
            -v "concat(x:target, '\n')" -n \
            "$XLIFF_FILE")
    fi

    # Remove the last line break.
    TRANSLATED_TEXT_LENGTH=${#TRANSLATED_TEXT}
    TRANSLATED_TEXT="${TRANSLATED_TEXT:0:$TRANSLATED_TEXT_LENGTH-2}"

    # Check if the output is not empty
    if [ -n "$TRANSLATED_TEXT" ]; then
        if [ "$LANGUAGE_CODE" == "es-ES" ]; then
            ES_FOUND=true
            echo "✅ Release note translations found for: $LANGUAGE_CODE (using $ORIGINAL_LANGUAGE_CODE)"
        else
            echo "✅ Release note translations found for: $LANGUAGE_CODE"
        fi

        echo "<$LANGUAGE_CODE>" >> $RELEASE_NOTES_FILE
        echo -e $TRANSLATED_TEXT >> $RELEASE_NOTES_FILE
        echo "</$LANGUAGE_CODE>" >> $RELEASE_NOTES_FILE
    else
        echo "❌ No release note translations found for: $LANGUAGE_CODE ($ORIGINAL_LANGUAGE_CODE)"
    fi
done

echo ""
echo "Release notes written to: $RELEASE_NOTES_FILE"
