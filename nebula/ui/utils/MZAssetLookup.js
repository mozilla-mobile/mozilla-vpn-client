var imageLookup = {
  'Chevron': {
    filenameLight: 'qrc:/nebula/resources/chevron.svg',
    filenameDark: 'qrc:/nebula/resources/lock.svg'
  },
  'ExternalLink': {
    filenameLight: 'qrc:/nebula/resources/externalLink.svg',
    filenameDark: 'qrc:/nebula/resources/lock.svg'
  },
  'Back': {
    filenameLight: 'qrc:/nebula/resources/back.svg',
    filenameDark: 'qrc:/nebula/resources/back.svg'
  },
  'CloseDark': {
    filenameLight: 'qrc:/nebula/resources/close-dark.svg',
    filenameDark: 'qrc:/nebula/resources/close-dark.svg'
  },
  'CloseDarker': {
    filenameLight: 'qrc:/nebula/resources/close-darker.svg',
    filenameDark: 'qrc:/nebula/resources/close-darker.svg'
  },
  'CloseWhite': {
    filenameLight: 'qrc:/nebula/resources/close-white.svg',
    filenameDark: 'qrc:/nebula/resources/close-white.svg'
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
  'Logo': {
    filenameLight: 'qrc:/ui/resources/logo.svg',
    filenameDark: 'qrc:/ui/resources/logo.svg'
  },
  'LogoConnecting': {
    filenameLight: 'qrc:/ui/resources/logo-connecting.svg',
    filenameDark: 'qrc:/ui/resources/logo-connecting.svg'
  },
  'PrivacyMask': {
    filenameLight: 'qrc:/ui/resources/logo-dns-privacy.svg',
    filenameDark: 'qrc:/ui/resources/logo-dns-privacy.svg'
  },
  'IconPrivacyMask': {
    filenameLight: 'qrc:/ui/resources/settings/privacy.svg',
    filenameDark: 'qrc:/ui/resources/settings/privacy.svg'
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
  'GlobeColorful': {
    filenameLight: 'qrc:/ui/resources/logo-dns-settings.svg',
    filenameDark: 'qrc:/ui/resources/logo-dns-settings.svg'
  },
  'ShieldOn': {
    filenameLight: 'qrc:/ui/resources/shield-on.svg',
    filenameDark: 'qrc:/ui/resources/shield-on.svg'
  },
  'ShieldOff': {
    filenameLight: 'qrc:/ui/resources/shield-off.svg',
    filenameDark: 'qrc:/ui/resources/shield-off.svg'
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
  },
  'DisclaimerShield': {
    filenameLight: 'qrc:/nebula/resources/shield-green50.svg',
    filenameDark: 'qrc:/nebula/resources/shield-green50.svg'
  }
}

function getImageSource(imageName) {
  const imageData = imageLookup[imageName];
  if (imageData === undefined) {
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
