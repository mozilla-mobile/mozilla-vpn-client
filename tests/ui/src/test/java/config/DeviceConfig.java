package config;

import org.aeonbits.owner.Config;

//This interface links to device.properties file and matches arguments provided in that file

@Config.LoadPolicy(Config.LoadType.MERGE)
@Config.Sources({
		"system:properties",
		"file:src/test/resources/configs/device.properties"
})
public interface DeviceConfig extends Config {
		@Key("deviceName")
		String deviceName();

		@Key("platformName")
		String platformName();

		@Key("platformVersion")
		String platformVersion();

		@Key("app")
		String app();

		@Key("udid")
		String udid();

}