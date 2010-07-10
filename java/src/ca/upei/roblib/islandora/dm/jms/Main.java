package ca.upei.roblib.islandora.dm.jms;

import org.apache.log4j.Logger;

public class Main {
	private static final Logger LOG = Logger.getLogger(Main.class.getName());
	public static void main(String[] args) throws Exception {
		LOG.info("starting jms clients");
		
		new ThumbnailListener().start();
		
		LOG.info("done");
	}
}
