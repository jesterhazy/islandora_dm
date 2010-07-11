package ca.upei.roblib.islandora.dm.jms;

import java.io.File;

public interface DatastreamConverter {
	String getFilenameSuffix();
	String getOutputDsid();
	String getOutputDsLabel();
	void convert(File input, File output) throws Exception;
}
