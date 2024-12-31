var imageLookup = {
  'Question': {
    filenameLight: 'qrc:/nebula/resources/question.svg',
    filenameDark: 'qrc:/nebula/resources/lock.svg'
  },
  'Spinner': {
    filenameLight: 'qrc:/nebula/resources/spinner.svg',
    filenameDark: 'qrc:/nebula/resources/lock.svg'
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
