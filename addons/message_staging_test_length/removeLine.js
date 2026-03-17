((api) => {
  if (!api.lineCount || api.lineCount === 0) return;
  api.addon.composer.remove('dynamic_' + api.lineCount);
  api.lineCount = api.lineCount - 1;
});
