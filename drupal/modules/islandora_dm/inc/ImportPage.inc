<?php

class ImportPage {
  const BTN_IMPORT_ALL = 'import_all';
  const FILENAME_PATTERN = '/^ *([0-9]+) +([0-9\-]{10}) +\(([0-9]+)\) +([0-9]+) *$/';
  function form() {
    $form = array();
    
    $form['import_all'] = array(
      '#type' => 'submit',
      '#name' => self::BTN_IMPORT_ALL,
      '#value' => t('Import All')
    );

    $form['import_selected'] = array(
      '#type' => 'submit',
      '#name' => 'import_selected',
      '#value' => t('Import Selected')
    );
   
    $files = $this->getFiles();
    $valid = array();
	  $invalid = array();

    $i = 1; // 1-based to compensate for bad drupal checkbox posts
    foreach($files as $file) {
      $file->validFilename ? $valid[$i++] = $file : $invalid[] = $file;
    }
	
	  // TODO: group table by date/po/batch
	  // TODO: add js to help users add/remove complete docs/batches
	    
    $form['files'] = array(
       '#type' => 'checkboxes',
       '#name' => 'files',
       '#options' =>  $valid,
       '#theme' => 'islandora_dm_import_files_checkboxes',
       '#description' => t('Select the files to import, or click the <i>Import All</i> button.')
     );
    
    $this->addInvalidFiles($form, $invalid);

    return $form;
  }
  
  function addInvalidFiles(&$form, $invalid) {
    if (!empty($invalid)) {
      $form['invalid_files']['header'] = array(
        '#value' => '<h4>' . t('Invalid files in import directory') . "</h4>\n<ul>",
      );

      $form['invalid_files']['files'] = array(
        '#type' => 'item',
        '#value' => implode('<br>', Files::extractBasenames($invalid)),
      );

      $form['invalid_files']['footer'] = array(
        '#value' => '</ul>',
      );
    }
  }

  function getFiles() {
    $dir = variable_get(Constants::IMPORT_FOLDER_PATH, null);
    $files = Files::listFiles($dir);
  	$this->parseFilenames($files);
  	
  	usort($files, 'ImportPage::compareFiles');
  
  	return $files;
   }
  
  function compareFiles($f1, $f2) {
    $result = 0;
    if ($f1->validFilename || $f2->validFilename) {
      $result = $f1->validFilename - $f2->validFilename;

      if ($result == 0) {
         $result = strcmp($f1->scanDate, $f2->scanDate);
      }

      if ($result == 0) {
         $result = strcmp($f1->poNumber, $f2->poNumber);
      }

      if ($result == 0) {
        $result = $f1->scanBatch - $f2->scanBatch;
      }

      if ($result == 0) {
         $result = $f1->scanPage - $f2->scanPage;
      }
    }  
   
    return $result;
  }
    
  function themeFileCheckboxes($form) {
    $files = $form['#options'];
    $output = null;
        
    if (empty($files)) {
      $item = array(
        '#type' => 'item', 
        '#name' => 'files', 
        '#value' => t('The are no valid files in the import directory.'),
      );
      
      $output = drupal_render($item);
    }
    
    else {
      $header = array('', t('Filename'), t('Size'), t('Last Modified'));
  
      $i = 1; // 1-based to compensate for bad drupal checkbox posts
      $rows[] = array();
      foreach($files as $file) {
        $checkbox = array(
          '#type' => 'checkbox',
          '#name' => $form['#name'] . '[' . $i++ . ']',
        );
        
        $rows[] = array(
          drupal_render($checkbox),
          check_plain($file->basename),
          Files::formatFilesize($file->size),
          Files::formatFilemtime($file->mtime)
        );
      }

      $output = theme('table', $header, $rows);
    }

    return $output;
  }
  
  function submit($form, &$form_state) {  
    $import_all = (self::BTN_IMPORT_ALL == $form_state['clicked_button']['#name']);
    $selected_files = array();
    
    $i = 1; // 1-based to compensate for bad drupal checkbox posts
    foreach ($form['files']['#options'] as $file) {
      if ($import_all || $form_state['values']['files'][$i]) {
        $selected_files[] = $file;
      }
      
      $i++;
    }
    
    $this->importFiles($selected_files);
  }
  
  function importFiles($files = array()) {
    foreach ($files as $file) {
      try {
        $this->importFile($file);
      }
      
      catch (Exception $e) {
        watchdog(t(Constants::MODULE_NAME), 'Error importing file @file: @ex', array('@file' => $file->filename, '@ex' => $e), WATCHDOG_WARN);
      }
    }
  }
  
  function importFile($file) {
    // TODO cache fedora and uis lookups. maybe.
    drupal_set_message('importFile called for ' . $file->basename);
    return;
    
    // attach po info from filename
    $this->parseFilename($file);
    if (!$file->poNumber) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because the name not valid.'), Constants::DRUPAL_MESSAGE_ERROR);
      return;
    }

    // verify read access
    if (!is_readable($file->filename)) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because the file permissions are wrong.'), Constants::DRUPAL_MESSAGE_ERROR);
      return;
    }
    
    // look up purchase order (in fedora and uis)
    $po = PurchaseOrder::fetchByNumber($file->poNumber);
    if(!$po->getFedora() && !$po->getUIS()) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because purchase order <b>' . $file->poNumber . t('</b> doesn\'t exist.')), Constants::DRUPAL_MESSAGE_ERROR);
      return;
    }
    
    // create fedora record if needed
    $po_fedora_item = $po->getFedora();
    if (!$po_fedora_item) {
      $po_fedora_item = $this->createPurchaseOrderRecord($po);
      if ($po_fedora_item) {
        drupal_set_message(t('Created purchase order <b>') . $file->poNumber . t('</b>.'));
      }
      
      if (!$po_fedora_item) {
        drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because an error occurred while storing the purchase order data.'), Constants::DRUPAL_MESSAGE_ERROR);        
        return;   
      }
    }
    
    // create document record, test success
    $document = $this->createDocumentRecord($po, $po_fedora_item, $file->filename);
    if (!$document) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because an error occurred while storing the document.'), Constants::DRUPAL_MESSAGE_ERROR);              
      return;
    }
    
    if (!variable_get(Constants::IMPORT_ARCHIVE_PATH, false)) {
      $archived = $this->archiveFile($file);
      if (!$archived) {
        drupal_set_message(t('Imported <b>') . $file->basename . t('</b>, but an error occurred while archiving the file.'), Constants::DRUPAL_MESSAGE_WARNING);
        return;
      }      
    }

    drupal_set_message(t('Added file <b>') . $file->basename . t('</b> to purchase order <b>') . $file->poNumber . t('</b>.'));
  }
  
  protected function getArchiveDir() {
    $base_dir = variable_get(Constants::IMPORT_ARCHIVE_PATH, null);
    $date = date(Constants::IMPORT_ARCHIVE_DATE_FORMAT);
    $archive_dir = $base_dir . '/' . $date;
    return $archive_dir;
  }
  
  protected function createPurchaseOrderRecord($po) {
    return create_purchase_order_fedora_item($po);
  }
  
  protected function createDocumentRecord($po, $po_fedora_item, $file) {
    return create_document_fedora_item($po, $po_fedora_item, $file);
  }
  
  protected function archiveFile($file) {
    $archive_dir = $this->getArchiveDir();
    $md = mkdir($archive_dir);
    
    if ($md) {
      $dest = $archive_dir . '/' . $file->basename;

      $cp = copy($file->filename, $dest);

      if ($cp) {
        $del = unlink($file->filename);
      }
    }
   
    $error = $md ? 'make archive directory for' : $cp ? 'copy' : $del ? 'delete' : null;
    
    if ($error) {
      watchdog(t(Constants::MODULE_NAME), t('Failed to ' . $error . ' import file: ' . $file->basename), null, WATCHDOG_WARN);
    }
    
    return $cp && $del;
  }

  protected function parseFilenames(&$files) {
	  foreach ($files as $file) {
  		$this->parseFilename($file);
  	}
  }
  
  protected function parseFilename(&$file) {
  	if (preg_match(self::FILENAME_PATTERN, $file->name, $m)) {
  		$file->validFilename = TRUE;
  		$file->poNumber = $m[1];
  		$file->scanDate = $m[2];
  		$file->scanBatch = $m[3] + 0;
  		$file->scanPage = $m[4] + 0;
  	}
  }
}

/*
 * Drupal callback adapters - map drupal callbacks onto class methods.
 */
 
function islandora_dm_import() {
  return drupal_get_form('islandora_dm_import_form');
}

function islandora_dm_import_form() {
  $page = new ImportPage();
  return $page->form();
}

function islandora_dm_import_form_submit($form, &$form_state) {
  $page = new ImportPage();
  return $page->submit($form, $form_state);
}

function theme_islandora_dm_import_files_checkboxes($form) {
  $page = new ImportPage();
  return $page->themeFileCheckboxes($form);
}
