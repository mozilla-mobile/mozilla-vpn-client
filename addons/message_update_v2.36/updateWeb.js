((api) => {
  if ('openLink' in api.urlOpener) {
    api.urlOpener.openLink(api.urlOpener.LinkUpdate);
  } else {
    api.urlOpener.openUrlLabel('update');
  }
});
