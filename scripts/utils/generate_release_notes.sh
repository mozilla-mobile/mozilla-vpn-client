#!/bin/bash

# Check if the required tool 'xmlstarlet' is installed
if ! command -v xmlstarlet &> /dev/null; then
  echo "Error: xmlstarlet is not installed. Please install it first."
  exit 1
fi

# Check for the correct number of command-line arguments
if [ "$#" -lt 2 ]; then
  echo ""
  echo "Usage: $0 stringId1 [stringId2 stringId3 ... stringIdN]"
  echo "The blocks are the ids of the addon blocks we want to extract strings from."
  echo "These blocks are from strings.yaml."
  echo "Example usage: $0 vpn.commonStrings.generalUpdateBulletIntro vpn.225updateMessage.bullet2"
  echo ""
  exit 1
fi

BLOCK_IDS=()
for BLOCK_SUFFIX in $@
do
    BLOCK_IDS+=("contains(@id, '$BLOCK_SUFFIX')")
done

WORKSPACE_ROOT="$( cd "$(dirname "$0")/../.." ; pwd -P )"
TRANSLATIONS_DIRECTORY="$WORKSPACE_ROOT/3rdparty/i18n"
XLIFF_FILE_NAME="strings.xliff"

RELEASE_NOTES_FILE="$WORKSPACE_ROOT/release_notes.txt"
rm $RELEASE_NOTES_FILE
touch $RELEASE_NOTES_FILE

ES_FOUND=false

for DIR in $TRANSLATIONS_DIRECTORY/*/
do
    ORIGINAL_LANGUAGE_CODE=$(basename "$DIR")

    XLIFF_FILE="$DIR/addons/$XLIFF_FILE_NAME"

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
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "cs" ]; then
        LANGUAGE_CODE="cs-CZ"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "da" ]; then
        LANGUAGE_CODE="da-DK"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "el" ]; then
        LANGUAGE_CODE="el-GR"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "fi" ]; then
        LANGUAGE_CODE="fi-FI"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "hu" ]; then
        LANGUAGE_CODE="hu-HU"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "id" ]; then
        LANGUAGE_CODE="id"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "ko" ]; then
        LANGUAGE_CODE="ko-KR"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "pt_BR" ]; then
        LANGUAGE_CODE="pt-BR"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "pt_PT" ]; then
        LANGUAGE_CODE="pt-PT"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "sk" ]; then
        LANGUAGE_CODE="sk"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "sl" ]; then
        LANGUAGE_CODE="sl"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "sv_SE" ]; then
        LANGUAGE_CODE="sv-SE"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "tr" ]; then
        LANGUAGE_CODE="tr-TR"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "uk" ]; then
        LANGUAGE_CODE="uk"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "vi" ]; then
        LANGUAGE_CODE="vi"
    elif [ "$ORIGINAL_LANGUAGE_CODE" == "zh_TW" ]; then
        LANGUAGE_CODE="zh-TW"
    else
        # Play stores don't ask for the other languages.
        continue;
    fi

    # Must do this as a for loop rather than a `contains` with multiple clauses to get the proper ordering.
    MISSING_TRANSLATION=false
    TRANSLATED_TEXT=""
    if [ "$LANGUAGE_CODE" == "en-US" ]; then
        for BLOCK_ID in "${BLOCK_IDS[@]}"
        do
          if [[ ($BLOCK_ID == *"bullet"* || $BLOCK_ID == *"Bullet"*) && $BLOCK_ID != *"generalUpdateBulletIntro"* ]]; then
            TRANSLATED_TEXT+="- "
          fi
          TRANSLATED_TEXT+=$(xmlstarlet sel \
                  -N x="urn:oasis:names:tc:xliff:document:1.2" \
                  -t -m "/x:xliff/x:file/x:body/x:trans-unit[$BLOCK_ID]" \
                  -v "concat(x:source, '\n')" -n \
                  "$XLIFF_FILE")
        done
    else
        for BLOCK_ID in "${BLOCK_IDS[@]}"
        do
          if [[ ($BLOCK_ID == *"bullet"* || $BLOCK_ID == *"Bullet"*) && $BLOCK_ID != *"generalUpdateBulletIntro"* ]]; then
            TRANSLATED_TEXT+="- "
          fi
          POTENTIAL_TRANSLATION=$(xmlstarlet sel \
                  -N x="urn:oasis:names:tc:xliff:document:1.2" \
                  -t -m "/x:xliff/x:file/x:body/x:trans-unit[$BLOCK_ID]" \
                  -v "concat(x:target, '\n')" -n \
                  "$XLIFF_FILE")
          if [[ "$POTENTIAL_TRANSLATION" == "\n" ]]; then
              MISSING_TRANSLATION=true
          else
              TRANSLATED_TEXT+=$POTENTIAL_TRANSLATION
          fi
        done
    fi

    # Remove the last line break.
    TRANSLATED_TEXT_LENGTH=${#TRANSLATED_TEXT}
    TRANSLATED_TEXT="${TRANSLATED_TEXT:0:$TRANSLATED_TEXT_LENGTH-2}"

    # Check if the output is not empty
    if [ $MISSING_TRANSLATION == false ]; then
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
        echo "❌ Release note translations missing for: $LANGUAGE_CODE ($ORIGINAL_LANGUAGE_CODE)"
    fi
done

echo ""
echo "Release notes written to: $RELEASE_NOTES_FILE"
