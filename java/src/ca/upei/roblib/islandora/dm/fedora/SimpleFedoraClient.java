package ca.upei.roblib.islandora.dm.fedora;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.rmi.RemoteException;

import org.apache.log4j.Logger;

import ca.upei.roblib.islandora.dm.util.Constants;
import ca.upei.roblib.islandora.dm.util.MIMEHelper;
import fedora.client.FedoraClient;
import fedora.server.access.FedoraAPIA;
import fedora.server.management.FedoraAPIM;
import fedora.server.types.gen.MIMETypedStream;

public class SimpleFedoraClient {
	private static final String FEDORA_ACTIVE_FLAG = "A";
	private static final String FEDORA_MANAGED_FLAG = "M";
	private Logger log = Logger.getLogger(getClass().getName());
	private FedoraClient fedoraClient;
	private FedoraAPIA apia;
	private FedoraAPIM apim;
	
	public void addDatastream(String pid, String dsid, String label, File file) throws Exception {
		String mimeType = MIMEHelper.getMimeType(file);
		
		try {
			String url = fedoraClient.uploadFile(file);
			apim.addDatastream(pid, dsid, null, label, true, mimeType, null, url, FEDORA_MANAGED_FLAG, FEDORA_ACTIVE_FLAG, null, null, null);
		} catch (RemoteException e) {
			log.error(String.format("error storing datastream %s in fedora object %s", dsid, pid));
			throw e;
		} catch (IOException e) {
			log.error(String.format("error uploading file %s to fedora", file.getAbsolutePath()));
			throw e;
		}	
	}

	public File getDatastream(String pid, String dsid) throws Exception {
		File output = null;
		MIMETypedStream ds = null;
		boolean ok = false;
		
		try {
			ds = apia.getDatastreamDissemination(pid, dsid, null);
			
			String extension = MIMEHelper.getFileExtension(ds.getMIMEType());
			output = File.createTempFile(Constants.TEMP_FILE_PREFIX, "." + extension);
			
			OutputStream os = new BufferedOutputStream(new FileOutputStream(output));
			os.write(ds.getStream());
			ok = true;
		}
		
		catch (RemoteException e) {
			log.error(String.format("error processing datatream %s for pid %s", dsid, pid));
			throw e;
		}
		
		catch (IOException e) {
			log.error(String.format("error saving local copy of datatream %s for pid %s", dsid, pid));
			throw e;
		}
		
		finally {
			if (!ok && output != null && output.exists()) {
				output.delete();
			}
		}
		
		return output;
	}

	public void initialize(String fedoraUrl, String fedoraUser, String fedoraPass) throws Exception {
		try {
			fedoraClient = new FedoraClient(fedoraUrl, fedoraUser, fedoraPass);
			apia = fedoraClient.getAPIA();
			apim = fedoraClient.getAPIM();
		} catch (Exception e) {
			log.error(String.format("can't connect to %s", fedoraUrl));
			throw e;
		}
	}
}
