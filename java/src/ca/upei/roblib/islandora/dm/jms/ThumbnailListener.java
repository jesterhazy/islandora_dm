package ca.upei.roblib.islandora.dm.jms;

import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.Map;

import javax.imageio.ImageIO;

public class ThumbnailListener extends TiffDatastreamListener {
	private static final String THUMBNAIL_DS_LABEL = "thumbnail image";
	private static final String THUMBNAIL_JAI_FORMAT_NAME = "jpeg";
	private static final int THUMBNAIL_HEIGHT = 110;
	private static final int THUMBNAIL_WIDTH = 85;
	private static final String FILENAME_SUFFIX = "jpg";
	private static final String OUTPUT_DSID = "tn";

	public ThumbnailListener(Map<String, String> env) {
		super(env);
	}

	@Override
	void convert(File input, File output) throws Exception {
		try {
			BufferedImage sourceImage = ImageIO.read(input);
			Image thumbnail = sourceImage.getScaledInstance(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
					Image.SCALE_SMOOTH);

			BufferedImage bi = new BufferedImage(thumbnail.getWidth(null),
					thumbnail.getHeight(null), BufferedImage.TYPE_INT_RGB);
			bi.getGraphics().drawImage(thumbnail, 0, 0, null);
			ImageIO.write(bi, THUMBNAIL_JAI_FORMAT_NAME, output);
		} catch (IOException e) {
			log.error("failed to convert data for dsid: " + getOutputDsid());
			throw e;
		}
	}

	@Override
	String getFilenameSuffix() {
		return FILENAME_SUFFIX;
	}

	@Override
	String getOutputDsid() {
		return OUTPUT_DSID;
	}

	@Override
	String getOutputDsLabel() {
		return THUMBNAIL_DS_LABEL;
	}
}
