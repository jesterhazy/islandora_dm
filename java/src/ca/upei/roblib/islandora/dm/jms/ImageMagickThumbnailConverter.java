package ca.upei.roblib.islandora.dm.jms;

import java.io.File;

import org.apache.log4j.Logger;

public class ImageMagickThumbnailConverter implements DatastreamConverter {
	final Logger log = Logger.getLogger(getClass().getName());

	private static final String DS_LABEL = "thumbnail image";
	private static final int THUMBNAIL_HEIGHT = 110;
	private static final int THUMBNAIL_WIDTH = 85;
	private static final String FILENAME_SUFFIX = "jpg";
	private static final String OUTPUT_DSID = "tn";
	
	@Override
	public void convert(File input, File output) throws Exception {
		
		//convert 1.tiff -thumbnail 85x110^ -gravity center -extent 85x110 1.jpg

		String dimensions = String.format("%sx%s", THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
		String command = String.format("convert %s -thumbnail %s^ -gravity center -extent %s %s", 
				input.getAbsolutePath(), 
				dimensions, 
				dimensions, 
				output.getAbsolutePath());
		
		if (log.isDebugEnabled()) {
			log.debug("executing command: " + command);
		}
		
		Process p = Runtime.getRuntime().exec(command);
		int status = p.waitFor();
		if (status != 0) {
			throw new Exception("error converting image");
		}
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
