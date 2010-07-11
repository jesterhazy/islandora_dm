package ca.upei.roblib.islandora.dm.jms;

import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;

import org.apache.log4j.Logger;

public class JAIJP2Converter implements DatastreamConverter {
	final Logger log = Logger.getLogger(getClass().getName());
	private static final String JAI_FORMAT_NAME = "jpeg2000";
	private static final String DS_LABEL = "jp2 image";
	private static final String FILENAME_SUFFIX = "jp2";
	private static final String OUTPUT_DSID = "jp2";
	
	@Override
	public void convert(File input, File output) throws Exception {
		BufferedImage sourceImage = ImageIO.read(input);
		ImageIO.write(sourceImage, JAI_FORMAT_NAME, output);
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
