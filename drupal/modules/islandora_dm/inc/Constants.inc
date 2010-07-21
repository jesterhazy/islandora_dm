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
  
  const PID_PO_COLLECTION = 'islandora-dm:po-collection';
  const PID_PO_CMODEL = 'islandora-dm:po-cmodel';
  const PID_PO_DOCUMENT_CMODEL = 'islandora-dm:po-document-cmodel';
  
  const DSID_POLICY = 'POLICY';
  const DSLABEL_POLICY = 'xacml policy';
  const DSID_DEFAULT_POLICY = 'defaultPOLICY';
  const DSLABEL_DEFAULT_POLICY = 'default access policy';
  
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
  const MIME_TYPE_XML = 'text/xml';
  
  const DC_TITLE = 'dc:title';
  const DC_IDENTIFIER = 'dc:identifier';
}

