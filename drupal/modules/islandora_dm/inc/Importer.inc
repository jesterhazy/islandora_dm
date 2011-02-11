<?php

/**
 * Importer provides methods used to import documents into the DM system. 
 */
class Importer {
	const BTN_IMPORT_ALL = 'import_all';
	const BTN_RESCHEDULE_ALL = 'reschedule_all';
	const FILENAME_PATTERN = '/^ *([0-9]{4}-[0-9]{2}-[0-9]{2}) +\(([0-9]+)\) +([0-9]+) *$/';
	
	function getFiles() { 
		$dir = variable_get(Constants::IMPORT_FOLDER_PATH, null); 
		$files = Files::listFiles($dir); 
		$this->parseFilenames($files);
		usort($files, array($this, 'compareFiles'));
		return $files;
	}

	function getFilesWithFormKeys() {
		$fk = array();
		$files = $this->getFiles();
		
		$i = 0;
		foreach ($files as $file) {
			$fk['f' . $i] = $file;
			$i++;
		}
		
		return $fk;
	}

	function compareFiles($f1, $f2) {
		$result = 0;

		if ($f1->parsed && $f2->parsed) {
			if ($result == 0) {
				$result = strcmp($f1->scanDate, $f2->scanDate);
			}

			if ($result == 0) {
				$result = $f1->scanBatch - $f2->scanBatch;
			}

			if ($result == 0) {
				$result = $f1->scanPage - $f2->scanPage;
			}
		}
		
		else {
			$result = -($f1->parsed - $f2->parsed);
		}
		
		if ($result == 0) {
			$result = strcmp($f1->name, $f2->name);
		}
		
		return $result;
	}

	protected function parseFilenames(&$files) {
		foreach ($files as $file) {
			$this->parseFilename($file);
		}
	}

	protected function parseFilename(&$file) {
		if (preg_match(self::FILENAME_PATTERN, $file->name, $m)) {
			$file->unnumbered = TRUE;
			$file->scanDate = $m[1];
			$file->scanBatch = $m[2] + 0;
			$file->scanPage = $m[3] + 0;
			$file->parsed = 1;
		}
		else {
			$file->parsed = 0;
		}
	}

	protected function getArchiveDir() {
		$base_dir = variable_get(Constants::IMPORT_ARCHIVE_PATH, null);
		$date = date(Constants::IMPORT_ARCHIVE_DATE_FORMAT);
		$archive_dir = $base_dir . '/' . $date;
		return $archive_dir;
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
			watchdog(Constants::MODULE_NAME, t('Failed to ' . $error . ' import file: ' . $file->basename), null, WATCHDOG_WARN);
		}

		return $cp && $del;
	}

	function importFiles(&$form, &$form_state) {
		$selected_files = $this->getSelectedFiles($form, $form_state);
		$documents = $this->groupDocuments($selected_files);
		$this->importDocuments($documents);
	}
	
	function batchKey($file) {
		return $file->parsed ? $file->scanDate . '-' . $file->scanBatch : $file->name;
	}
	
	function groupDocuments($files = array()) {
		$docs = array();
		foreach($files as $file) {
			$k = $this->batchKey($file);
			$docs[$k][] = $file;
		}
		return $docs;
	}
	
	function importDocuments($documents) {
		foreach($documents as $k => $v) {
			$this->importDocument($k, $v);
		}
	}
	
	function importDocument($scan_batch, $files) {
		$document = $this->createDocument();

		if ($document) {					
			$this->importPages($document, $files);
		}
	}

	function createDocument() {
		try {
			$document = new DMDocument();
			$document->createFedoraObject();
			$document->addMembership($this->getDocumentCollection());
			$document->addMembership($this->getUnlinkedDocumentCollection());
			
			drupal_set_message(t('Added document @d.', array('@d' => $document->pid())));	
		}
		catch (Exception $e) {
			$this->logObjectCreateError($document->pid(), $e);
			drupal_set_message(
				t('Can\'t create document for @s, because an error occurred while storing the document.', 
					array('@s' =>	$scan_batch)),
					Constants::DRUPAL_MESSAGE_ERROR);

			$document = null;
		}
		
		return $document;
	}
	
	function importPages(&$document, $files) {
		foreach ($files as $file) {
			$this->importPage($document, $file);
		}
	}
	
	function importPage(&$document, $file) {
		$page = $this->createPage($document, $file);
		
		if ($page) {
			// archive the file
			if (!variable_get(Constants::IMPORT_ARCHIVE_DISABLED, FALSE)) {
				$archived = $this->archiveFile($file);
				if (!$archived) {
					drupal_set_message(
						t("Imported @s, but an error occurred while archiving the file.", 
						array('@s' => $file->basename)), 
						Constants::DRUPAL_MESSAGE_ERROR);
				}
			}
		}
	}
	
	function createPage(&$document, $file) {
		try {
			$page = new DMPage();
			$page->createFedoraObject();
			$page->addMembership($document->pid());
			$page->addMembership($this->getDocumentCollection());
			$page->addMembership($this->getIncompleteImportCollection());
			$page->addTiffFile($file);
			
			drupal_set_message(t('Added page @p to document @d as @pid', 
				array('@p' => $file->basename, 
					'@d' => $document->pid(),
					'@pid' => $page->pid())));
		}
		catch (Exception $e) {
			$this->logObjectCreateError($page->pid(), $e);
			drupal_set_message(
				t("Can't create page for @s, because an error occurred while storing the page.", 
					array('@s' => $file->basename)), 
					Constants::DRUPAL_MESSAGE_ERROR);
			$page = null;
		}

		return $page;
	}
	
	function logObjectCreateError($pid, $e) {
		watchdog(Constants::MODULE_NAME, 
				'Error while creating fedora object @pid: @e', 
				array('@pid' => $pid, '@e' => $e->getMessage()), WATCHDOG_WARN);
	}
	
	function getDocumentCollection() {
		// purchase orders is hard-coded here, but 
		// this could be changed to a look up function
		// based on the current user's affiliations
		return Constants::PID_COLLECTION_PURCHASE_ORDERS;
	}
	
	function getIncompleteImportCollection() {
		return Constants::PID_COLLECTION_PURCHASE_ORDERS_INCOMPLETE_IMPORT;
	}
	
	function getUnlinkedDocumentCollection() {
		// purchase orders is hard-coded here, but 
		// this could be changed to a look up function
		// based on the current user's affiliations
		return Constants::PID_COLLECTION_PURCHASE_ORDERS_UNLINKED_DOCUMENTS;
	}
	
	function getSelectedFiles(&$form, &$form_state) {
		$selected_files = array();
		$import_all = (self::BTN_IMPORT_ALL == $form_state['clicked_button']['#name']);

		foreach ($form['files']['#options'] as $key => $file) {
			if ($import_all || $form_state['values']['files'][$key]) {
				
				// verify read access
				if (is_readable($file->filename)) {
					$selected_files[] = $file;
				}
				else {
					drupal_set_message(
						t("Can't import @s, because the file permissions are wrong.", 
							array('@s' => $file->basename)), 
						Constants::DRUPAL_MESSAGE_ERROR);				
				}
			}
		}
		
		return $selected_files;
	}
	
	function unlinkedDocumentMarkupStart($document) {
		$view = new ViewDocument($document);
		
		$html = '<div class="islandora-dm-import-link-document search-result">';
		$html .= '<div class="title">';
		$html .= $document->title();
		$html .= '</div>';
		$html .= $view->thumbnailForImport();
		$html .= theme('table', null, $view->documentDetailsAsTableRows());
		
		return $html;
	}

	function unlinkedDocumentMarkupEnd($document) {
		return '</div>';
	}
	
	/**
	 * Load unlinked documents. By default, this will return only documents where 
	 * every page has been properly ingested ($page->incompleteImport() == FALSE).
	 * Passing in a FALSE argument reverses this behaviour.
	 *
	 * @var $incomplete return incomplete docs or not (default == FALSE).
	 */
	function loadUnlinkedDocuments($incomplete = FALSE) {
		$documents = array();
		$pids = DMObject::findRelatedPids(Constants::PID_COLLECTION_PURCHASE_ORDERS_UNLINKED_DOCUMENTS, Constants::PID_CMODEL_DOCUMENT);
		
		foreach($pids as $pid) {
			$document = new DMDocument($pid);
			$document->load();
			
			if ($incomplete & $document->incompleteImport(FALSE)) {
				$documents[] = $document;
			}
		}		
		return $documents;
	}
		
	function getLinkFormDocumentNumber(&$form_state) {
		$button = $form_state['clicked_button']['#name'];
		return preg_replace('/-submit$/', '', $button);
	}
	
	function getLinkFormPurchaseOrderNumbers(&$form_state) {
		$doc = $this->getLinkFormDocumentNumber($form_state);
		return trim($form_state['values'][$doc . '-numbers']);
	}
	
	function splitLinkFormPurchaseOrderNumbers($number_field) {
		return preg_split('/[, ]+/', $number_field);
	}
	
	function linkDocuments($form, &$form_state) {
		try {
			$doc_id = $this->getLinkFormDocumentNumber($form_state);
			$po_number_field = $this->getLinkFormPurchaseOrderNumbers($form_state);
			$po_numbers = $this->splitLinkFormPurchaseOrderNumbers($po_number_field);

			// load document
			$document_pid = DMObject::pidBase() . $doc_id;
			$document = new DMDocument($document_pid);
			$document->load();

			// load purchase orders
			$purchase_orders = $this->loadOrImportPurchaseOrders($po_numbers);

			// attach po pid to documents		
			if (!empty($purchase_orders)) {
				foreach($purchase_orders as $purchase_order) {
					$document->addMembership($purchase_order->pid());
					$document->removeMembership(Constants::PID_COLLECTION_PURCHASE_ORDERS_UNLINKED_DOCUMENTS);
					drupal_set_message(t('Linked document @d to purchase order @p.', 
						array(
							'@d' => $document->pidNumber(), 
							'@p' => $purchase_order->pidNumber())));
				}
			}
		}
		catch (Exception $e) {
			$this->logObjectCreateError($document_pid, $e);
			drupal_set_message(
				t("An error occurred while linking document @s.", 
					array('@s' => $doc_id)), 
				Constants::DRUPAL_MESSAGE_ERROR);
		}
	}
	
	/**
	 * Load or import purchase order objects for each po number in the given array.
	 *
	 * Will return a null set if any errors occur for any po number. 
	 */
	function loadOrImportPurchaseOrders($po_numbers) {
		$purchase_orders = array();
		
		foreach($po_numbers as $number) {
			$po = $this->loadOrImportPurchaseOrder($number);
			if ($po) {
				$purchase_orders[] = $po;
			}
			
			else {
				$purchase_orders = NULL;
				break;
			}
		}
		
		return $purchase_orders;
	}
	
	function loadOrImportPurchaseOrder($number) {
		$pid = DMPurchaseOrder::number2Pid($number);
		$po = new DMPurchaseOrder($pid);
		$po->load();
		
		if (!$po->loaded()) {
			if ($po->uisLoaded()) {
				$po = $this->importPurchaseOrder($po);
			}
			else{
				drupal_set_message(
					t("Import failed, because purchase order @s doesn't exist.", 
						array('@s' => $number)), 
					Constants::DRUPAL_MESSAGE_ERROR);
				$po = NULL;
			}
		}
		
		return $po;
	}
	
	function importPurchaseOrder($po) {
		try {
			$po->createFedoraObject();
			$po->addMembership(Constants::PID_COLLECTION_PURCHASE_ORDERS);
			$po->addUisDatastream();
			$this->addCollectionDatastreams($po);
			
			drupal_set_message(t('Imported purchase order @s.', array('@s' => $po->number())));
		}
		catch (Exception $e) {
			$this->logObjectCreateError($po->pid(), $e);
			drupal_set_message(
				t('Import failed for purchase order @s', array('@s' => $po->number())), 
				Constants::DRUPAL_MESSAGE_ERROR);
			$po = null;	
		}
		
		return $po;
	}
	
	function addCollectionDatastreams($po) {
		$po_collection = new DMObject(Constants::PID_COLLECTION_PURCHASE_ORDERS);
		$po_collection->load();
		
		if ($po_collection->loaded()) {
			$this->addClassification($po, $po_collection);
			$this->addDisposition($po, $po_collection);
		}
		
		else {
			throw new Exception('error retrieving collection ' . Constants::PID_COLLECTION_PURCHASE_ORDERS);
		}
	}
	
	function addClassification($po, $po_collection) {
		$xml = $po_collection->datastreamAsXml(Constants::DSID_DEFAULT_CLASSIFICATION);
		
		global $user;
		$xml->depositorOfRecord = $user->name;
		$xml->dateOfDeposit = date('Y-m-d');
		$xml_string = $xml->asXML();
		
		$po->addClassificationDatastream($xml_string);
	}
	
	function addDisposition($po, $po_collection) {
		$xml = $po_collection->datastreamAsXml(Constants::DSID_DEFAULT_DISPOSITION);
		
		global $user;
		$xml->dispositionDecisionUser = $user->name;
		$dispositionDate = date('Y-m-d', mktime(0, 0, 0, date('m') + $xml->retentionPeriod, date('d'), date('Y')));
		$xml->dispositionDate = $dispositionDate;
		$xml_string = $xml->asXML();
		
		$po->addDispositionDatastream($xml_string);
	}
	
	function loadIncompleteDocuments() {
		$documents = array();
		$pids = DMObject::findRelatedPids(Constants::PID_COLLECTION_PURCHASE_ORDERS_INCOMPLETE_IMPORT, Constants::PID_CMODEL_PAGE);
		
		foreach($pids as $pid) {
			$page = new DMPage($pid);
			$page->load();
			$document = $page->document();
			$document->load();
			$documents[$document->pid()] = $document;
		}
		
		return array_values($documents);
	}
	
	function incompleteDocumentsOptions() {
		$options = array();
		$documents = $this->loadIncompleteDocuments();
		foreach($documents as $document) {
			$options[$document->pidWithoutNamespace()] = $document;
		}

		return $options;
	}
	
	function rescheduleIncomplete(&$form, &$form_state) {
		$documents = $this->getSelectedDocuments($form, $form_state);
		
		foreach($documents as $document) {
			// $document->load();
			$this->rescheduleIncompleteDocument($document);
		}
	}
	
	function getSelectedDocuments(&$form, &$form_state) {
		$selected = array();
		$import_all = (self::BTN_RESCHEDULE_ALL == $form_state['clicked_button']['#name']);

		foreach ($form['documents']['#options'] as $key => $document) {
			if ($import_all || $form_state['values']['documents'][$key]) {
				$selected[] = $document;
			}
		}
		
		return $selected;
	}
	
	function rescheduleIncompleteDocument($document) {
		$pages = $document->pages();
		
		$incompleteCount = 0;
		foreach($pages as $page) {
			$page->load();
				
			if ($page->incompleteImport()) {
				if ($this->rescheduleIncompletePage($page)) {
					$incompleteCount++;
				}
			}
		}
		
		drupal_set_message(t('Scheduled re-import for @x/@y pages in document @d.', 
			array(
				'@x' => $incompleteCount, 
				'@y' => $document->pageCount(),
				'@d' => $document->pidNumber())));
	}
	
	function rescheduleIncompletePage($page) {
		$ok = FALSE;
		try {
			$page->rescheduleImport();
			$ok = TRUE;
		}
		catch (Exception $e) {
			drupal_set_message(t('Error rescheduling import page @p in document @d.', 
				array(
					'@p' => $page->pidNumber(), 
					'@d' => $document->pidNumber())));
			watchdog(Constants::MODULE_NAME, 
					'Error rescheduling import for page @pid: @e', 
					array('@pid' => $page->pid(), '@e' => $e->getMessage()), WATCHDOG_WARN);
		}
		
		return $ok;
	}
}