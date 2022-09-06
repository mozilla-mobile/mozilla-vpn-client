(function(vpn, condition) {
// Let's use XHR to be async
const xhr = new XMLHttpRequest();
xhr.open('GET', 'https://mozilla.org');
xhr.send();
xhr.onreadystatechange = () => {
  condition.enable();

  const xhr2 = new XMLHttpRequest();
  xhr2.open('GET', 'https://mozilla.org');
  xhr2.send();
  xhr2.onreadystatechange = () => {
    condition.disable()
  }
}
})
