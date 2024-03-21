

# Install Linux Releases from the Mozilla Debian Repo: 

On `ship-client` we will upload .deb packages from taskcluster to the [mozilla apt repo](https://blog.nightly.mozilla.org/2023/10/30/introducing-mozillas-firefox-nightly-deb-packages-for-debian-based-linux-distributions/)

## First, import the APT repository signing key:
```bash
sudo install -d -m 0755 /etc/apt/keyrings  
wget -q https://packages.mozilla.org/apt/repo-signing-key.gpg -O- | sudo tee /etc/apt/keyrings/packages.mozilla.org.asc > /dev/null  
```

## Next, add the APT repository to your sources list:
```bash
echo "deb [signed-by=/etc/apt/keyrings/packages.mozilla.org.asc] https://packages.mozilla.org/apt mozilla main" | sudo tee -a /etc/apt/sources.list.d/mozilla.list > /dev/null
``

## Update the package list
sudo apt-get update
apt-cache search mozillavpn
