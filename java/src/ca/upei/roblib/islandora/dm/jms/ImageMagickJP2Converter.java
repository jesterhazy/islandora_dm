package ca.upei.roblib.islandora.dm.jms;

import java.io.File;

import org.apache.log4j.Logger;

public class ImageMagickJP2Converter implements DatastreamConverter {
	final Logger log = Logger.getLogger(getClass().getName());

	private static final String DS_LABEL = "jp2 image";
	private static final String FILENAME_SUFFIX = "jp2";
	private static final String OUTPUT_DSID = "jp2";
	
	@Override
	public void convert(File input, File output) throws Exception {
		
		//convert 1.tiff 1.jp2
		throw new Exception("disabled");

//		String command = String.format("convert %s %s", 
//				input.getAbsolutePath(), 
//				output.getAbsolutePath());
//		
//		if (log.isDebugEnabled()) {
//			log.debug("executing command: " + command);
//		}
//		
//		Process p = Runtime.getRuntime().exec(command);
//		int status = p.waitFor();
//		if (status != 0) {
//			throw new Exception("error converting image");
//		}
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
