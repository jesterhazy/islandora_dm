package ca.upei.roblib.islandora.dm.jms;

import java.util.Map;

public abstract class TiffDatastreamListener extends FedoraListener {

	private static final String JMS_MESSAGE_SELECTOR = "methodName = 'addDatastream' and pid like 'islandora-dm:%'";
	private static final String TARGET_DSID = "tiff";

	public TiffDatastreamListener(Map<String, String> settings) {
		super(settings);
	}
	
	@Override
	void handleMessage(String pid, String dsid) throws Exception {
		if (!isTargetMessage(pid, dsid)) {
			if (log.isDebugEnabled()) {
				log.debug(String.format("ignoring message for pid %s, dsid %s", pid, dsid));
			}
		}
		
		else {
			handleTiffMessage(pid, dsid);
		}
	}
	
	@Override
	protected String getJmsMessageSelector() {
		return JMS_MESSAGE_SELECTOR;
	}
	
	abstract void handleTiffMessage(String pid, String dsid);

	private boolean isTargetMessage(String pid, String dsid) {
		return TARGET_DSID.equals(dsid);
	}
}
