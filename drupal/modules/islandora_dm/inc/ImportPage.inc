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
    // group by poNumber
    $po_files = array();
    foreach($files as $file) {
      
      // verify read access
      if (!is_readable($file->filename)) {
        drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because the file permissions are wrong.'), Constants::DRUPAL_MESSAGE_ERROR);
      }
      else {
        $po_files[$file->poNumber][] = $file;        
      }
    }
    
    // import each po
    foreach($po_files as $k => $v) {
      try {
        $this->importPurchaseOrder($k, $v);
      }
      catch (Exception $e) {
        watchdog(t(Constants::MODULE_NAME), 'Error importing file @file: @ex', array('@file' => $file->filename, '@ex' => $e), WATCHDOG_WARN);
      }
    }
  }
  
  function importPurchaseOrder($po_number, $files) {
    // look up purchase order (in fedora and uis)
    $po = PurchaseOrder::fetchByNumber($po_number);
    if(!$po->getFedora() && !$po->getUIS()) {
      drupal_set_message(
        t('Import failed for purchase order <b>') 
        . $po_number . t('</b>, because purchase order doesn\'t exist. ') 
        . count($files) . t(' file(s) skipped.'), Constants::DRUPAL_MESSAGE_ERROR);
      return;
    }
    
    // create fedora record if needed
    $po_fedora_item = $po->getFedora();
    if (!$po_fedora_item) {
      $po_fedora_item = $this->createPurchaseOrderRecord($po);
      if ($po_fedora_item) {
        drupal_set_message(t('Created purchase order <b>') . $po->getPid() . t('</b>.'));
      }
      
      else {
        drupal_set_message(
          t('Import failed for purchase order <b>') 
          . $po_number . t('</b>, because an error occurred while storing the purchase order data. ') 
          . count($files) . t(' file(s) skipped.'), Constants::DRUPAL_MESSAGE_ERROR);
        return;   
      }
    }
    
    $this->importDocuments($po, $files);   
  }
  
  function importDocuments(&$po, $files) {
    // group by document (which is date + batch)
    $doc_files = array();
    foreach($files as $file) {
      $k = $file->scanDate . $file->scanBatch;
      $doc_files[$k][] = $file;
    }
    
    // import each document
    foreach($doc_files as $k => $v) {
      $this->importDocument($po, $v);
    }
  }
  
  function importDocument(&$po, $files) {
    // create document record, test success
    $document_fedora_item = $this->createDocumentRecord($po);
    if (!$document_fedora_item) {
      drupal_set_message(t('Can\'t create document for <b>') . $po->getPid() . t('</b>, because an error occurred while storing the document.'), Constants::DRUPAL_MESSAGE_ERROR);              
      return;
    }
    
    $po->addDocument($document_fedora_item->pid);
    drupal_set_message(t('Added document <b>') . $document_fedora_item->pid . t('</b> to purchase order <b>') . $po->getPid() . t('</b>.'));
    
    // import pages
    $this->importPages($po, $document_fedora_item->pid, $files);  
  }
  
  function importPages(&$po, $document_pid, $files) {
    foreach($files as $file) {
      $this->importFile($po, $document_pid, $file);
    }
  }
  
  function importFile(&$po, $document_pid, $file) {
    // create page record, test success
    $page_fedora_item = $this->createPageRecord($po, $document_pid, $file);
    if (!$page_fedora_item) {
      drupal_set_message(t('Can\'t create page for <b>') . $file->basename . t('</b>, because an error occurred while storing the page.'), Constants::DRUPAL_MESSAGE_ERROR);              
      return;
    }
    
    $po->addPage($document_pid, $page_fedora_item->pid);
    drupal_set_message(t('Added page <b>') . $file->basename . t('</b> to document <b>') . $document_pid . t('</b>.'));
    
    // archive the file
    if (!variable_get(Constants::IMPORT_ARCHIVE_PATH, false)) {
      $archived = $this->archiveFile($file);
      if (!$archived) {
        drupal_set_message(t('Imported <b>') . $file->basename . t('</b>, but an error occurred while archiving the file.'), Constants::DRUPAL_MESSAGE_WARNING);
        return;
      }      
    }
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
  
  protected function createDocumentRecord($po) {
    return create_document_fedora_item($po);
  }
  
  protected function createPageRecord($po, $document_pid, $file) {
    return create_page_fedora_item($po, $document_pid, $file);
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
