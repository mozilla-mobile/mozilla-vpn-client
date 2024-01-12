(function(api, condition) {
if (!('navigator' in api)) {
  return;
}

condition.enable();
})
