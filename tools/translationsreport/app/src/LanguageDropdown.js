import { useState } from 'react'

import { Dropdown } from 'semantic-ui-react'

// Language names from src/translations/extras.xliff
export const LANGUAGES = [
  { value: "bg", text: "Bulgarian" },
  { value: "co", text: "Corsican" },
  { value: "cs", text: "Czech" },
  { value: "cy", text: "Welsh" },
  { value: "da", text: "Danish" },
  { value: "de", text: "German" },
  { value: "dsb", text: "Lower Sorbian" },
  { value: "el", text: "Greek" },
  { value: "en", text: "English" },
  { value: "en_CA", text: "Canadian English" },
  { value: "en_GB", text: "British English" },
  { value: "es_ES", text: "Spanish" },
  { value: "es_MX", text: "Mexican Spanish" },
  { value: "fa", text: "Persian" },
  { value: "fi", text: "Finnish" },
  { value: "fr", text: "French" },
  { value: "fy_NL", text: "Western Frisian" },
  { value: "hsb", text: "Upper Sorbian" },
  { value: "hu", text: "Hungarian" },
  { value: "ia", text: "Interlingua" },
  { value: "id", text: "Indonesian" },
  { value: "is", text: "Icelandic" },
  { value: "it", text: "Italian" },
  { value: "ja", text: "Japanese" },
  { value: "lo", text: "Lao" },
  { value: "nl", text: "Dutch" },
  { value: "pa_IN", text: "Punjabi" },
  { value: "pl", text: "Polish" },
  { value: "pt_BR", text: "Portuguese" },
  { value: "pt_PT", text: "European Portuguese" },
  { value: "ru", text: "Russian" },
  { value: "sk", text: "Slovak" },
  { value: "sl", text: "Slovenian" },
  { value: "sq", text: "Albanian" },
  { value: "sv_SE", text: "Swedish" },
  { value: "tr", text: "Turkish" },
  { value: "uk", text: "Ukrainian" },
  { value: "zh_CN", text: "Simplified Chinese" },
  { value: "zh_TW", text: "Traditional Chinese" },
]

function LanguageDropdown({ onChange }) {
  const [value, setValue] = useState()

  const handleChange = (_, { value }) => {
    setValue(value);
    onChange(value);
  }

  return (
    <Dropdown
      button
      className='icon'
      clearable
      floating
      labeled
      icon='filter'
      value={value}
      options={LANGUAGES.map(l => ({
        ...l,
        text: `${l.text} (${l.value})`,
        key: l.text
      }))}
      onChange={handleChange}
      placeholder='Filter by language'
    />
  );
}

export default LanguageDropdown;
