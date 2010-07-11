package ca.upei.roblib.islandora.dm.jms;

import java.io.File;
import java.io.IOException;
import java.util.Collections;
import java.util.Map;
import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.TextMessage;
import javax.naming.Context;

import org.apache.log4j.Logger;

import ca.upei.roblib.islandora.dm.fedora.SimpleFedoraClient;
import ca.upei.roblib.islandora.dm.util.Constants;
import fedora.client.messaging.JmsMessagingClient;
import fedora.client.messaging.MessagingClient;
import fedora.client.messaging.MessagingListener;
import fedora.server.errors.MessagingException;
import fedora.server.messaging.JMSManager;

public abstract class TiffDatastreamListener implements MessagingListener {
	
	private static final String FEDORA_EXTRACT_DSID_PATTERN = ".*<category term=\"([^\"]+)\" scheme=\"fedora-types:dsID\" label=\"xsd:string\"></category>.*";
	private static final String FEDORA_TOPIC_PROPERTY_VALUE = "fedora.apim.*";
	private static final String FEDORA_TOPIC_PROPERTY_NAME = "topic.fedora";
	private static final String FEDORA_MESSAGE_PID_PROPERTY = "pid";
	private static final String TARGET_DSID = "tiff"; 
	private static final String JMS_PROVIDER_URL = "tcp://localhost:61616";
	private static final String JMS_INITIAL_CONTEXT_FACTORY = "org.apache.activemq.jndi.ActiveMQInitialContextFactory";
	private static final String JMS_CONNECTION_FACTORY_NAME = "ConnectionFactory";
	private static final String JMS_MESSAGE_SELECTOR = "methodName = 'addDatastream' and pid like 'islandora-dm:%'";
	private static final boolean JMS_DURABLE_FLAG = false;
	private static final String FEDORA_USER_KEY = "_islandora_dm_java_fedora_user";
	private static final String FEDORA_PASS_KEY = "_islandora_dm_java_fedora_pass";
	private static final String FEDORA_URL_KEY = "_islandora_dm_java_fedora_url";

	final Logger log = Logger.getLogger(getClass().getName());
	final Map<String, String> settings;
	
	private MessagingClient messagingClient;
	private ThreadLocal<SimpleFedoraClient> client = new ThreadLocal<SimpleFedoraClient>();
	
	public TiffDatastreamListener(Map<String, String> settings) {
		this.settings = Collections.unmodifiableMap(settings);
	}
	
	abstract String getFilenameSuffix();
	abstract String getOutputDsid();
	abstract String getOutputDsLabel();
	abstract void convert(File input, File output) throws Exception;

	public void start() throws MessagingException {
		log.info(String.format("starting jms client %s", getClientId()));
		
		Properties properties = new Properties();
		properties.setProperty(Context.INITIAL_CONTEXT_FACTORY,
				JMS_INITIAL_CONTEXT_FACTORY);
		properties.setProperty(Context.PROVIDER_URL, JMS_PROVIDER_URL);
		properties.setProperty(JMSManager.CONNECTION_FACTORY_NAME,
				JMS_CONNECTION_FACTORY_NAME);
		properties.setProperty(FEDORA_TOPIC_PROPERTY_NAME,
				FEDORA_TOPIC_PROPERTY_VALUE);

		messagingClient = new JmsMessagingClient(getClientId(), this,
				properties, JMS_MESSAGE_SELECTOR, JMS_DURABLE_FLAG);

		messagingClient.start();
	}

	String getClientId() {
		return getClass().getName();
	}

	public void stop() throws MessagingException {
		messagingClient.stop(false);
	}

	public void onMessage(String clientId, Message message) {
		String pid = null;
		String dsid = null;

		try {
			pid = getPid(message);
			dsid = getDsid(message);
			log.debug(String.format("pid: %s, dsid: %s", pid, dsid));
		} catch (JMSException e) {
			log.error("error retrieving message text: " + e.getMessage(), e);
		}

		if (isProcessingRequired(pid, dsid)) {
			process(pid, dsid);
			log.debug("processing complete");
		}
	}

	private void process(String pid, String dsid) {

		File input = null;
		File output = null;

		try {
			try {
				initializeClient();

				try {
					input = getInputFile(pid, dsid);
					output = getOutputFile();
					convert(input, output);
					storeOutputFile(pid, output);
				}

				finally {
					deleteTempFile(input);
					deleteTempFile(output);
				}
			}

			finally {
				removeClient();
			}
		}

		catch (Exception e) {
			log.error(String.format(
					"error processing datatream %s for pid %s: %s", dsid, pid,
					e.getMessage()), e);
		}
	}

	private void deleteTempFile(File file) {
		if (file != null && file.exists()) {
			if (!file.delete()) {
				log.warn("failed to delete temp file " + file.getAbsolutePath());
			}
		}
	}

	private void removeClient() {
		client.remove();
	}

	private void initializeClient() throws Exception{
		String url = settings.get(FEDORA_URL_KEY);
		String user = settings.get(FEDORA_USER_KEY);
		String pass = settings.get(FEDORA_PASS_KEY);
		log.info(String.format("Connecting to fedora at %s as user %s", url, user));
		
		getFedora().initialize(url, user, pass);
	}

	private SimpleFedoraClient getFedora() {
		SimpleFedoraClient fedora = client.get();

		if (fedora == null) {
			fedora = new SimpleFedoraClient();
			client.set(fedora);
		}

		return fedora;
	}

	private void storeOutputFile(String pid, File output) throws Exception {
		getFedora().addDatastream(pid, getOutputDsid(), getOutputDsLabel(), output);
	}

	private File getInputFile(String pid, String dsid) throws Exception{
		File file = getFedora().getDatastream(pid, dsid);
		log.debug(String.format("input file is: %s", file.getAbsolutePath()));
		return file;
	}

	private File getOutputFile() throws IOException {
		File file = File.createTempFile(Constants.TEMP_FILE_PREFIX, "." + getFilenameSuffix());
		log.debug(String.format("output file is: %s", file.getAbsolutePath()));
		return file;
	}

	/**
	 * Should this message be processed? 
	 * We only care about addDatastream
	 * messages where the datastream id is "tiff".
	 * 
	 * @param pid
	 *            the pid associated with the incoming message
	 * @param dsid
	 *            the dsid extracted from the incoming message
	 * @return true if message should be processed.
	 */
	private boolean isProcessingRequired(String pid, String dsid) {
		return TARGET_DSID.equals(dsid);
	}

	private String getPid(Message message) throws JMSException {
		return message.getStringProperty(FEDORA_MESSAGE_PID_PROPERTY);
	}

	private String getDsid(Message message) throws JMSException {
		String messageText = ((TextMessage) message).getText();
		return getDsid(messageText);
	}

	private String getDsid(String messageText) {
		String dsid = null;

		if (messageText != null) {

			Matcher m = Pattern.compile(FEDORA_EXTRACT_DSID_PATTERN, Pattern.DOTALL).matcher(
					messageText);

			if (m.matches()) {
				dsid = m.group(1);
			}
		}

		if (dsid == null && log.isDebugEnabled()) {
			log.debug(String.format("failed to extract dsid from message %s", messageText));
		}
		
		return dsid;
	}
}
