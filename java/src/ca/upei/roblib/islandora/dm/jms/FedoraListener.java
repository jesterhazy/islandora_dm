package ca.upei.roblib.islandora.dm.jms;

import java.util.Map;
import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.TextMessage;
import javax.naming.Context;

import org.apache.log4j.Logger;

import fedora.client.messaging.JmsMessagingClient;
import fedora.client.messaging.MessagingClient;
import fedora.client.messaging.MessagingListener;
import fedora.server.errors.MessagingException;
import fedora.server.messaging.JMSManager;

public abstract class FedoraListener implements MessagingListener {
	final Logger log = Logger.getLogger(getClass().getName());
	final Map<String, String> settings;
	
	private MessagingClient messagingClient;
	
	private static final String FEDORA_EXTRACT_DSID_PATTERN = ".*<category term=\"([^\"]+)\" scheme=\"fedora-types:dsID\" label=\"xsd:string\"></category>.*";
	private static final String FEDORA_TOPIC_PROPERTY_VALUE = "fedora.apim.*";
	private static final String FEDORA_TOPIC_PROPERTY_NAME = "topic.fedora";
	private static final String FEDORA_MESSAGE_PID_PROPERTY = "pid";
	private static final String JMS_PROVIDER_URL = "tcp://localhost:61616";
	private static final String JMS_INITIAL_CONTEXT_FACTORY = "org.apache.activemq.jndi.ActiveMQInitialContextFactory";
	private static final String JMS_CONNECTION_FACTORY_NAME = "ConnectionFactory";
	private static final boolean JMS_DURABLE_FLAG = false;
	
	public FedoraListener(Map<String, String> settings) {
		this.settings = settings;
	}
	
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
				properties, getJmsMessageSelector(), JMS_DURABLE_FLAG);

		messagingClient.start();
	}

	protected String getJmsMessageSelector() {
		return null;
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
		} catch (JMSException e) {
			log.error("error retrieving message text: " + e.getMessage(), e);
		}
		
		try {
			handleMessage(pid, dsid);
		} catch (Exception e) {
			log.error("error handling message: " + e.getMessage(), e);
		}
		
		log.debug("processing complete");
	}
	
	abstract void handleMessage(String pid, String dsid) throws Exception;

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
