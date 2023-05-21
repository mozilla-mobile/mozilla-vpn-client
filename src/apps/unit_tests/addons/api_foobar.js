(function(api, condition) {
if (!('foobar' in api) || api.foobar != 42) {
  return;
}

condition.enable();
})
