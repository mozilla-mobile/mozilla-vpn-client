var imageLookup = {
  'Chevron': {
    filenameLight: 'qrc:/nebula/resources/chevron.svg',
    filenameDark: 'qrc:/nebula/resources/chevron.svg'
  },
  'Question': {
    filenameLight: 'qrc:/nebula/resources/question.svg',
    filenameDark: 'qrc:/nebula/resources/lock.svg'
  },
  'QuestionDarker': {
    filenameLight: 'qrc:/ui/resources/settings/questionMark.svg',
    filenameDark: 'qrc:/ui/resources/settings/questionMark.svg'
  },
  'Spinner': {
    filenameLight: 'qrc:/nebula/resources/spinner.svg',
    filenameDark: 'qrc:/nebula/resources/lock.svg'
  },
  'InfoIcon': {
    filenameLight: 'qrc:/nebula/resources/info.svg',
    filenameDark: 'qrc:/nebula/resources/info.svg'
  },
  'SuccessIcon': {
    filenameLight: 'qrc:/nebula/resources/success.svg',
    filenameDark: 'qrc:/nebula/resources/success.svg'
  },
  'WarningIcon': {
    filenameLight: 'qrc:/nebula/resources/warning-gray.svg',
    filenameDark: 'qrc:/nebula/resources/warning-gray.svg'
  },
  'WarningRed': {
    filenameLight: 'qrc:/nebula/resources/warning.svg',
    filenameDark: 'qrc:/nebula/resources/warning.svg'
  },
  'WarningOrange': {
    filenameLight: 'qrc:/nebula/resources/warning-orange.svg',
    filenameDark: 'qrc:/nebula/resources/warning-orange.svg'
  },
  'WarningDarkOrange': {
    filenameLight: 'qrc:/nebula/resources/warning-dark-orange.svg',
    filenameDark: 'qrc:/nebula/resources/warning-dark-orange.svg'
  },
  'WarningWhite': {
    filenameLight: 'qrc:/ui/resources/warning-white.svg',
    filenameDark: 'qrc:/ui/resources/warning-white.svg'
  },
  'GlobeWarning': {
    filenameLight: 'qrc:/ui/resources/globe-warning.svg',
    filenameDark: 'qrc:/ui/resources/globe-warning.svg'
  },
  'CrashWarning': {
    filenameLight: 'qrc:/ui/resources/warning-crash.svg',
    filenameDark: 'qrc:/ui/resources/warning-crash.svg'
  },
  'EyeVisible': {
    filenameLight: 'qrc:/nebula/resources/eye-visible.svg',
    filenameDark: 'qrc:/nebula/resources/eye-visible.svg'
  },
  'EyeHidden': {
    filenameLight: 'qrc:/nebula/resources/eye-hidden.svg',
    filenameDark: 'qrc:/nebula/resources/eye-hidden.svg'
  },
  'CopyTextColor': {
    filenameLight: 'qrc:/nebula/resources/copy.svg',
    filenameDark: 'qrc:/nebula/resources/copy.svg'
  },
  'CopyLinkColor': {
    filenameLight: 'qrc:/ui/resources/copy.svg',
    filenameDark: 'qrc:/ui/resources/copy.svg'
  },
  'Paste': {
    filenameLight: 'qrc:/nebula/resources/paste.svg',
    filenameDark: 'qrc:/nebula/resources/paste.svg'
  }
}

function getImageSource(imageName) {
  const imageData = imageLookup[imageName] if (imageData === undefined) {
    console.error('Unable to find image data for: ' + imageName)
    return
  }
  const potentialName = MZTheme.colors.useDarkAssets ? imageData.filenameDark :
                                                       imageData.filenameLight
  if (potentialName === undefined) {
    console.error('Unable to find image information for: ' + imageName)
    return
  }
  return potentialName
}
