<?php

module_load_include('inc', 'Fedora_Repository', 'api/fedora_item');
module_load_include('inc', 'Fedora_Repository', 'api/fedora_collection');
module_load_include('inc', 'Fedora_Repository', 'api/dublin_core');

/**
 * Abstract parent class for DM content objects backed by Fedora objects.
 * 
 * Provides useful methods common to all DM content objects, and isolates
 * calls to the fedora api, to simplify them, and protect the DM module
 * from changes in the api.
 */
class DMObject {
	const RELSEXT_IS_MEMBER_OF = 'isMemberOf';
	
	protected $pid;
	protected $fedora;
	
	function __construct($pid = NULL) {
		$this->pid = $pid;
	}
	
	function pid() {
		return $this->pid;
	}
	
	function pidWithoutNamespace() {
		return substr($this->pid, strlen(self::pidBase()));
	}
	
	function pidNumber() {
		return substr($this->pid, strlen(self::pidBase() . $this->pidPrefix()));
	}
	
	private function generateBarePid() {
		$pid = Fedora_Item::get_next_PID_in_namespace(Constants::PID_NAMESPACE);
		return substr($pid, strlen(Constants::PID_NAMESPACE) + 1); 
	}
	
	static function pidBase() {
		return Constants::PID_NAMESPACE . ':';
	}
	
	/**
	 * Returns a prefix for generateBarePid. Subclasses can override to customize their pids.
	 */
	function pidPrefix() {
		return '';
	}
	
	private function generatePid() {
		if (!isset($this->pid)) {
			$this->pid = self::pidBase() . $this->pidPrefix() . $this->generateBarePid();
		}
	}
	
	function typeLabel() {
		return 'Object';
	}
	
	function title() {
		return $this->typeLabel() . ' ' . $this->pidNumber();
	}

	function memberships() {
		$pids = array();
		$rels = $this->fedora->get_relationships(self::RELSEXT_IS_MEMBER_OF);
		
		foreach($rels as $rel) {
			$pids[] = $rel['object'];
		}
		
		return $pids;
	}
	
	function isMember($collection_pid) {
		return in_array($collection_pid, $this->memberships());
	}
	
	function addMembership($pid) {
		$pids = is_array($pid) ? $pid : array($pid);
		
		foreach($pids as $pid) {
			$this->fedora->add_relationship(self::RELSEXT_IS_MEMBER_OF, $pid);
		}
	}
	
	function removeMembership($pid) {
		$this->fedora->purge_relationship(self::RELSEXT_IS_MEMBER_OF, $pid);
	}
	
	function addModel($model_pid) {
		$this->fedora->add_relationship('hasModel', $model_pid, FEDORA_MODEL_URI);
	}
	
	function addDatastreamFromFile($file, $dsid, $dslabel, $mime_type) {
		$this->fedora->add_datastream_from_file($file, $dsid, $dslabel, $mime_type);
	}
	
	/**
	* Find the document associated with the given DMObject. Should only be used for DMPage or DMNote objects.
	*/
	static function findDocumentPid($dm_object) {
		$pids = $dm_object->memberships();
		foreach($pids as $pid) {
			if (DMDocument::isDocumentPid($pid)) {
				$documentPid = $pid;
				break; // there should only be one anyway.
			}
		}
				
		return $documentPid;	
	}
	
	/**
	* Find the purchase order associated with this object. Should only be used for DMNote objects.
	*/
	static function findPurchaseOrderPid($dm_object) {
		$pids = self::findPurchaseOrderPids($dm_object);
		return !empty($pids) ? $pids[0] : null;
	}
	
	/**
	* Find the purchase orders associated with this object. Should only be used for DMDocument objects.
	*/
	static function findPurchaseOrderPids($dm_object) {
		$pids = $dm_object->memberships();
		$poPids = array();
		foreach($pids as $pid) {
			if (DMPurchaseOrder::isPurchaseOrderPid($pid)) {
				$poPids[] = $pid;
			}
		}
		return $poPids;
	}
	
	function createdDateLong() {
		return strtotime($this->fedora->objectProfile->objCreateDate);
	}
	
	function createdDate() {
		return date('Y-m-d', $this->createdDateLong());
	}
	
	function createdDateTime() {
		return date('Y-m-d \a\\t H:i:s', $this->createdDateLong());
	}
	
	function createFedoraObject() {
		$this->generatePid();
		
		// determine owner
		global $user;
		$owner = $user->name;
		
		// create and store fedora item
		$foxml = Fedora_Item::create_object_FOXML($this->pid(), 'A', $this->title(), $owner);	
		$this->fedora = Fedora_Item::ingest_from_FOXML($foxml);
		
		// create and attach dublin core stream
		$dc = new Dublin_Core($fedora);
		$dc->set_element(Constants::DC_TITLE, array($title));
		$dc->set_element(Constants::DC_IDENTIFIER, array($pid));
		$dc->save();
	}
	
	function getDatastreams() {
		return $this->fedora->get_datastreams_list_as_array();
	}
	
	function getDatastream($dsid) {
		$datastreams = $this->getDatastreams();
		return $datastreams[$dsid];
	}
	
	function datastreamLink($dsid) {
		$link = NULL;
		$ds = $this->getDatastream($dsid);
		
		if ($ds) {
			global $base_url;
			$label = $ds['label'];
			$url = $base_url . '/fedora/repository/' . drupal_urlencode($this->pid()) . '/' . $dsid . '/' . drupal_urlencode($label);
			$link = '<a href="' . $url . '">' . check_plain($label) . '</a>';
		}
		
		return $link;
	}

	function fedoraBaseUrl() {
		return variable_get('fedora_base_url', 'http://localhost:8080/fedora');
	}
	
	/**
	* Should only be called on DMPage and DMNote objects.
	*/
	function document() {
		$pid = DMObject::findDocumentPid($this);
		return empty($pid) ? NULL : new DMDocument($pid);
	}

	function datastreamAsXml($datastream_id) {
		$ds = $this->fedora->get_datastream_dissemination($datastream_id);
		return new SimpleXmlElement($ds);
	}

	static function findRelatedPids($pid, $cmodel = NULL) {
		$member_pids = get_related_items_as_array($pid, self::RELSEXT_IS_MEMBER_OF, NULL, NULL, NULL, $cmodel);		
		natsort($member_pids);
		return $member_pids;
	}
	
	function load() {
		if ($this->loaded()) return;
	
		try {
			$fedora = new Fedora_Item($this->pid);
			if ($fedora->exists()) {
				$this->fedora = $fedora;
			}
		}
		
		catch (Exception $e) {
			watchdog(Constants::MODULE_NAME, t('Unable to retrieve ' . $pid . '.'), null, WATCHDOG_ERROR);
			$this->fedora = NULL;
		}
	}
	
	function loaded() {
		return !empty($this->fedora);
	}
	
	/**
	 * Add a log message to the fedora object (via modifyObject).
	 */
	function addLogMessage($message) {
		$this->fedora->modify_object(null, null, null, $message);
	}
	
	function purge() {
		$this->fedora->purge();
	}
}
