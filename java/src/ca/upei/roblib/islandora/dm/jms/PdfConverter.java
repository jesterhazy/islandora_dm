package ca.upei.roblib.islandora.dm.jms;

import java.io.File;

import org.apache.log4j.Logger;

import com.artofsolving.jodconverter.DefaultDocumentFormatRegistry;
import com.artofsolving.jodconverter.DocumentConverter;
import com.artofsolving.jodconverter.DocumentFamily;
import com.artofsolving.jodconverter.DocumentFormat;
import com.artofsolving.jodconverter.openoffice.connection.OpenOfficeConnection;
import com.artofsolving.jodconverter.openoffice.connection.SocketOpenOfficeConnection;
import com.artofsolving.jodconverter.openoffice.converter.OpenOfficeDocumentConverter;

public class PdfConverter implements DatastreamConverter {
	final Logger log = Logger.getLogger(getClass().getName());
	private static final String DS_LABEL = "pdf image";
	private static final String FILENAME_SUFFIX = "pdf";
	private static final String OUTPUT_DSID = "pdf";
	
	private final DocumentFormat tiff;
	private final DocumentFormat pdf;
	private DefaultDocumentFormatRegistry formatRegistry;
	
	public PdfConverter() {
		formatRegistry = new DefaultDocumentFormatRegistry();
		tiff = new DocumentFormat("TIFF", DocumentFamily.DRAWING, "image/tiff", "tif"); 
		formatRegistry.addDocumentFormat(tiff);
		pdf = formatRegistry.getFormatByFileExtension("pdf");
		pdf.setExportFilter(DocumentFamily.DRAWING, "draw_pdf_Export");
	}
	
	@Override
	public void convert(File input, File output) throws Exception {
		// connect to an OpenOffice.org instance running on port 8100
		OpenOfficeConnection connection = new SocketOpenOfficeConnection(8100);
		connection.connect();
		
		// convert
		DocumentConverter converter = new OpenOfficeDocumentConverter(connection, formatRegistry);
		
		converter.convert(input, tiff, output, pdf);
		 
		// close the connection
		connection.disconnect();
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
