package ca.upei.roblib.islandora.dm.jms;

import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

import org.apache.log4j.Logger;

public class JAIThumbnailConverter implements DatastreamConverter {
	final Logger log = Logger.getLogger(getClass().getName());

	private static final String JAI_FORMAT_NAME = "jpeg";
	private static final String DS_LABEL = "thumbnail image";
	private static final int THUMBNAIL_HEIGHT = 110;
	private static final int THUMBNAIL_WIDTH = 85;
	private static final String FILENAME_SUFFIX = "jpg";
	private static final String OUTPUT_DSID = "tn";
	
	@Override
	public void convert(File input, File output) throws Exception {
		
		try {
			BufferedImage sourceImage = ImageIO.read(input);
			Image thumbnail = sourceImage.getScaledInstance(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
					Image.SCALE_SMOOTH);

			BufferedImage bi = new BufferedImage(thumbnail.getWidth(null),
					thumbnail.getHeight(null), BufferedImage.TYPE_INT_RGB);
			bi.getGraphics().drawImage(thumbnail, 0, 0, null);
			ImageIO.write(bi, JAI_FORMAT_NAME, output);
		} catch (IOException e) {
			log.error("failed to convert data for dsid: " + getOutputDsid());
			throw e;
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
