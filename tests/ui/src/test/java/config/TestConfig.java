package config;

import org.aeonbits.owner.Config;

//This interface links to test.properties file and matches arguments provided in that file

@Config.LoadPolicy(Config.LoadType.MERGE)
@Config.Sources({
		"system:properties",
		"file:src/test/resources/configs/test.properties"
})
public interface TestConfig extends Config {

		@Key("remoteURL")
		String remoteURL();

		@Key("PLATFORM")
		String PLATFORM();
}