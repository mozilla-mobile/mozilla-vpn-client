(function(vpn, condition) {
// Let's use XHR to be async
const xhr = new XMLHttpRequest();
xhr.open('GET', 'https://mozilla.org');
xhr.send();
xhr.onreadystatechange = () => condition.enable()
})
