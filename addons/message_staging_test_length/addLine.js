((api) => {
  if (api.lineCount === undefined) api.lineCount = 0;
  api.lineCount = api.lineCount + 1;
  const id = 'dynamic_' + api.lineCount;
  const block = api.addon.composer.create(
      id, 'text', {content: 'This is a sample additional line of text.'});
  api.addon.composer.insert(api.lineCount - 1, block);
});
