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
  
  const PID_NAMESPACE = 'islandora-dm';
  const PID_PO_COLLECTION = 'islandora-dm:po-collection';
  const PID_PO_CMODEL = 'islandora-dm:po-cmodel';
  const PID_PO_DOCUMENT_CMODEL = 'islandora-dm:po-document-cmodel';
  const PID_PO_PAGE_CMODEL = 'islandora-dm:po-page-cmodel';
  const PID_PO_NOTE_CMODEL = 'islandora-dm:po-note-cmodel';
  const PID_PO_ATTACHMENT_CMODEL = 'islandora-dm:po-attachment-cmodel';
  
  const DSID_DEFAULT_CLASSIFICATION = 'defaultClassification';
  const DSLABEL_DEFAULT_CLASSIFICATION = 'default classification metadata';
  const DSID_CLASSIFICATION = 'classification';
  const DSLABEL_CLASSIFICATION = 'classification metadata';
  
  const DSID_DEFAULT_DISPOSITION = 'defaultDisposition';
  const DSLABEL_DEFAULT_DISPOSITION = 'default disposition metadata';
  const DSID_DISPOSITION = 'disposition';
  const DSLABEL_DISPOSITION = 'disposition metadata';
  
  const DSID_DM = 'dm';
  const DSLABEL_DM = 'islandora-dm metadata';
  
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
  
  const PO_STATUS_CLOSED = 'closed';
  const PO_STATUS_NEW = 'new';
}

