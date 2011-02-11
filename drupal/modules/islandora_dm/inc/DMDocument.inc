<?php

/*
 * DMDocument - methods for retrieving, displaying, and manipulating documents.
 */
class DMDocument extends DMObject {
	const PID_PREFIX = 'd-';

	/**
	* List of pages related to this document. sorted by pid, 
	* which should be same as insertion order. Not initialized by default.
	*/
	protected $pages;
	
	/**
	* List of notes related to this document. sorted by pid, 
	* which should be same as insertion order. Not initialized by default.
	*/
	protected $notes;
	
	function typeLabel() {
		return 'Document';
	}
	
	function pidPrefix() {
		return self::PID_PREFIX;
	}
	
	function createFedoraObject() {
		parent::createFedoraObject();
		$this->addModel(Constants::PID_CMODEL_DOCUMENT, FEDORA_MODEL_URI);
	}
	
	function load() {
		if ($this->loaded()) return;
		
		parent::load();		
		$this->pages = DMPage::findRelatedPages($this->pid());
		$this->notes = DMNote::findRelatedNotes($this->pid());
	}
	
	static function findRelatedDocuments($parent_pid) {
		$documents = array();
		
		$pids = DMObject::findRelatedPids($parent_pid, Constants::PID_CMODEL_DOCUMENT);
		foreach($pids as $pid) {
			$documents[] = new DMDocument($pid);
		}
		
		return $documents;
	}
	
	static function number2pid($number) {
		return self::pidBase() . $number;
	}
	
	static function pidBase() {
		return DMObject::pidBase() . self::PID_PREFIX;
	}

	static function isDocumentPid($pid) {
		return strpos($pid, DMObject::pidBase() . self::PID_PREFIX) !== FALSE;
	}

	function purchaseOrders() {
		$purchase_orders = array();
		$pids = DMObject::findPurchaseOrderPids($this);
		
		foreach($pids as $pid) {
			$purchase_orders[] = new DMPurchaseOrder($pid);
		}
		
		return $purchase_orders;
	}
	
	function incompleteImport($quick = TRUE) {
		foreach($this->pages as $page) {
			$page->load();
			
			if ($page->incompleteImport()) {
				return TRUE;
			}
			
			// quick mode (default) only tests first page
			if ($quick) {
				break;
			}
		}
		
		return FALSE;
	}

	function thumbnailUrl() {
		if (!empty($this->pages)) {
			$page = $this->pages[0];
			$page->load();
			$url = $page->thumbnailUrl();
		}
		
		return $url;
	}
	
	function pageCount() {
		return count($this->pages);
	}
	
	function pages() {
		return $this->pages;
	}
	
	function notes() {
		return $this->notes;
	}
	
}
