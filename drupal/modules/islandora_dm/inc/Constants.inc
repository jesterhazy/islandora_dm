<?php

class Constants {
	const MODULE_NAME = 'islandora_dm';
	const DRUPAL_MESSAGE_STATUS = 'status';
	const DRUPAL_MESSAGE_WARNING = 'warning';
	const DRUPAL_MESSAGE_ERROR = 'error';
 
	const PERMISSION_ADMIN = 'admin islandora_dm';
	const PERMISSION_EDIT = 'edit islandora_dm';
	const PERMISSION_VIEW = 'view islandora_dm';
	
	const IMPORT_FOLDER_PATH = 'islandora_dm_import_folder_path';
	const IMPORT_ARCHIVE_PATH = 'islandora_dm_import_archive_path';
	const IMPORT_ARCHIVE_DISABLED = 'islandora_dm_archive_disabled';
	const IMPORT_ARCHIVE_DATE_FORMAT = 'Ymd-Hi';

	const SETTING_SOLR_URL = 'fedora_solr_search_url';
	const SETTING_SOLR_URL_DEFAULT = 'http://localhost:8080/solr';
		
	const PID_NAMESPACE = 'islandora-dm';
	const PID_COLLECTION_PURCHASE_ORDERS = 'islandora-dm:purchase-orders';
	const PID_COLLECTION_PURCHASE_ORDERS_UNLINKED_DOCUMENTS = 'islandora-dm:purchase-orders-unlinked-documents';
	const PID_COLLECTION_PURCHASE_ORDERS_INCOMPLETE_IMPORT = 'islandora-dm:purchase-orders-incomplete-import';
	const PID_CMODEL_PURCHASE_ORDER = 'islandora-dm:cmodel-purchase-order';
	const PID_CMODEL_DOCUMENT = 'islandora-dm:cmodel-document';
	const PID_CMODEL_PAGE = 'islandora-dm:cmodel-page';
	const PID_CMODEL_NOTE = 'islandora-dm:cmodel-note';
	
	const DSID_DEFAULT_CLASSIFICATION = 'defaultClassification';
	const DSLABEL_DEFAULT_CLASSIFICATION = 'default classification metadata';
	const DSID_CLASSIFICATION = 'classification';
	const DSLABEL_CLASSIFICATION = 'classification metadata';
	
	const DSID_DEFAULT_DISPOSITION = 'defaultDisposition';
	const DSLABEL_DEFAULT_DISPOSITION = 'default disposition metadata';
	const DSID_DISPOSITION = 'disposition';
	const DSLABEL_DISPOSITION = 'disposition metadata';
	
	const DSID_UIS = 'uis';
	const DSLABEL_UIS = 'islandora-dm purchase order data';
	
	const DSID_TIFF = 'tiff';
	const DSLABEL_TIFF = 'tiff image';
	const MIME_TYPE_TIFF = 'image/tiff';
	
	const DSID_JP2_ARCHIVE = 'jp2lossless';
	const DSLABEL_JP2_ARCHIVE = 'jp2 image (lossless)';
	const MIME_TYPE_JP2_ARCHIVE = 'image/jp2';
	
	const DSID_JP2 = 'jp2';
	const DSLABEL_JP2 = 'jp2 image';
	const MIME_TYPE_JP2 = 'image/jp2';
	
	const DSID_TN = 'tn';
	const DSLABEL_TN = 'thumbnail image';
	const MIME_TYPE_TN = 'image/jpeg';
	
	const MIME_TYPE_XML = 'text/xml';
	
	const DC_TITLE = 'dc:title';
	const DC_IDENTIFIER = 'dc:identifier';
	
	const DSID_NOTE = 'note';
	const DSLABEL_NOTE = 'note';
	const DSID_NOTE_CREATOR = 'creator';
	const DSLABEL_NOTE_CREATOR = 'note creator';
	
	const DSID_ATTACHMENT = 'attachment';
}
