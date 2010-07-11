package ca.upei.roblib.islandora.dm.jms;

import java.io.File;

import org.apache.log4j.Logger;

public class AbbyConverter implements DatastreamConverter {
	final Logger log = Logger.getLogger(getClass().getName());

	private static final String DS_LABEL = "ocr xml";
	private static final String FILENAME_SUFFIX = "xml";
	private static final String OUTPUT_DSID = "ocrXml";
	
	@Override
	public void convert(File input, File output) throws Exception {
		throw new Exception("unimplemented");
	}

	@Override
	public String getFilenameSuffix() {
		return FILENAME_SUFFIX;
	}

	@Override
	public String getOutputDsid() {
		return OUTPUT_DSID;
	}

	@Override
	public String getOutputDsLabel() {
		return DS_LABEL;
	}
}
