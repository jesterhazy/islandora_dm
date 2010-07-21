package ca.upei.roblib.islandora.dm.jms;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import ca.upei.roblib.islandora.dm.fedora.SimpleFedoraClient;
import ca.upei.roblib.islandora.dm.util.Constants;

public class TiffConverterListener extends TiffDatastreamListener {
	
	private final List<DatastreamConverter> converters;
	private ThreadLocal<SimpleFedoraClient> client = new ThreadLocal<SimpleFedoraClient>();

	
	private static final String FEDORA_USER_KEY = "_islandora_dm_jms_fedora_user";
	private static final String FEDORA_PASS_KEY = "_islandora_dm_jms_fedora_pass";
	private static final String FEDORA_URL_KEY = "_islandora_dm_jms_fedora_url";

	public TiffConverterListener(Map<String, String> settings) {
		super(settings);
		
		converters = Arrays.asList(
				new ImageMagickThumbnailConverter(),
//				new ImageMagickJP2Converter(),
//				new PdfConverter(),
				new AbbyConverter()
			);
	}

	@Override
	void handleTiffMessage(String pid, String dsid) {
		File input = null;
		
		try {
			try {
				initializeClient();
				
				try {				
					input = fetchInputFile(pid, dsid);
					for (DatastreamConverter converter : converters) {
						File output = null;
						try {
							output = getOutputFile(converter.getFilenameSuffix());
							converter.convert(input, output);
							storeOutputFile(pid, converter.getOutputDsid(), converter.getOutputDsLabel(), output);
						}
						
						catch (Exception e) {
							log.error(String.format("error creating datastream %s for pid %s", converter.getOutputDsid(), pid));
						}
						
						finally {
							deleteTempFile(output);
						}
					}
				}
				
				finally {
					deleteTempFile(input);
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

	private File fetchInputFile(String pid, String dsid) throws Exception {
		File file = getFedora().getDatastream(pid, dsid);
		log.debug(String.format("input file is: %s", file.getAbsolutePath()));
		return file;
	}
	
	private File getOutputFile(String suffix) throws IOException {
		File file = File.createTempFile(Constants.TEMP_FILE_PREFIX, "." + suffix);
		log.debug(String.format("output file is: %s", file.getAbsolutePath()));
		return file;
	}
	
	private void storeOutputFile(String pid, String dsid, String dsLabel, File output) throws Exception {
		getFedora().addDatastream(pid, dsid, dsLabel, output);
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
	
	private void deleteTempFile(File file) {
		if (file != null && file.exists()) {
			if (!file.delete()) {
				log.warn("failed to delete temp file " + file.getAbsolutePath());
			}
		}
	}
}
