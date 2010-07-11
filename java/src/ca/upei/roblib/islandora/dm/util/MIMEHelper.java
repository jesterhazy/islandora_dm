package ca.upei.roblib.islandora.dm.util;

import java.io.File;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

public class MIMEHelper {
	private static final String DEFAULT_FILE_EXTENSION = "dat";
	private static final String DEFAULT_MIME_TYPE = "application/octet-stream";
	
	private static final Map<String, String> FILE_EXTENSION_MAP;
	private static final Map<String, String> MIME_TYPE_MAP;
	static {
		Map<String, String> m1 = new HashMap<String, String>();
		m1.put("image/tiff", "tif");
		m1.put("image/jp2", "jp2");
		FILE_EXTENSION_MAP = Collections.unmodifiableMap(m1);
		
		Map<String, String> m2 = new HashMap<String, String>();
		for (Entry<String, String> entry : m1.entrySet()) {
			m2.put(entry.getValue(), entry.getKey());
		}
		
		MIME_TYPE_MAP = Collections.unmodifiableMap(m2);
	}
	

	
	public static String getFileExtension(String mimeType) {
		String extension = FILE_EXTENSION_MAP.get(mimeType);
		
		if (extension == null) {
			extension = DEFAULT_FILE_EXTENSION;
		}
		
		return extension;
	}
	
	public static String getMimeType(File file) {
		
		String extension = null;
		
		int pos = file.getName().lastIndexOf(".");
		if (pos > 0) {
			extension = file.getName().substring(pos);
		}
		
		return getMimeType(extension);
	}
	
	public static String getMimeType(String fileExtension) {
		String extension = MIME_TYPE_MAP.get(fileExtension);
		
		if (extension == null) {
			extension = DEFAULT_MIME_TYPE;
		}
		
		return extension;
	}
}
