

# Install Linux Preview-Releases: 

On `promote-client` we will upload .deb packages from taskcluster to the [mozilla apt repo](https://blog.nightly.mozilla.org/2023/10/30/introducing-mozillas-firefox-nightly-deb-packages-for-debian-based-linux-distributions/)

## First, import the APT repository signing key:
wget -q https://repository.stage.productdelivery.nonprod.webservices.mozgcp.net/apt/repo-signing-key.gpg -O- | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/repository.stage.productdelivery.nonprod.webservices.mozgcp.net.gpg > /dev/null
> The fingerprint should be 35BA A0B3 3E9E B396 F59C A838 C0BA 5CE6 DC63 15A3

## Next, add the APT repository to your sources list:
echo "deb [signed-by=/etc/apt/trusted.gpg.d/repository.stage.productdelivery.nonprod.webservices.mozgcp.net.gpg] https://repository.stage.productdelivery.nonprod.webservices.mozgcp.net/apt mozilla main" | sudo tee -a /etc/apt/sources.list.d/mozilla.list > /dev/null

## Update the package list
sudo apt-get update
apt-cache search mozillavpn
