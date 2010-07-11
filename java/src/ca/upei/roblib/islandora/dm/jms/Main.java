package ca.upei.roblib.islandora.dm.jms;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

public class Main {
	private static final Logger LOG = Logger.getLogger(Main.class.getName());

	public static void main(String[] args) throws Exception {
		LOG.info("starting jms clients");
		
		Map<String, String> env = System.getenv();
		Map<String, String> settings = new HashMap<String, String>();
		
		for (Entry<String, String> entry : env.entrySet()) {
			if (entry.getKey().contains("islandora")) {
				settings.put(entry.getKey(), entry.getValue());
			}
		}
		
		new TiffConverterListener(settings).start();
		LOG.info("startup complete");
	}
}
