<?php

/*
 * DMPurchaseOrder - methods for retrieving, displaying, and manipulating purchase orders.
 */
class DMPurchaseOrder extends DMObject {
	const PID_PREFIX = 'po-';
	
	/**
	* SimpleXMLElement object containing up to date UIS data
	*/
	protected $uis;
	
	/**
	* SimpleXMLElement object containing fedoraUis from fedora
	*/
	protected $fedoraUis;
	
	/**
	* SimpleXMLElement object containing classificationXml from fedora
	*/
	protected $classificationXml;
	
	/**
	* SimpleXMLElement object containing dispositionXml from fedora
	*/
	protected $dispositionXml;

	/**
		* List of documents related to this purchase order. sorted by pid, 
		* which should be same as insertion order. Not initialized by default.
	*/
	protected $documents;
		
	/**
	* List of notes related to this purchase order. sorted by pid, 
	* which should be same as insertion order. Not initialized by default.
	*/
	protected $notes;
	
	function load() {
		if ($this->loaded()) return;
		
		parent::load();
		
		// load UIS record
		$this->loadUIS();
		
		// continue populating, if fedora record was found
		if ($this->loaded()) {			
			// convert fedora streams to xml
			$this->fedoraUis = $this->datastreamAsXml(Constants::DSID_UIS);
			$this->classificationXml = $this->datastreamAsXml(Constants::DSID_CLASSIFICATION); 
			$this->dispositionXml = $this->datastreamAsXml(Constants::DSID_DISPOSITION);

			$this->notes = DMNote::findRelatedNotes($this->pid());
			$this->documents = DMDocument::findRelatedDocuments($this->pid());
		}
	}
	
	function uisLoaded() {
		return !empty($this->uis);
	}
	
	/**
	* Retrieve and return the uis record for this purchase order,
	* or NULL if the item is not found.
	*/
	function loadUIS() {
		$this->uis = fetch_uis_po($this->number());
	}
	
	static function isPurchaseOrderPid($pid) {
		return strpos($pid, self::pidBase()) !== FALSE;
	}
	
	function pidPrefix() {
		return self::PID_PREFIX;
	}
	
	static function number2pid($po_number) {
		return self::pidBase() . $po_number;
	}
	
	static function pidBase() {
		return DMObject::pidBase() . self::PID_PREFIX;
	}
	
	function number() {
		return $this->pidNumber();;
	}
	
	function typeLabel() {
		return 'Purchase Order';
	}
	
	function createFedoraObject() {
		parent::createFedoraObject();
		$this->addModel(Constants::PID_CMODEL_PURCHASE_ORDER, FEDORA_MODEL_URI);
	}
	
	function poDate() {
		return substr($this->fedoraUis->podate, 0, strpos($this->fedoraUis->podate, ' '));
	}
	
	function department() {
		return $this->fedoraUis->buddept->dept;
	}
	
	function departmentId() {
		return $this->fedoraUis->buddept->idept;
	}
	
	function supplier() {
		return $this->fedoraUis->supplier->name;
	}
	
	function supplierId() {
		return $this->fedoraUis->supplier->isup;
	}
	
	function requisitionNumber() {
		return substr($this->fedoraUis->re, strpos($this->fedoraUis->re, '#') + 1);
	}
	
	function disposition() {
		return $this->dispositionXml;
	}
	
	function classification() {
		return $this->classificationXml;
	}
	
	function notes() {
		return $this->notes;
	}
	
	function documents() {
		return $this->documents;
	}
	
	function uis() {
		return $this->uis;
	}
	
	function synchedWithUIS() {
		// assume synched, if uis data is missing
		$synched = TRUE;
		if ($this->uis) {
			$uis_hash = md5(strstr($this->uis->asXML(), '<po>'));
			$fedora_hash = md5(strstr($this->fedoraUis->asXML(), '<po>'));
			$uis_hash == $fedora_hash;
		}
		
		return $synched;
	}

	function linkUrl() {
		$number = $this->pidNumber();
		return base_path() . 'islandora_dm/po/' . $number . '/view';
	}
	
	function addUisDatastream() {
		$xml_string = $this->uis->asXML();
		$this->fedora->add_datastream_from_string($xml_string, Constants::DSID_UIS, Constants::DSLABEL_UIS, Constants::MIME_TYPE_XML, 'X');
		
		// update in-memory copy, too
		$this->fedoraUis = $this->uis;
	}
	
	function updateUISDatastream() {
		$xml_string = $this->uis->asXML();
		$this->fedora->modify_datastream_by_value($xml_string, Constants::DSID_UIS, Constants::DSLABEL_UIS, Constants::MIME_TYPE_XML);
		
		// update in-memory copy, too
		$this->fedoraUis = $this->uis;
	}
	
	function addClassificationDatastream($xml_string) {
		$this->fedora->add_datastream_from_string($xml_string, 
			Constants::DSID_CLASSIFICATION, 
			Constants::DSLABEL_CLASSIFICATION, 
			Constants::MIME_TYPE_XML, 'X');
	}
	
	function addDispositionDatastream($xml_string) {
		$this->fedora->add_datastream_from_string($xml_string, 
			Constants::DSID_DISPOSITION, 
			Constants::DSLABEL_DISPOSITION, 
			Constants::MIME_TYPE_XML, 'X');
	}
}


function islandora_dm_po_view($po_number) {
	$pid = DMPurchaseOrder::number2pid($po_number);
	$purchase_order = new DMPurchaseOrder($pid);
	$purchase_order->load();
	$view = new ViewPurchaseOrder($purchase_order);
	return $view->view();
}

function islandora_dm_update_uis_data($po_number) {
	$pid = DMPurchaseOrder::number2pid($po_number);
	$purchase_order = new DMPurchaseOrder($pid);
	$purchase_order->load();
	$purchase_order->updateUISDatastream();
	drupal_set_message(t('Updated record from UIS'));
	drupal_goto('islandora_dm/po/' . $po_number . '/view');
}
